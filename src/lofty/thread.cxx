/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/thread.hxx>
#include "coroutine-scheduler.hxx"
#include "_pvt/signal_dispatcher.hxx"
#include "thread-impl.hxx"

#include <cstdlib> // std::abort()

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
   #include <signal.h> // SIG* sigaction sig*()
   #include <time.h> // nanosleep()
   #if !LOFTY_HOST_API_DARWIN
      #if LOFTY_HOST_API_FREEBSD
         #include <pthread_np.h> // pthread_getthreadid_np()
      #elif LOFTY_HOST_API_LINUX
         #include <sys/syscall.h> // SYS_*
         #include <unistd.h> // syscall()
      #endif
   #endif
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace lofty { namespace _pvt {

#if LOFTY_HOST_API_DARWIN
simple_event::simple_event() :
   disp_sem(::dispatch_semaphore_create(0)) {
   if (!disp_sem) {
      exception::throw_os_error();
   }
}
#elif LOFTY_HOST_API_POSIX
simple_event::simple_event() {
   if (::sem_init(&sem, 0, 0)) {
      exception::throw_os_error();
   }
}
#elif LOFTY_HOST_API_WIN32
simple_event::simple_event() :
   event(::CreateEvent(nullptr, true /*manual reset*/, false /*not signaled*/, nullptr)) {
   if (!event) {
      exception::throw_os_error();
   }
}
#else
   #error "TODO: HOST_API"
#endif

simple_event::~simple_event() {
#if LOFTY_HOST_API_DARWIN
   ::dispatch_release(disp_sem);
#elif LOFTY_HOST_API_POSIX
   ::sem_destroy(&sem);
#elif LOFTY_HOST_API_WIN32
   ::CloseHandle(event);
#else
   #error "TODO: HOST_API"
#endif
}

void simple_event::raise() {
#if LOFTY_HOST_API_DARWIN
   ::dispatch_semaphore_signal(disp_sem);
#elif LOFTY_HOST_API_POSIX
   ::sem_post(&sem);
#elif LOFTY_HOST_API_WIN32
   ::SetEvent(event);
#else
   #error "TODO: HOST_API"
#endif
}

void simple_event::wait() {
#if LOFTY_HOST_API_DARWIN
   ::dispatch_semaphore_wait(disp_sem, DISPATCH_TIME_FOREVER);
#elif LOFTY_HOST_API_POSIX
   /* Block until the new thread is finished updating *this. The only possible failure is EINTR, so we just
   keep on retrying. */
   while (::sem_wait(&sem)) {
      this_coroutine::interruption_point();
   }
#elif LOFTY_HOST_API_WIN32
   this_thread::interruptible_wait_for_single_object(event);
#else
   #error "TODO: HOST_API"
#endif
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

thread_local_value<thread::impl *> thread::impl::pimpl_via_tls /*= nullptr*/;

/*explicit*/ thread::impl::impl(_std::function<void ()> main_fn) :
#if LOFTY_HOST_API_POSIX
   id(0),
#elif LOFTY_HOST_API_WIN32
   handle(nullptr),
   interruption_event(::CreateEvent(nullptr, false /*auto reset*/, false /*not signaled*/, nullptr)),
#else
   #error "TODO: HOST_API"
#endif
   started_event_ptr(nullptr),
   pending_x_type(exception::common_type::none),
   terminating_(false),
   inner_main_fn(_std::move(main_fn)) {
#if LOFTY_HOST_API_WIN32
   if (!interruption_event) {
      exception::throw_os_error();
   }
#endif
}

/*explicit*/ thread::impl::impl(std::nullptr_t) :
#if LOFTY_HOST_API_POSIX
   handle(::pthread_self()),
   id(this_thread::id()),
#elif LOFTY_HOST_API_WIN32
   handle(::OpenThread(
      THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, false, ::GetCurrentThreadId()
   )),
   interruption_event(::CreateEvent(nullptr, false /*auto reset*/, false /*not signaled*/, nullptr)),
#else
   #error "TODO: HOST_API"
#endif
   started_event_ptr(nullptr),
   pending_x_type(exception::common_type::none),
   terminating_(false) {
#if LOFTY_HOST_API_WIN32
   if (!interruption_event) {
      auto err = ::GetLastError();
      ::CloseHandle(handle);
      exception::throw_os_error(err);
   }
#endif
   /* The main thread’s impl instance is instantiated by the main thread itself, so pimpl_via_tls for the main
   thread can be initialized here. */
   pimpl_via_tls = this;
}

thread::impl::~impl() {
#if LOFTY_HOST_API_WIN32
   if (handle) {
      ::CloseHandle(handle);
   }
   if (interruption_event) {
      ::CloseHandle(interruption_event);
   }
#endif
}

void thread::impl::inject_exception(
   exception::common_type x_type
#if LOFTY_HOST_API_POSIX
   , bool send_signal /*= true*/
#endif
) {
   /* Avoid interrupting the thread if there’s already a pending interruption (expected_x_type != none). This
   is not meant to prevent multiple concurrent interruptions; see @ref interruption-points. */
   auto expected_x_type = exception::common_type::none;
   if (pending_x_type.compare_exchange_strong(expected_x_type, x_type.base())) {
#if LOFTY_HOST_API_POSIX
      if (send_signal) {
         // Ensure that the thread is not blocked in a syscall.
         if (int err = ::pthread_kill(
            handle, _pvt::signal_dispatcher::instance().thread_interruption_signal())
         ) {
            exception::throw_os_error(err);
         }
      }
#elif LOFTY_HOST_API_WIN32
      /* In Win32 there’s no way to interrupt a syscall; however we can break out of the two wait-like
      syscalls used by Lofty:
      •  ::WaitFor*() calls made by Lofty include interruption_event, which can be raised to break the wait;
      •  ::GetQueuedCompletionStatus() can be made return by posting something to it; posting the IOCP’s own
         handle is be a cue to lofty::coroutine::scheduler::find_coroutine_to_activate() to check for pending
         exceptions. */
      if (!::SetEvent(interruption_event)) {
         exception::throw_os_error();
      }
      if (coro_sched) {
         ::PostQueuedCompletionStatus(
            coro_sched->iocp(), 0, reinterpret_cast< ::ULONG_PTR>(coro_sched->iocp()), nullptr
         );
      }
#else
   #error "TODO: HOST_API"
#endif
   }
}

void thread::impl::join() {
   // TODO: wait using coroutine::scheduler.
#if LOFTY_HOST_API_POSIX
   if (int err = ::pthread_join(handle, nullptr)) {
      exception::throw_os_error(err);
   }
#elif LOFTY_HOST_API_WIN32
   this_thread::interruptible_wait_for_single_object(handle);
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
}

#if LOFTY_HOST_API_POSIX
/*static*/ void * thread::impl::outer_main(void * p) {
#elif LOFTY_HOST_API_WIN32
/*static*/ ::DWORD WINAPI thread::impl::outer_main(void * p) {
   // Establish this as early as possible.
   _pvt::signal_dispatcher::init_for_current_thread();
#else
   #error "TODO: HOST_API"
#endif
   // Not really necessary since TLS will be lazily allocated, but this avoids a heap allocation.
   _pvt::thread_local_storage tls;

   /* Get a copy of the shared_ptr owning *this, so that members will be guaranteed to be accessible even
   after start() returns, in the creating thread.
   Dereferencing p is safe because the creating thread, which owns *p, is blocked, waiting for this thread to
   signal that it’s finished starting. */
   _std::shared_ptr<impl> this_pimpl(*static_cast<_std::shared_ptr<impl> *>(p));
   /* Store this_pimpl in TLS. No need to clear it before returning, since it can only be accessed by this
   thread, which will terminate upon returning. */
   pimpl_via_tls = this_pimpl.get();
#if LOFTY_HOST_API_POSIX
   this_pimpl->id = this_thread::id();
#endif

   bool uncaught_exception = false;
   try {
      _pvt::signal_dispatcher::instance().nonmain_thread_started(this_pimpl);
      // Report that this thread is done with writing to *this_pimpl.
      this_pimpl->started_event_ptr->raise();
      /* Afer the user’s main() returns we’ll set terminating_ to true, so no exceptions can be injected
      beyond this scope. A simple bool flag will work because it’s only accessed by this thread (POSIX) or
      when this thread is suspended (Win32). */
      LOFTY_DEFER_TO_SCOPE_END(this_pimpl->terminating_.store(true));
      this_pimpl->inner_main_fn();
   } catch (_std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      uncaught_exception = true;
   } catch (...) {
      exception::write_with_scope_trace();
      uncaught_exception = true;
   }
   _pvt::signal_dispatcher::instance().nonmain_thread_terminated(this_pimpl.get(), uncaught_exception);

#if LOFTY_HOST_API_POSIX
   return nullptr;
#elif LOFTY_HOST_API_WIN32
   return 0;
#else
   #error "TODO: HOST_API"
#endif
}

void thread::impl::start(_std::shared_ptr<impl> * this_pimpl_ptr) {
   _pvt::simple_event started_event;
   started_event_ptr = &started_event;
   LOFTY_DEFER_TO_SCOPE_END(started_event_ptr = nullptr);
#if LOFTY_HOST_API_POSIX
   /* In order to have the new thread block signals reserved for the main thread, block them on the current
   thread, then create the new thread, and restore them back. */
   ::sigset_t blocked_sigset, orig_sigset;
   sigemptyset(&blocked_sigset);
   sigaddset(&blocked_sigset, SIGINT);
   sigaddset(&blocked_sigset, SIGTERM);
   ::pthread_sigmask(SIG_BLOCK, &blocked_sigset, &orig_sigset);
   {
      // Reset this thread’s signal mask to orig_sigset right after (failing to?) create the thread.
      LOFTY_DEFER_TO_SCOPE_END(::pthread_sigmask(SIG_BLOCK, &orig_sigset, nullptr));
      if (int err = ::pthread_create(&handle, nullptr, &outer_main, this_pimpl_ptr)) {
         exception::throw_os_error(err);
      }
   }
#elif LOFTY_HOST_API_WIN32
   handle = ::CreateThread(nullptr, 0, &outer_main, this_pimpl_ptr, 0, nullptr);
   if (!handle) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // Block until the new thread is finished updating *this.
   started_event.wait();
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*explicit*/ thread::thread(_std::function<void ()> main_fn) :
   pimpl(_std::make_shared<impl>(_std::move(main_fn))) {
   pimpl->start(&pimpl);
}

thread::~thread() {
   if (joinable()) {
      std::abort();
   }
}

thread::id_type thread::id() const {
#if LOFTY_HOST_API_POSIX
   return pimpl ? pimpl->id : 0;
#elif LOFTY_HOST_API_WIN32
   if (pimpl) {
      ::DWORD tid = ::GetThreadId(pimpl->handle);
      if (tid == 0) {
         exception::throw_os_error();
      }
      return tid;
   } else {
      return 0;
   }
#else
   #error "TODO: HOST_API"
#endif
}

void thread::interrupt() {
   if (!pimpl) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   pimpl->inject_exception(exception::common_type::execution_interruption);
}

void thread::join() {
   if (!pimpl) {
      // TODO: use a better exception class.
      LOFTY_THROW(argument_error, ());
   }
   // Empty pimpl immediately; this will also make joinable() return false.
   auto pimpl_(_std::move(pimpl));
   pimpl_->join();

   this_coroutine::interruption_point();
}

thread::native_handle_type thread::native_handle() const {
   return pimpl ? pimpl->handle : native_handle_type();
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<thread>::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<thread>::write(thread const & src, io::text::ostream * dst) {
   dst->write(LOFTY_SL("TID:"));
   if (auto id = src.id()) {
      to_text_ostream<decltype(id)>::write(id, dst);
   } else {
      dst->write(LOFTY_SL("-"));
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace this_thread {

_std::shared_ptr<coroutine::scheduler> const & attach_coroutine_scheduler(
   _std::shared_ptr<coroutine::scheduler> coro_sched /*= nullptr*/
) {
   auto & curr_coro_sched = get_impl()->coroutine_scheduler();
   if (coro_sched) {
      if (curr_coro_sched) {
         // The current thread already has a coroutine scheduler.
         // TODO: use a better exception class.
         LOFTY_THROW(generic_error, ());
      }
      curr_coro_sched = _std::move(coro_sched);
   } else {
      // Create and set a new coroutine scheduler if the current thread didn’t already have one.
      if (!curr_coro_sched) {
         curr_coro_sched = _std::make_shared<coroutine::scheduler>();
      }
   }
   return curr_coro_sched;
}

_std::shared_ptr<coroutine::scheduler> const & coroutine_scheduler() {
   return get_impl()->coroutine_scheduler();
}

void detach_coroutine_scheduler() {
   get_impl()->coroutine_scheduler().reset();
}

thread::impl * get_impl() {
   return thread::impl::pimpl_via_tls;
}

thread::id_type id() {
#if LOFTY_HOST_API_DARWIN
   thread::id_type id;
   ::pthread_threadid_np(nullptr, &id);
   return id;
#elif LOFTY_HOST_API_FREEBSD
   static_assert(
      sizeof(thread::id_type) == sizeof(decltype(::pthread_getthreadid_np())),
      "thread::id_type must be the same size as the return type of ::pthread_getthreadid_np()"
   );
   return ::pthread_getthreadid_np();
#elif LOFTY_HOST_API_LINUX
   static_assert(
      sizeof(thread::id_type) == sizeof(::pid_t), "thread::id_type must be the same size as ::pid_t"
   );
   // This is a call to ::gettid().
   return static_cast< ::pid_t>(::syscall(SYS_gettid));
#elif LOFTY_HOST_API_WIN32
   return ::GetCurrentThreadId();
#else
   #error "TODO: HOST_API"
#endif
}

#if LOFTY_HOST_API_WIN32
void interruptible_wait_for_single_object(::HANDLE handle) {
   ::HANDLE handles[] = { handle, get_impl()->interruption_event_handle() };
   ::DWORD ret = ::WaitForMultipleObjects(LOFTY_COUNTOF(handles), handles, false, INFINITE);
   if (/*ret < WAIT_OBJECT_0 ||*/ ret >= WAIT_OBJECT_0 + LOFTY_COUNTOF(handles)) {
      exception::throw_os_error();
   }
}
#endif

void interruption_point() {
   /* This load/store is multithread-safe: the “if” condition being true means that thread::interrupt() is
   preventing other threads from changing pending_x_type until we reset it to none. */
   auto const & pimpl = get_impl();
   auto x_type = pimpl->pending_x_type.load();
   if (x_type != exception::common_type::none) {
      pimpl->pending_x_type.store(exception::common_type::none/*, _std::memory_order_relaxed*/);
      exception::throw_common_type(x_type, 0, 0);
   }
}

void run_coroutines() {
   if (auto & coro_sched = coroutine_scheduler()) {
      coro_sched->run();
   }
}

void sleep_for_ms(unsigned millisecs) {
#if LOFTY_HOST_API_POSIX
   ::timespec requested_time, remaining_time;
   requested_time.tv_sec = static_cast< ::time_t>(millisecs / 1000u);
   requested_time.tv_nsec = static_cast<long>(
      static_cast<unsigned long>(millisecs % 1000u) * 1000000u
   );
   /* This loop will only repeat in case of EINTR. Technically ::nanosleep() may fail with EINVAL, but the
   calculation above makes that impossible. */
   while (::nanosleep(&requested_time, &remaining_time) < 0) {
      interruption_point();
      // Set the new requested time to whatever we didn’t get to sleep in the last nanosleep() call.
      requested_time = remaining_time;
   }
#elif LOFTY_HOST_API_WIN32
   /* Sleep while remaining alertable via interruption_event. Note that if interruption_event does get
   signaled we won't care to enter WFSO again, because who set it probably wanted to alter the code execution
   flow. */
   ::WaitForSingleObject(get_impl()->interruption_event_handle(), millisecs);
#else
   #error "TODO: HOST_API"
#endif
   interruption_point();
}

void sleep_until_fd_ready(
   io::filedesc_t fd, bool write
#if LOFTY_HOST_API_WIN32
   , io::overlapped * ovl
#endif
) {
#if LOFTY_HOST_API_POSIX
   ::pollfd pfd;
   pfd.fd = fd;
   pfd.events = (write ? POLLIN : POLLOUT) | POLLPRI;
   while (::poll(&pfd, 1, -1) < 0) {
      int err = errno;
      if (err == EINTR) {
         interruption_point();
         break;
      }
      exception::throw_os_error(err);
   }
   if (pfd.revents & (POLLERR | POLLNVAL)) {
      // TODO: how should POLLERR and POLLNVAL be handled?
   }
   /* Consider POLLHUP as input, so that ::read() can return 0 bytes. This helps mitigate the considerable
   differences among poll(2) implementations documented at <http://www.greenend.org.uk/rjk/tech/poll.html>,
   and Linux happens to be one of those who set *only* POLLHUP on a pipe with no open write fds. */
   if (pfd.revents & (write ? POLLOUT : POLLIN | POLLHUP)) {
      if (pfd.revents & POLLPRI) {
         // TODO: anything special about “high priority data”?
      }
   } else {
      // TODO: what to do if ::poll() returned but no meaningful bits are set in pfd.revents?
   }
   interruption_point();
#elif LOFTY_HOST_API_WIN32
   LOFTY_UNUSED_ARG(write);
   interruptible_wait_for_single_object(fd);
   interruption_point();
   // If we’re still here, the wait must’ve been interrupted by fd, so update *ovl.
   ovl->get_result();
#else
   #error "TODO: HOST_API"
#endif
}

}} //namespace lofty::this_thread

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

thread_local_storage_registrar::data_members thread_local_storage_registrar::data_members_ =
   LOFTY__PVT_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

#if LOFTY_HOST_API_POSIX
   //! TLS key.
   static pthread_key_t tls_key;
   _std::atomic<unsigned> thread_local_storage::instances_count(0);
#elif LOFTY_HOST_API_WIN32
   //! TLS index.
   static ::DWORD tls_index = TLS_OUT_OF_INDEXES;
#endif

thread_local_storage::thread_local_storage() :
   context_local_storage_impl(&thread_local_storage_registrar::instance()),
   current_crls(&default_crls) {

#if LOFTY_HOST_API_POSIX
   if (instances_count++ == 0) {
      if (int err = pthread_key_create(&tls_key, &destruct)) {
         LOFTY_UNUSED_ARG(err);
         // throw an exception (err).
      }
   }
   pthread_setspecific(tls_key, this);
#elif LOFTY_HOST_API_WIN32
   ::TlsSetValue(tls_index, this);
#endif
}

thread_local_storage::~thread_local_storage() {
   unsigned remaining_attempts = 10;
   bool any_destructed;
   do {
      // Destruct CRLS for this thread.
      any_destructed = default_crls.destruct_vars(coroutine_local_storage_registrar::instance());
      if (destruct_vars(thread_local_storage_registrar::instance())) {
         any_destructed = true;
      }
   } while (--remaining_attempts > 0 && any_destructed);

#if LOFTY_HOST_API_POSIX
   pthread_setspecific(tls_key, nullptr);
   if (--instances_count == 0) {
      pthread_key_delete(tls_key);
   }
#elif LOFTY_HOST_API_WIN32
   ::TlsSetValue(tls_index, nullptr);
#endif
}

#if LOFTY_HOST_API_POSIX
/*static*/ void thread_local_storage::destruct(void * thread_this) {
   /* This is necessary (at least under Linux/glibc) to prevent creating a duplicate (which will be
   leaked) due to re-entrant calls to instance() in the destructor. The destructor ensures that this
   pointer is eventually cleared. */
   pthread_setspecific(tls_key, thread_this);
   delete static_cast<thread_local_storage *>(thread_this);
}
#endif

#if LOFTY_HOST_API_WIN32
/*static*/ bool thread_local_storage::dllmain_hook(unsigned reason) {
   if (reason == DLL_PROCESS_ATTACH) {
      tls_index = ::TlsAlloc();
      if (tls_index == TLS_OUT_OF_INDEXES) {
         // throw an exception (::GetLastError()).
      }
   } else if (reason == DLL_THREAD_DETACH || reason == DLL_PROCESS_DETACH) {
      /* Allow instance() to return nullptr if the TLS slot was not initialized for this thread, in
      which case nothing will happen. */
      delete &instance(false);
      if (reason == DLL_PROCESS_DETACH) {
         ::TlsFree(tls_index);
      }
   }
   // TODO: handle errors and return false in case.
   return true;
}
#endif //if LOFTY_HOST_API_WIN32

/*static*/ thread_local_storage & thread_local_storage::instance(bool create_new_if_null /*= true*/) {
   void * thread_this =
#if LOFTY_HOST_API_POSIX
      pthread_getspecific(tls_key);
#elif LOFTY_HOST_API_WIN32
      ::TlsGetValue(tls_index);
#endif
   if (thread_this || !create_new_if_null) {
      return *static_cast<thread_local_storage *>(thread_this);
   } else {
      // First call for this thread: initialize the TLS slot.
      return *(new thread_local_storage);
   }
}

}} //namespace lofty::_pvt

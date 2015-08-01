/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/thread.hxx>
#include "coroutine-scheduler.hxx"
#include "detail/signal_dispatcher.hxx"
#include "thread-impl.hxx"

#include <cstdlib> // std::abort()

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
   #include <signal.h> // SIG* sigaction sig*()
   #include <time.h> // nanosleep()
   #if !ABC_HOST_API_DARWIN
      #if ABC_HOST_API_FREEBSD
         #include <pthread_np.h> // pthread_getthreadid_np()
      #elif ABC_HOST_API_LINUX
         #include <sys/syscall.h> // SYS_*
         #include <unistd.h> // syscall()
      #endif
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace abc { namespace detail {

#if ABC_HOST_API_DARWIN
simple_event::simple_event() :
   m_dsem(::dispatch_semaphore_create(0)) {
   if (!m_dsem) {
      exception::throw_os_error();
   }
}
#elif ABC_HOST_API_POSIX
simple_event::simple_event() {
   if (::sem_init(&m_sem, 0, 0)) {
      exception::throw_os_error();
   }
}
#elif ABC_HOST_API_WIN32
simple_event::simple_event() :
   m_hEvent(::CreateEvent(nullptr, true /*manual reset*/, false /*not signaled*/, nullptr)) {
   if (!m_hEvent) {
      exception::throw_os_error();
   }
}
#else
   #error "TODO: HOST_API"
#endif

simple_event::~simple_event() {
#if ABC_HOST_API_DARWIN
   ::dispatch_release(m_dsem);
#elif ABC_HOST_API_POSIX
   ::sem_destroy(&m_sem);
#elif ABC_HOST_API_WIN32
   ::CloseHandle(m_hEvent);
#else
   #error "TODO: HOST_API"
#endif
}

void simple_event::raise() {
#if ABC_HOST_API_DARWIN
   ::dispatch_semaphore_signal(m_dsem);
#elif ABC_HOST_API_POSIX
   ::sem_post(&m_sem);
#elif ABC_HOST_API_WIN32
   ::SetEvent(m_hEvent);
#else
   #error "TODO: HOST_API"
#endif
}

void simple_event::wait() {
#if ABC_HOST_API_DARWIN
   ::dispatch_semaphore_wait(m_dsem, DISPATCH_TIME_FOREVER);
#elif ABC_HOST_API_POSIX
   /* Block until the new thread is finished updating *this. The only possible failure is EINTR, so
   we just keep on retrying. */
   while (::sem_wait(&m_sem)) {
      ;
   }
#elif ABC_HOST_API_WIN32
   ::WaitForSingleObject(m_hEvent, INFINITE);
#else
   #error "TODO: HOST_API"
#endif
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

thread_local_value<thread::impl *> thread::impl::sm_pimplThis /*= nullptr*/;

/*explicit*/ thread::impl::impl(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
   m_id(0),
#elif ABC_HOST_API_WIN32
   m_h(nullptr),
   m_hInterruptionEvent(::CreateEvent(
      nullptr, false /*auto reset*/, false /*not signaled*/, nullptr
   )),
#else
   #error "TODO: HOST_API"
#endif
   m_pseStarted(nullptr),
   m_xctPending(exception::common_type::none),
   m_bTerminating(false),
   m_fnInnerMain(std::move(fnMain)) {
#if ABC_HOST_API_WIN32
   if (!m_hInterruptionEvent) {
      exception::throw_os_error();
   }
#endif
}

/*explicit*/ thread::impl::impl(std::nullptr_t) :
#if ABC_HOST_API_POSIX
   m_h(::pthread_self()),
   m_id(this_thread::id()),
#elif ABC_HOST_API_WIN32
   m_h(::OpenThread(
      THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, false, ::GetCurrentThreadId()
   )),
   m_hInterruptionEvent(::CreateEvent(
      nullptr, false /*auto reset*/, false /*not signaled*/, nullptr
   )),
#else
   #error "TODO: HOST_API"
#endif
   m_pseStarted(nullptr),
   m_xctPending(exception::common_type::none),
   m_bTerminating(false) {
#if ABC_HOST_API_WIN32
   if (!m_hInterruptionEvent) {
      ::DWORD iErr = ::GetLastError();
      ::CloseHandle(m_h);
      exception::throw_os_error(iErr);
   }
#endif
   /* The main thread’s impl instance is instantiated by the main thread itself, so sm_pimplThis for
   the main thread can be initialized here. */
   sm_pimplThis = this;
}

thread::impl::~impl() {
#if ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
   }
   if (m_hInterruptionEvent) {
      ::CloseHandle(m_hInterruptionEvent);
   }
#endif
}

void thread::impl::inject_exception(exception::common_type xct) {
   ABC_TRACE_FUNC(this, xct);

   /* Avoid interrupting the thread if there’s already a pending interruption (xctExpected != none).
   This is not meant to prevent multiple concurrent interruptions, with a second interruption
   occurring after a first one has been thrown. */
   auto xctExpected = exception::common_type::none;
   if (m_xctPending.compare_exchange_strong(xctExpected, xct.base())) {
#if ABC_HOST_API_POSIX
      // Ensure that the thread is not blocked in a syscall.
      if (int iErr = ::pthread_kill(
         m_h, detail::signal_dispatcher::instance().interruption_signal_number())
      ) {
         exception::throw_os_error(iErr);
      }
#elif ABC_HOST_API_WIN32
      /* If the thread is in a wait function entered by Abaclade, this ensures that the thread will
      be able to stop waiting. Otherwise there’s no way to interrupt a syscall. */
      if (!::SetEvent(m_hInterruptionEvent)) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
   }
}

#if ABC_HOST_API_POSIX
/*static*/ void thread::impl::interruption_signal_handler(
   int iSignal, ::siginfo_t * psi, void * pctx
) {
   ABC_UNUSED_ARG(psi);

   if (iSignal == detail::signal_dispatcher::instance().interruption_signal_number()) {
      /* Can happen in any thread; all this really does is allow to break out of a syscall with
      EINTR, so the code following the interrupted call can check m_xctPending. */
      return;
   }

   exception::common_type::enum_type xct;
   if (iSignal == SIGINT) {
      // Can only happen in the main thread.
      xct = exception::common_type::user_forced_interruption;
   } else if (iSignal == SIGTERM) {
      // Can only happen in the main thread.
      xct = exception::common_type::execution_interruption;
   } else {
      // Should never happen.
      std::abort();
   }
   // This will skip injecting the exception if the thread is terminating.
   exception::inject_in_context(xct, 0, 0, pctx);
}
#endif

void thread::impl::join() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   if (int iErr = ::pthread_join(m_h, nullptr)) {
      exception::throw_os_error(iErr);
   }
#elif ABC_HOST_API_WIN32
   ::DWORD iRet = ::WaitForSingleObject(m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

#if ABC_HOST_API_POSIX
/*static*/ void * thread::impl::outer_main(void * p) {
#elif ABC_HOST_API_WIN32
/*static*/ ::DWORD WINAPI thread::impl::outer_main(void * p) {
   // Establish this as early as possible.
   detail::signal_dispatcher::init_for_current_thread();
#else
   #error "TODO: HOST_API"
#endif
   // Not really necessary since TLS will be lazily allocated, but this avoids a heap allocation.
   detail::thread_local_storage tls;

   /* Get a copy of the shared_ptr owning *this, so that members will be guaranteed to be accessible
   even after start() returns, in the creating thread.
   Dereferencing p is safe because the creating thread, which owns *p, is blocked, waiting for this
   thread to signal that it’s finished starting. */
   std::shared_ptr<impl> pimplThis(*static_cast<std::shared_ptr<impl> *>(p));
   /* Store pimplThis in TLS. No need to clear it before returning, since it can only be accessed by
   this thread, which will terminate upon returning. */
   sm_pimplThis = pimplThis.get();
#if ABC_HOST_API_POSIX
   pimplThis->m_id = this_thread::id();
#endif

   bool bUncaughtException = false;
   try {
      detail::signal_dispatcher::instance().nonmain_thread_started(pimplThis);
      // Report that this thread is done with writing to *pimplThis.
      pimplThis->m_pseStarted->raise();
      auto deferred1(defer_to_scope_end([&pimplThis] () {
         pimplThis->m_bTerminating.store(true);
      }));
      // Run the user’s main().
      pimplThis->m_fnInnerMain();
      /* deferred1 will set m_bTerminating to true, so no exceptions can be injected beyond this
      point. A simple bool flag will work because it’s only accessed by this thread (POSIX) or when
      this thread is suspended (Win32). */
   } catch (std::exception const & x) {
      exception::write_with_scope_trace(nullptr, &x);
      bUncaughtException = true;
   } catch (...) {
      exception::write_with_scope_trace();
      bUncaughtException = true;
   }
   detail::signal_dispatcher::instance().nonmain_thread_terminated(
      pimplThis.get(), bUncaughtException
   );

#if ABC_HOST_API_POSIX
   return nullptr;
#elif ABC_HOST_API_WIN32
   return 0;
#else
   #error "TODO: HOST_API"
#endif
}

void thread::impl::start(std::shared_ptr<impl> * ppimplThis) {
   ABC_TRACE_FUNC(this, ppimplThis);

   detail::simple_event seStarted;
   m_pseStarted = &seStarted;
   auto deferred1(defer_to_scope_end([this] () {
      m_pseStarted = nullptr;
   }));
#if ABC_HOST_API_POSIX
   /* In order to have the new thread block signals reserved for the main thread, block them on the
   current thread, then create the new thread, and restore them back. */
   ::sigset_t sigsetBlock, sigsetPrev;
   sigemptyset(&sigsetBlock);
   sigaddset(&sigsetBlock, SIGINT);
   sigaddset(&sigsetBlock, SIGTERM);
   ::pthread_sigmask(SIG_BLOCK, &sigsetBlock, &sigsetPrev);
   {
      auto deferred2(defer_to_scope_end([&sigsetPrev] () {
         ::pthread_sigmask(SIG_BLOCK, &sigsetPrev, nullptr);
      }));
      if (int iErr = ::pthread_create(&m_h, nullptr, &outer_main, ppimplThis)) {
         exception::throw_os_error(iErr);
      }
      // deferred2 will reset this thread’s signal mask to sigsetPrev.
   }
#elif ABC_HOST_API_WIN32
   m_h = ::CreateThread(nullptr, 0, &outer_main, ppimplThis, 0, nullptr);
   if (!m_h) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // Block until the new thread is finished updating *this.
   seStarted.wait();
   // deferred1 will reset m_pseStarted to nullptr.
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*explicit*/ thread::thread(std::function<void ()> fnMain) :
   m_pimpl(std::make_shared<impl>(std::move(fnMain))) {
   ABC_TRACE_FUNC(this);

   m_pimpl->start(&m_pimpl);
}

thread::~thread() {
   if (joinable()) {
      std::abort();
   }
}

thread::id_type thread::id() const {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   return m_pimpl ? m_pimpl->m_id : 0;
#elif ABC_HOST_API_WIN32
   if (m_pimpl) {
      ::DWORD iTid = ::GetThreadId(m_pimpl->m_h);
      if (iTid == 0) {
         exception::throw_os_error();
      }
      return iTid;
   } else {
      return 0;
   }
#else
   #error "TODO: HOST_API"
#endif
}

void thread::interrupt() {
   ABC_TRACE_FUNC(this);

   if (!m_pimpl) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   m_pimpl->inject_exception(exception::common_type::execution_interruption);
}

void thread::join() {
   ABC_TRACE_FUNC(this);

   if (!m_pimpl) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
   // Empty m_pimpl; this will also make joinable() return false.
   auto pimpl(std::move(m_pimpl));
   pimpl->join();

   // Check for pending interruptions.
   this_thread::interruption_point();
}

thread::native_handle_type thread::native_handle() const {
   return m_pimpl ? m_pimpl->m_h : native_handle_type();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

to_str_backend<thread>::to_str_backend() {
}

to_str_backend<thread>::~to_str_backend() {
}

void to_str_backend<thread>::set_format(istr const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_str_backend<thread>::write(thread const & thr, io::text::writer * ptwOut) {
   ABC_TRACE_FUNC(this/*, thr*/, ptwOut);

   if (thread::id_type id = thr.id()) {
      m_tsbStr.write(istr(ABC_SL("TID:")), ptwOut);
      m_tsbId.write(id, ptwOut);
   } else {
      m_tsbStr.write(istr(ABC_SL("TID:-")), ptwOut);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace this_thread {

std::shared_ptr<coroutine::scheduler> const & attach_coroutine_scheduler(
   std::shared_ptr<coroutine::scheduler> pcorosched /*= nullptr*/
) {
   ABC_TRACE_FUNC(pcorosched);

   std::shared_ptr<coroutine::scheduler> & pcoroschedCurr = coroutine::scheduler::sm_pcorosched;
   if (pcorosched) {
      if (pcoroschedCurr) {
         // The current thread already has a coroutine scheduler.
         // TODO: use a better exception class.
         ABC_THROW(generic_error, ());
      }
      pcoroschedCurr = std::move(pcorosched);
   } else {
      // Create and set a new coroutine scheduler if the current thread didn’t already have one.
      if (!pcoroschedCurr) {
         pcoroschedCurr = std::make_shared<coroutine::scheduler>();
      }
   }
   return pcoroschedCurr;
}

std::shared_ptr<coroutine::scheduler> const & coroutine_scheduler() {
   return coroutine::scheduler::sm_pcorosched;
}

void detach_coroutine_scheduler() {
   coroutine::scheduler::sm_pcorosched.reset();
}

thread::impl * get_impl() {
   return thread::impl::sm_pimplThis;
}

thread::id_type id() {
#if ABC_HOST_API_DARWIN
   thread::id_type id;
   ::pthread_threadid_np(nullptr, &id);
   return id;
#elif ABC_HOST_API_FREEBSD
   static_assert(
      sizeof(thread::id_type) == sizeof(decltype(::pthread_getthreadid_np())),
      "return type of ::pthread_getthreadid_np() must be the same size as thread::id_type"
   );
   return ::pthread_getthreadid_np();
#elif ABC_HOST_API_LINUX
   static_assert(
      sizeof(thread::id_type) == sizeof(::pid_t), "::pid_t must be the same size as thread::id_type"
   );
   // This is a call to ::gettid().
   return static_cast< ::pid_t>(::syscall(SYS_gettid));
#elif ABC_HOST_API_WIN32
   return ::GetCurrentThreadId();
#else
   #error "TODO: HOST_API"
#endif
}

void interruption_point() {
   /* This load/store is multithread-safe: the “if” condition being true means that
   thread::interrupt() is preventing other threads from changing m_xctPending until we reset it to
   none. */
   auto const & pimpl = get_impl();
   auto xct = pimpl->m_xctPending.load();
   if (xct != exception::common_type::none) {
      pimpl->m_xctPending.store(exception::common_type::none, std::memory_order_relaxed);
      exception::throw_common_type(xct, 0, 0);
   }
}

void run_coroutines() {
   if (auto & pcorosched = coroutine_scheduler()) {
      pcorosched->run();
   }
}

void sleep_for_ms(unsigned iMillisecs) {
#if ABC_HOST_API_POSIX
   ::timespec tsRequested, tsRemaining;
   tsRequested.tv_sec = static_cast< ::time_t>(iMillisecs / 1000u);
   tsRequested.tv_nsec = static_cast<long>(
      static_cast<unsigned long>(iMillisecs % 1000u) * 1000000u
   );
   /* This loop will only repeat in case of EINTR. Technically ::nanosleep() may fail with EINVAL,
   but the calculation above makes that impossible. */
   while (::nanosleep(&tsRequested, &tsRemaining) < 0) {
      // Set the new requested time to whatever we didn’t get to sleep in the last nanosleep() call.
      tsRequested = tsRemaining;
   }
#elif ABC_HOST_API_WIN32
   /* Sleep while remaining alertable via m_hInterruptionEvent. Note that if m_hInterruptionEvent
   does get signaled we won't care to enter WFSO again, because who set it probably wanted to alter
   the code execution flow. */
   ::WaitForSingleObject(get_impl()->interruption_event_handle(), iMillisecs);
#else
   #error "TODO: HOST_API"
#endif

   // Check for pending interruptions.
   interruption_point();
}

void sleep_until_fd_ready(io::filedesc_t fd, bool bWrite) {
#if ABC_HOST_API_POSIX
   ::pollfd pfd;
   pfd.fd = fd;
   pfd.events = (bWrite ? POLLIN : POLLOUT) | POLLPRI;
   while (::poll(&pfd, 1, -1) < 0) {
      int iErr = errno;
      if (iErr == EINTR) {
         break;
      }
      exception::throw_os_error(iErr);
   }
   if (pfd.revents & (POLLERR | POLLNVAL)) {
      // TODO: how should POLLERR and POLLNVAL be handled?
   }
   /* Consider POLLHUP as input, so that ::read() can return 0 bytes. This helps mitigate the
   considerable differences among poll(2) implementations documented at
   <http://www.greenend.org.uk/rjk/tech/poll.html>, and Linux happens to be one of those who set
   *only* POLLHUP on a pipe with no open write fds. */
   if (pfd.revents & (bWrite ? POLLOUT : POLLIN | POLLHUP)) {
      if (pfd.revents & POLLPRI) {
         // TODO: anything special about “high priority data”?
      }
   } else {
      // TODO: what to do if ::poll() returned but no meaningful bits are set in pfd.revents?
   }
#elif ABC_HOST_API_WIN32
   ABC_UNUSED_ARG(bWrite);
   ::HANDLE ah[] = { fd, get_impl()->interruption_event_handle() };
   ::DWORD iRet = ::WaitForMultipleObjects(ABC_COUNTOF(ah), ah, false, INFINITE);
   if (/*iRet < WAIT_OBJECT_0 ||*/ iRet >= WAIT_OBJECT_0 + ABC_COUNTOF(ah)) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif

   // Check for pending interruptions.
   interruption_point();
}

}} //namespace abc::this_thread

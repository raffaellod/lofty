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
#include "exception-fault_converter.hxx"
#include "thread-tracker.hxx"
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
   m_hEvent(::CreateEvent(nullptr, true /*manual reset*/, false /*not signlaled*/, nullptr)) {
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

thread::impl::impl(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
   m_id(0),
#elif ABC_HOST_API_WIN32
   m_h(nullptr),
#else
   #error "TODO: HOST_API"
#endif
   m_pseStarted(nullptr),
   m_bTerminating(false),
   m_fnInnerMain(std::move(fnMain)) {
}
// This overload is used to instantiate an impl for the main thread.
thread::impl::impl(std::nullptr_t) :
#if ABC_HOST_API_POSIX
   m_h(::pthread_self()),
   m_id(this_thread::id()),
#elif ABC_HOST_API_WIN32
   m_h(::OpenThread(
      THREAD_GET_CONTEXT | THREAD_SET_CONTEXT | THREAD_SUSPEND_RESUME, false, ::GetCurrentThreadId()
   )),
#else
   #error "TODO: HOST_API"
#endif
   m_pseStarted(nullptr),
   m_bTerminating(false) {
   /* The main thread’s impl instance is instantiated by the main thread itself, so sm_pimplThis for
   the main thread can be initialized here. */
   sm_pimplThis = this;
}

thread::impl::~impl() {
#if ABC_HOST_API_WIN32
   if (m_h) {
      ::CloseHandle(m_h);
   }
#endif
}

void thread::impl::inject_exception(exception::common_type xct) {
   ABC_TRACE_FUNC(this, xct);

#if ABC_HOST_API_POSIX
   int iSignal;
   switch (xct.base()) {
      case exception::common_type::app_execution_interruption:
         // TODO: different signal number.
         iSignal = tracker::instance()->exception_injection_signal_number();
         break;
      case exception::common_type::app_exit_interruption:
         // TODO: different signal number.
         iSignal = tracker::instance()->exception_injection_signal_number();
         break;
      case exception::common_type::execution_interruption:
         iSignal = tracker::instance()->exception_injection_signal_number();
         break;
      case exception::common_type::user_forced_interruption:
         // TODO: different signal number.
         iSignal = tracker::instance()->exception_injection_signal_number();
         break;
      default:
         ABC_THROW(domain_error, ());
   }
   if (int iErr = ::pthread_kill(m_h, iSignal)) {
      exception::throw_os_error(iErr);
   }
#elif ABC_HOST_API_WIN32
   if (::SuspendThread(m_h) == ::DWORD(-1)) {
      exception::throw_os_error();
   }
   auto deferred1(defer_to_scope_end([this] () {
      ::ResumeThread(m_h);
   }));
   /* As an attempt to avoid bugs like <http://stackoverflow.com/questions/3444190/windows-
   suspendthread-doesnt-getthreadcontext-fails>, repeatedly yield and get the thread context until
   that reveals that the thread has really stopped, which wouldn’t be otherwise guaranteed on a
   multi-processor system.

   It’s still possible that the thread might be executing kernel code, which would not cause it to
   stop even on an uniprocessor system; yielding could also avoid that, by giving the OS a chance to
   run the thread (priority changes can void this assumption), changing its context and making the
   non-suspension detectable.

   See <http://www.dcl.hpi.uni-potsdam.de/research/WRK/2009/01/what-does-suspendthread-really-do/>
   for the two race conditions mentioned above. */
   ::CONTEXT ctx;
   std::uintptr_t iLastPC = 0;
   for (;;) {
      ctx.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
      if (!::GetThreadContext(m_h, &ctx)) {
         exception::throw_os_error();
      }
      std::uintptr_t iCurrPC =
#if ABC_HOST_ARCH_ARM
         ctx.Pc;
#elif ABC_HOST_ARCH_I386
         ctx.Eip;
#elif ABC_HOST_ARCH_X86_64
         ctx.Rip;
#else
   #error "TODO: HOST_ARCH"
#endif
      if (iCurrPC == iLastPC) {
         /* Assume that the thread has stopped. This condition is prone to false positives; the thread
         might be executing a loop, or a recursive function call, or who knows what. Still, this is
         better than not making an effort to wait for the thread to stop. */
         break;
      }
      ::Sleep(0);
      iLastPC = iCurrPC;
   }

   /* Now that the thread is really suspended, inject the exception and resume it, unless the thread
   is already terminating, in which case we’ll avoid throwing an exception. This check is safe
   because m_bTerminating can only be written to by the thread we just suspended. */
   if (!m_bTerminating.load()) {
      exception::inject_in_context(xct, 0, 0, &ctx);
      if (!::SetThreadContext(m_h, &ctx)) {
         exception::throw_os_error();
      }
   }
   // deferred1 will resume the thread.
#else
   #error "TODO: HOST_API"
#endif
}

#if ABC_HOST_API_POSIX
/*static*/ void thread::impl::interruption_signal_handler(
   int iSignal, ::siginfo_t * psi, void * pctx
) {
   ABC_UNUSED_ARG(psi);

   exception::common_type::enum_type xct;
   if (iSignal == SIGINT) {
      // Can only happen in the main thread.
      xct = exception::common_type::user_forced_interruption;
   } else if (iSignal == SIGTERM) {
      // Can only happen in the main thread.
      xct = exception::common_type::execution_interruption;
   } else if (iSignal == tracker::instance()->exception_injection_signal_number()) {
      // Can happen in any thread.
      /* TODO: determine the exception type by accessing a mutex-guarded member variable, set by the
      thread that raised the signal. */
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
#else
   #error "TODO: HOST_API"
#endif
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
#elif ABC_HOST_API_WIN32
   exception::fault_converter::init_for_current_thread();
#endif
   bool bUncaughtException = false;
   try {
      tracker::instance()->nonmain_thread_started(pimplThis);
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
   tracker::instance()->nonmain_thread_terminated(pimplThis.get(), bUncaughtException);

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

thread::tracker * thread::tracker::sm_pInst = nullptr;

thread::tracker::tracker()
#if ABC_HOST_API_POSIX
   : mc_iInterruptionSignal(
   #if ABC_HOST_API_DARWIN
      // SIGRT* not available.
      SIGUSR1
   #else
      SIGRTMIN + 1
   #endif
   ) {
#else
   {
#endif
   sm_pInst = this;
#if ABC_HOST_API_POSIX
   // Setup signal handlers.
   struct ::sigaction sa;
   sa.sa_sigaction = &impl::interruption_signal_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   ::sigaction(mc_iInterruptionSignal, &sa, nullptr);
   ::sigaction(SIGINT,                 &sa, nullptr);
   ::sigaction(SIGTERM,                &sa, nullptr);
#endif
}

thread::tracker::~tracker() {
#if ABC_HOST_API_POSIX
   // Restore the default signal handlers.
   ::signal(SIGINT,                 SIG_DFL);
   ::signal(SIGTERM,                SIG_DFL);
   ::signal(mc_iInterruptionSignal, SIG_DFL);
#endif
   sm_pInst = nullptr;
}

void thread::tracker::main_thread_started() {
   m_pimplMainThread = std::make_shared<impl>(nullptr);
}

void thread::tracker::main_thread_terminated(exception::common_type xct) {
   /* Note: at this point, a correct program should have no other threads running. As a courtesy,
   Abaclade will prevent the process from terminating while threads are still running, by ensuring
   that all Abaclade-managed threads are joined before termination; however, app::main() returning
   when m_hmThreads.size() > 0 should be considered an exception (and a bug) rather than the
   rule. */

   // Make this thread uninterruptible by other threads.
   m_pimplMainThread->m_bTerminating.store(true);

   std::unique_lock<std::mutex> lock(m_mtxThreads);
   // Signal every other thread to terminate.
   ABC_FOR_EACH(auto kv, m_hmThreads) {
      kv.value->inject_exception(xct);
   }
   /* Wait for all threads to terminate; as they do, they’ll invoke nonmain_thread_terminated() and
   have themselves removed from m_hmThreads. We can’t join() them here, since they might be joining
   amongst themselves in some application-defined order, and we can’t join the same thread more than
   once (at least in POSIX). */
   while (!m_hmThreads.empty()) {
      lock.unlock();
      // Yes, we just sleep. Remember, this should not really happen (see the note above).
      this_thread::sleep_for_ms(1);
      // Re-lock the mutex and check again.
      lock.lock();
   }
}

void thread::tracker::nonmain_thread_started(std::shared_ptr<impl> const & pimpl) {
   std::lock_guard<std::mutex> lock(m_mtxThreads);
   m_hmThreads.add_or_assign(pimpl.get(), pimpl);
}

void thread::tracker::nonmain_thread_terminated(impl * pimpl, bool bUncaughtException) {
   // Remove the thread from the bookkeeping list.
   {
      std::lock_guard<std::mutex> lock(m_mtxThreads);
      m_hmThreads.remove(pimpl);
   }
   /* If the thread was terminated by an exception making it all the way out of the thread function,
   all other threads must terminate as well. Achieve this by “forwarding” the exception to the main
   thread, so that its termination will in turn cause the termination of all other threads. */
   if (bUncaughtException) {
      /* TODO: use a more specific exception subclass of execution_interruption, such as
      “other_thread_execution_interrupted”. */
      m_pimplMainThread->inject_exception(exception::common_type::execution_interruption);
   }
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
      std::terminate();
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
   auto deferred1(defer_to_scope_end([this] () {
      /* Release the impl instance; this will also make joinable() return false.
      If *this was interrupted, it might cause the current thread to be interrupted as well; make
      sure that m_pimpl is released in any case. Under POSIX, pthread_join() will not return EINTR,
      meaning that we can rely on the fact that *this really terminated, so releasing the pointer is
      correct; under Win32, TODO: validate. */
      m_pimpl.reset();
   }));
   m_pimpl->join();
   // deferred1 will release m_pimpl.
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
   ::Sleep(iMillisecs);
#else
   #error "TODO: HOST_API"
#endif
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
   if (::WaitForSingleObject(fd, INFINITE) != WAIT_OBJECT_0) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

}} //namespace abc::this_thread

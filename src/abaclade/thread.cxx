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
#include "thread-comm_manager.hxx"

#if ABC_HOST_API_POSIX
   #include <errno.h> // EINVAL errno
   #include <signal.h> // SIG* sigaction sig*()
   #include <time.h> // nanosleep()
   #if ABC_HOST_API_DARWIN
      #include <dispatch/dispatch.h>
   #else
      #include <semaphore.h>
      #if ABC_HOST_API_FREEBSD
         #include <pthread_np.h> // pthread_getthreadid_np()
      #elif ABC_HOST_API_LINUX
         #include <sys/syscall.h> // SYS_*
         #include <unistd.h> // syscall()
      #endif
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::simple_event

/*! Event that can be waited for. Not compatible with coroutines, since it doesn’t yield to a
coroutine::scheduler. */
// TODO: make this a non-coroutine-friendly general-purpose event.
namespace abc {
namespace detail {

class simple_event : public noncopyable {
public:
   //! Constructor.
   simple_event() {
#if ABC_HOST_API_DARWIN
      m_dsem = ::dispatch_semaphore_create(0);
      if (!m_dsem) {
         exception::throw_os_error();
      }
#elif ABC_HOST_API_POSIX
      if (::sem_init(&m_sem, 0, 0)) {
         exception::throw_os_error();
      }
#elif ABC_HOST_API_WIN32
      m_hEvent = ::CreateEvent(nullptr, true, false, nullptr);
      if (!m_hEvent) {
         exception::throw_os_error();
      }
#else
   #error "TODO: HOST_API"
#endif
   }

   //! Destructor.
   ~simple_event() {
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

   //! Raises the event.
   void raise() {
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

   //! Waits for the event to be raised by another thread.
   void wait() {
#if ABC_HOST_API_DARWIN
      ::dispatch_semaphore_wait(m_dsem, DISPATCH_TIME_FOREVER);
#elif ABC_HOST_API_POSIX
      /* Block until the new thread is finished updating *this. The only possible failure is EINTR,
      so we just keep on retrying. */
      while (::sem_wait(&m_sem)) {
         ;
      }
#elif ABC_HOST_API_WIN32
      ::WaitForSingleObject(m_hEvent, INFINITE);
#else
   #error "TODO: HOST_API"
#endif
   }

private:
#if ABC_HOST_API_DARWIN
   //! Underlying dispatch semaphore.
   ::dispatch_semaphore_t m_dsem;
#elif ABC_HOST_API_POSIX
   //! Underlying POSIX semaphore.
   ::sem_t m_sem;
#elif ABC_HOST_API_WIN32
   //! Underlying event.
   ::HANDLE m_hEvent;
#else
   #error "TODO: HOST_API"
#endif
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

class thread::impl {
private:
   friend class thread;

public:
   /*! Constructor

   @param fnMain
      Initial value for m_fnInnerMain.
   */
   impl(std::function<void ()> fnMain) :
#if ABC_HOST_API_POSIX
      m_id(0),
#elif ABC_HOST_API_WIN32
      m_h(nullptr),
#else
   #error "TODO: HOST_API"
#endif
      m_pseStarted(nullptr),
      m_fnInnerMain(std::move(fnMain)) {
   }

   //! Destructor.
   ~impl() {
#if ABC_HOST_API_WIN32
      if (m_h) {
         ::CloseHandle(m_h);
      }
#endif
   }

   /*! Injects the requested type of exception in the thread.

   TODO: prevent concurrent injections by multiple threads. Could be done by keeping an
   atomic<unsigned> “outstanding interruptions counter” member that execution_interruption instances
   increment/decrement via a pointer, and that inhibits further injections if non-zero. This should
   then be ported to abc::coroutine::context for the same reason.

   @param inj
      Type of exception to inject.
   */
   void inject_exception(exception::injectable inj) {
      ABC_TRACE_FUNC(this, inj);

#if ABC_HOST_API_POSIX
      int iSignal = comm_manager::instance()->injectable_exception_signal_number(inj);
      if (int iErr = ::pthread_kill(m_h, iSignal)) {
         exception::throw_os_error(iErr);
      }
#elif ABC_HOST_API_WIN32
      if (::SuspendThread(m_h) == ::DWORD(-1)) {
         exception::throw_os_error();
      }
      auto deferred1(defer_to_scope_end([this] () -> void {
         ::ResumeThread(m_h);
      }));
      std::uintptr_t iLastPC = 0;
      ::CONTEXT ctx;
      /* As an attempt to avoid bugs like <http://stackoverflow.com/questions/3444190/windows-
      suspendthread-doesnt-getthreadcontext-fails>, repeatedly yield and get the thread context
      until that reveals that the thread has really stopped, which wouldn’t be otherwise guaranteed
      on a multi-processor system.

      It’s still possible that the thread might be executing kernel code, which would not cause it
      to stop even on an uniprocessor system; yielding could also avoid that, by giving the OS a
      chance to run the thread (priority changes can void this assumption), changing its context and
      making the non-suspension detectable.

      See <http://www.dcl.hpi.uni-potsdam.de/research/WRK/2009/01/what-does-suspendthread-really-
      do/> for the two race conditions mentioned above. */
      do {
         if (!::GetThreadContext(m_h, &ctx)) {
            exception::throw_os_error();
         }
         std::uintptr_t iCurrPC;
   #if ABC_HOST_ARCH_ARM
         iCurrPC = ctx.Pc;
   #elif ABC_HOST_ARCH_I386
         iCurrPC = ctx.Eip;
   #elif ABC_HOST_ARCH_X86_64
         iCurrPC = ctx.Rip;
   #else
      #error "TODO: HOST_ARCH"
   #endif
      } while (iCurrPC != iLastPC);

      // Now that the thread is really suspended, inject the exception and resume it.
      exception::inject_in_context(inj, 0, 0, &ctx);
      if (!::SetThreadContext(m_h, &ctx)) {
         exception::throw_os_error();
      }
      // deferred1 will resume the thread.
#else
   #error "TODO: HOST_API"
#endif
   }

   /*! Creates a thread to run outer_main().

   @param ppimplThis
      Pointer by which outer_main() will get a reference (as in refcount) to *this, preventing it
      from being deallocated while still running.
   */
   void start(std::shared_ptr<impl> * ppimplThis) {
      ABC_TRACE_FUNC(this, ppimplThis);

      detail::simple_event seStarted;
      m_pseStarted = &seStarted;
      auto deferred1(defer_to_scope_end([this] () -> void {
         m_pseStarted = nullptr;
      }));
#if ABC_HOST_API_POSIX
      /* In order to have the new thread block signals reserved for the main thread, block them on
      the current thread, then create the new thread, and restore them back. */
      ::sigset_t sigsetBlock, sigsetPrev;
      sigemptyset(&sigsetBlock);
      ::sigaddset(&sigsetBlock, SIGINT);
      ::sigaddset(&sigsetBlock, SIGTERM);
      ::pthread_sigmask(SIG_BLOCK, &sigsetBlock, &sigsetPrev);
      {
         auto deferred2(defer_to_scope_end([&sigsetPrev] () -> void {
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

private:
   /*! Lower-level wrapper for the thread function passed to the constructor. Under POSIX, this is
   also needed to assign the thread ID to the owning abc::thread instance.

   @param p
      Pointer to a shared_ptr to *this.
   @return
      Unused.
   */
#if ABC_HOST_API_POSIX
   static void * outer_main(void * p) {
#elif ABC_HOST_API_WIN32
   static ::DWORD WINAPI outer_main(void * p) {
#else
   #error "TODO: HOST_API"
#endif
      /* Get a copy of the shared_ptr owning *this, so that members will be guaranteed to be
      accessible even after start() returns, in the creating thread.
      Dereferencing p is safe because the creating thread, which owns *p, is blocked, waiting for
      this thread to signal that it’s finished starting. */
      std::shared_ptr<impl> pimplThis(*static_cast<std::shared_ptr<impl> *>(p));
#if ABC_HOST_API_POSIX
      pimplThis->m_id = this_thread::id();
#elif ABC_HOST_API_WIN32
      exception::fault_converter::init_for_current_thread();
#endif
      try {
         // Report that this thread is done with writing to *pimplThis.
         pimplThis->m_pseStarted->raise();
         // Run the user’s main().
         pimplThis->m_fnInnerMain();
      } catch (std::exception const & x) {
         exception::write_with_scope_trace(nullptr, &x);
         // TODO: kill every other thread.
      } catch (...) {
         exception::write_with_scope_trace();
         // TODO: kill every other thread.
      }
#if ABC_HOST_API_POSIX
      return nullptr;
#elif ABC_HOST_API_WIN32
      return 0;
#else
   #error "TODO: HOST_API"
#endif
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_POSIX
   //! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   id_type m_id;
#endif
   /*! Pointer to an event used by the new thread to report to its parent that it has started. Only
   non-nullptr during the execution of start(). */
   detail::simple_event * m_pseStarted;
   //! Function to be executed in the thread.
   std::function<void ()> m_fnInnerMain;
};


thread::comm_manager * thread::comm_manager::sm_pInst = nullptr;

thread::comm_manager::comm_manager()
#if ABC_HOST_API_POSIX
   : mc_iInterruptionSignal(
   #if ABC_HOST_API_DARWIN
      // SIGRT* not available.
      SIGUSR1
   #else
      SIGRTMIN + 1
   #endif
   )
#endif
{
   sm_pInst = this;
#if ABC_HOST_API_POSIX
   // Setup signal handlers.
   struct ::sigaction sa;
   sa.sa_sigaction = &interruption_signal_handler;
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_SIGINFO;
   ::sigaction(mc_iInterruptionSignal, &sa, nullptr);
#endif
}

thread::comm_manager::~comm_manager() {
   // Restore the default signal handlers.
   ::signal(mc_iInterruptionSignal, SIG_DFL);
   sm_pInst = nullptr;
}

#if ABC_HOST_API_POSIX
/*static*/ void thread::comm_manager::interruption_signal_handler(
   int iSignal, ::siginfo_t * psi, void * pctx
) {
   ABC_UNUSED_ARG(psi);

   comm_manager * ptcmThis = instance();
   exception::injectable::enum_type inj;
   if (iSignal == ptcmThis->mc_iInterruptionSignal) {
      // Can happen in any thread.
      /* TODO: determine the exception type by accessing a mutex-guarded member variable, set by the
      thread that raised the signal. */
      inj = exception::injectable::execution_interruption;
   } else if (iSignal == SIGINT) {
      // Can only happen in main thread.
      inj = exception::injectable::user_forced_interruption;
   } else if (iSignal == SIGTERM) {
      // Can only happen in main thread.
      inj = exception::injectable::execution_interruption;
   } else {
      std::abort();
   }

   // Inject a function call to exception::throw_injected_exception().
   exception::inject_in_context(inj, 0, 0, pctx);
}

int thread::comm_manager::injectable_exception_signal_number(exception::injectable inj) const {
   switch (inj.base()) {
      /*case exception::injectable::app_execution_interruption:
         return ?;*/
      case exception::injectable::execution_interruption:
         return mc_iInterruptionSignal;
      /*case exception::injectable::user_forced_interruption:
         return ?;*/
      default:
         ABC_THROW(domain_error, ());
   }
}
#endif


/*explicit*/ thread::thread(std::function<void ()> fnMain) :
   m_pimpl(std::make_shared<impl>(std::move(fnMain))) {
   ABC_TRACE_FUNC(this);

   m_pimpl->start(&m_pimpl);
}

thread::~thread() {
   if (joinable()) {
      // TODO: std::terminate() or something similar.
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
   m_pimpl->inject_exception(exception::injectable::execution_interruption);
}

void thread::join() {
   ABC_TRACE_FUNC(this);

   if (!m_pimpl) {
      // TODO: use a better exception class.
      ABC_THROW(argument_error, ());
   }
#if ABC_HOST_API_POSIX
   if (int iErr = ::pthread_join(m_pimpl->m_h, nullptr)) {
      exception::throw_os_error(iErr);
   }
#elif ABC_HOST_API_WIN32
   ::DWORD iRet = ::WaitForSingleObject(m_pimpl->m_h, INFINITE);
   if (iRet == WAIT_FAILED) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
   // Release the impl instance; this will also make joinable() return false.
   m_pimpl.reset();
}

thread::native_handle_type thread::native_handle() const {
   return m_pimpl ? m_pimpl->m_h : native_handle_type();
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::thread

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
// abc::this_thread

namespace abc {
namespace this_thread {

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

std::shared_ptr<coroutine::scheduler> const & get_coroutine_scheduler() {
   return coroutine::scheduler::sm_pcorosched;
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
   if (auto & pcorosched = get_coroutine_scheduler()) {
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

} //namespace this_thread
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

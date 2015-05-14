/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#ifndef _ABACLADE_COROUTINE_SCHEDULER_HXX
#define _ABACLADE_COROUTINE_SCHEDULER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/coroutine.hxx>
#include <abaclade/collections/list.hxx>
#include <abaclade/collections/map.hxx>
#include <abaclade/thread.hxx>

#if ABC_HOST_API_POSIX
   #if ABC_HOST_API_DARWIN
      #define _XOPEN_SOURCE
   #endif
   #include <ucontext.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine::scheduler

namespace abc {

class coroutine::scheduler : public noncopyable {
private:
   friend coroutine::id_type this_coroutine::id();
   friend std::shared_ptr<coroutine::scheduler> const & this_thread::attach_coroutine_scheduler(
      std::shared_ptr<coroutine::scheduler> pcorosched /*= nullptr*/
   );
   friend std::shared_ptr<coroutine::scheduler> const & this_thread::coroutine_scheduler();
   friend void this_thread::detach_coroutine_scheduler();

public:
   //! Constructor.
   scheduler();

   //! Destructor.
   ~scheduler();

   /*! Adds a coroutine to those ready to run. Ready coroutines take precedence over coroutines that
   were known to be blocked but might be ready on the next find_coroutine_to_activate() invocation.

   @param pcoroimpl
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   void add_ready(std::shared_ptr<coroutine::impl> pcoroimpl);

   /*! Allows other coroutines to run, preventing the calling coroutine from being rescheduled until
   at least iMillisecs milliseconds have passed.

   @param iMillisecs
      Minimum duration for which to yield to other coroutines.
   */
   void block_active_for_ms(unsigned iMillisecs);

   /*! Allows other coroutines to run while the asynchronous I/O operation completes, as an
   alternative to blocking while waiting for its completion.

   @param fd
      File descriptor that the calling coroutine is waiting for I/O on.
   @param bWrite
      true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
   */
   void block_active_until_fd_ready(io::filedesc_t fd, bool bWrite);

   //! Switches context to the current thread’s own context.
   void return_to_scheduler();

   /*! Begins scheduling and running coroutines on the current thread. Only returns after every
   coroutine added with add_coroutine() returns. */
   void run();

private:
   /*! Finds a coroutine ready to execute; if none are, but there are blocked coroutines, it blocks
   the current thread until one of them becomes ready.

   @return
      Pointer to a coroutine (implementation) that’s ready to execute.
   */
   std::shared_ptr<coroutine::impl> find_coroutine_to_activate();

   /*! Switches context from the coroutine context pointed to by pcoroimplLastActive to the current
   thread’s own context.

   @param pcoroimplLastActive
      Pointer to the coroutine (implementation) that is being inactivated.
   */
   void switch_to_scheduler(coroutine::impl * pcoroimplLastActive);

private:
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   io::filedesc m_fdKqueue;
   /*! Coroutines that are blocked on a timer wait. The keys are the same as the values, but this
   can’t be changed into a set<shared_ptr<impl>> because we need it to hold a strong reference to
   the coroutine implementation while allowing lookups without having a shared_ptr. */
   collections::map<std::uintptr_t, std::shared_ptr<coroutine::impl>> m_mapCorosBlockedByTimer;
#elif ABC_HOST_API_LINUX
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   /*! Timers currently being waited for. The key is the same as the value, but this can’t be
   changed into a set<io::filedesc> until io::filedesc is hashable. */
   collections::map<io::filedesc_t, io::filedesc> m_mapActiveTimers;
#elif ABC_HOST_API_WIN32
   //! File descriptor of the internal IOCP.
   io::filedesc m_fdIocp;
#else
   #error "TODO: HOST_API"
#endif
   //! Coroutines that are blocked on a fd wait.
   collections::map<io::filedesc_t, std::shared_ptr<coroutine::impl>> m_mapCorosBlockedByFD;
   /*! List of coroutines that are ready to run. Includes coroutines that have been scheduled, but
   have not been started yet. */
   collections::list<std::shared_ptr<coroutine::impl>> m_listReadyCoros;
   //! Pointer to the active (current) coroutine, or nullptr if none is active.
   static thread_local_value<std::shared_ptr<coroutine::impl>> sm_pcoroimplActive;
   //! Pointer to the coroutine scheduler for the current thread.
   static thread_local_value<std::shared_ptr<scheduler>> sm_pcorosched;
#if ABC_HOST_API_POSIX
   //! Pointer to the original context of every thread running a coroutine scheduler.
   static thread_local_value< ::ucontext_t *> sm_puctxReturn;
#elif ABC_HOST_API_POSIX
   //! Handle to the original fiber of every thread running a coroutine scheduler.
   static thread_local_value<void *> sm_pfbrReturn;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_SCHEDULER_HXX

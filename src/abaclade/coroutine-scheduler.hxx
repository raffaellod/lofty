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

#include <abaclade.hxx>
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
   friend std::shared_ptr<coroutine::scheduler> const & this_thread::attach_coroutine_scheduler(
      std::shared_ptr<coroutine::scheduler> pcorosched /*= nullptr*/
   );
   friend std::shared_ptr<coroutine::scheduler> const & this_thread::get_coroutine_scheduler();
   friend coroutine::id_type this_coroutine::id();

public:
   //! Constructor.
   scheduler();

   //! Destructor.
   ~scheduler();

   /*! Adds a coroutine to those ready to start.

   @param coro
      Coroutine to add.
   */
   void add(coroutine const & coro);

   //! Switches context to the current thread’s own context.
   void return_to_scheduler();

   /*! Begins scheduling and running coroutines on the current thread. Only returns after every
   coroutine added with add_coroutine() returns. */
   void run();

   /*! Allows other coroutines to run, preventing the calling coroutine from being rescheduled until
   at least iMillisecs milliseconds have passed.

   @param iMillisecs
      Minimum duration for which to yield to other coroutines.
   */
   void yield_for(unsigned iMillisecs);

   /*! Allows other coroutines to run while the asynchronous I/O operation completes, as an
   alternative to blocking while waiting for its completion.

   @param fd
      File descriptor that the calling coroutine is waiting for I/O on.
   @param bWrite
      true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
   */
   void yield_until_fd_ready(io::filedesc_t fd, bool bWrite);

private:
   /*! Finds a coroutine ready to execute; if none are, but there are blocked coroutines, it blocks
   the current thread until one of them becomes ready.

   @return
      Pointer to the context of a coroutine that’s ready to execute.
   */
   std::shared_ptr<coroutine::context> find_coroutine_to_activate();

   /*! Switches context from the coroutine context pointed to by pcoroctxLastActive to the current
   thread’s own context.

   @param pcoroctxLastActive
      Pointer to the coroutine context that is being inactivated.
   */
   void switch_to_scheduler(coroutine::context * pcoroctxLastActive);

private:
#if ABC_HOST_API_BSD
   //! File descriptor of the internal kqueue.
   io::filedesc m_fdKqueue;
   /*! Coroutines that are blocked on a timer wait. The keys are the same as the values, but this
   can’t be changed into a set<shared_ptr<context>> because we need it to hold a strong reference to
   the coroutine context while allowing lookups without having a shared_ptr. */
   collections::map<std::uintptr_t, std::shared_ptr<coroutine::context>> m_mapCorosBlockedByTimer;
#elif ABC_HOST_API_LINUX
   //! File descriptor of the internal epoll.
   io::filedesc m_fdEpoll;
   /*! Timers currently being waited for. The key is the same as the value, but this can’t be
   changed into a set<io::filedesc> until io::filedesc is hashable. */
   collections::map<io::filedesc_t, io::filedesc> m_mapActiveTimers;
#else
   #error "TODO: HOST_API"
#endif
   //! Coroutines that are blocked on a fd wait.
   collections::map<io::filedesc_t, std::shared_ptr<coroutine::context>> m_mapCorosBlockedByFD;
   //! List of coroutines that have been scheduled, but have not been started yet.
   collections::list<std::shared_ptr<coroutine::context>> m_listStartingCoros;
   //! Pointer to the active (current) coroutine, or nullptr if none is active.
   static thread_local_value<std::shared_ptr<coroutine::context>> sm_pcoroctxActive;
   //! Pointer to the coroutine scheduler for the current thread.
   static thread_local_value<std::shared_ptr<scheduler>> sm_pcorosched;
#if ABC_HOST_API_POSIX
   //! Pointer to the context of every thread running a coroutine scheduler.
   static thread_local_value< ::ucontext_t *> sm_puctxReturn;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

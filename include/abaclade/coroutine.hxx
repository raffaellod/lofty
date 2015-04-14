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

#ifndef _ABACLADE_COROUTINE_HXX
#define _ABACLADE_COROUTINE_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine

namespace abc {

/*! Subroutine for use in non-preemptive multitasking, enabling asynchronous I/O in most abc::io
classes. */
class ABACLADE_SYM coroutine : public noncopyable {
private:
   friend class detail::coroutine_scheduler;

public:
   //! OS-dependent execution context for the coroutine.
   class context;
   //! Type of the unique coroutine IDs.
   typedef std::intptr_t id_type;

public:
   /*! Constructor.

   @param fnMain
      Function to invoke once the coroutine is first scheduled.
   @param coro
      Source object.
   */
   coroutine();
   explicit coroutine(std::function<void ()> fnMain);
   coroutine(coroutine && coro) :
      m_pctx(std::move(coro.m_pctx)) {
   }

   //! Destructor.
   ~coroutine();

   /*! Returns a process-wide unique ID for the coroutine.

   @return
      Unique ID representing the coroutine.
   */
   id_type id() const;

private:
   //! Pointer to the coroutine’s execution context.
   std::shared_ptr<context> m_pctx;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::coroutine

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<coroutine> {
public:
   //! Constructor.
   to_str_backend();

   //! Destructor.
   ~to_str_backend();

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

   /*! Writes a string, applying the formatting options.

   @param op
      Path to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(coroutine const & coro, io::text::writer * ptwOut);

protected:
   //! Backend used to write strings.
   to_str_backend<istr> m_tsbStr;
   //! Backend used to write coroutine ID.
   to_str_backend<coroutine::id_type> m_tsbId;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::this_coroutine

namespace abc {
//! Functions that can only affect the current coroutine. Coroutine counterpart to abc::this_thread.
namespace this_coroutine {

/*! Returns a process-wide unique ID for the current coroutine.

@return
   Unique ID representing the current coroutine.
*/
ABACLADE_SYM coroutine::id_type id();

/*! Suspends execution of the current coroutine for at least the specified duration.

@return
   Duration for which the current coroutine should not execute, in milliseconds.
*/
ABACLADE_SYM void sleep_for_ms(unsigned iMilliseconds);

/*! Suspends execution of the current coroutine until an asynchronous I/O operation completes.

@param fd
   File descriptor that the calling coroutine is waiting for I/O on.
@param bWrite
   true if the coroutine is waiting to write to fd, or false if it’s waiting to read from it.
*/
ABACLADE_SYM void sleep_until_fd_ready(io::filedesc_t fd, bool bWrite);
inline void sleep_until_fd_ready(io::filedesc const & fd, bool bWrite) {
   sleep_until_fd_ready(fd.get(), bWrite);
}

} //namespace this_coroutine
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COROUTINE_HXX

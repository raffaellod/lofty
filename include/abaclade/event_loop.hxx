/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#ifndef _ABACLADE_EVENT_LOOP_HXX
#define _ABACLADE_EVENT_LOOP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io/binary/file.hxx>
#include <abaclade/process.hxx>
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::timer

namespace abc {

//! Generates events based on a time interval expressed in milliseconds.
class ABACLADE_SYM timer {
private:
   friend class event_loop;

public:
   //! Constructor.
   timer();

   //! Destructor.
   ~timer();

   void start();

   void stop();

private:
   //! Underlying OS-dependent ID/handle.
#if ABC_HOST_API_BSD
   typedef int m_id;
#elif ABC_HOST_API_LINUX
   typedef int m_fd;
#elif ABC_HOST_API_WIN32
   typedef HANDLE m_h;
#else
   #error "TODO: HOST_API"
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::event_loop

namespace abc {

//! Generic event loop for a single thread.
class ABACLADE_SYM event_loop : public noncopyable {
public:
   /*! Constructor.

   @param lpSrc
      Source event loop.
   */
   event_loop();
   event_loop(event_loop && elSrc) :
      m_pImpl(elSrc.m_pImpl) {
      elSrc.m_pImpl = nullptr;
   }

   //! Destructor.
   ~event_loop();

   /*! Assignment operator.

   @param lpSrc
      Source event loop.
   @return
      *this.
   */
   event_loop & operator=(event_loop && elSrc) {
      this->~event_loop();
      m_pImpl = elSrc.m_pImpl;
      elSrc.m_pImpl = nullptr;
      return *this;
   }

   void add_file_source(
      std::shared_ptr<io::binary::file_base> pfile,
      std::function<void (std::shared_ptr<io::binary::file_base>)> fnHandler
   );
   template <typename T>
   void add_file_source(
      typename std::enable_if<
         std::is_base_of<io::binary::file_base, T>::value, std::shared_ptr<T>
      >::type pfile,
      std::function<void (std::shared_ptr<T>)> fnHandler
   ) {
      add_file_source(
         std::static_pointer_cast<io::binary::file_base>(pfile),
         [pfile, fnHandler] (std::shared_ptr<io::binary::file_base>) {
            fnHandler(pfile);
         }
      );
   }

   void add_process_source(
      std::shared_ptr<process> pproc, std::function<void (std::shared_ptr<process>)> fnHandler
   );

   void add_thread_source(
      std::shared_ptr<thread> pthr, std::function<void (std::shared_ptr<thread>)> fnHandler
   );

   timer add_timer_source(
      std::uint32_t iMilliseconds, std::function<void (timer)> fnHandler
   );

   void run();

private:
   //! Most members depend on OS-specific data types, so this pointer abstracts them away.
   void * m_pImpl;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_EVENT_LOOP_HXX

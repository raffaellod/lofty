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

#ifndef _ABACLADE_THREAD_HXX
#define _ABACLADE_THREAD_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

/*! Thread of program execution. Replacement for std::thread supporting cooperation with
abc::event_loop. */
class ABACLADE_SYM thread : public noncopyable {
public:
   //! Underlying OS-dependent ID/handle type.
#if ABC_HOST_API_POSIX
   typedef pthread_t native_handle_type;
#elif ABC_HOST_API_WIN32
   typedef HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

   //! OS-dependent type for unique thread IDs.
#if ABC_HOST_API_BSD
   typedef ::pthread_id_np_t id_type;
#elif ABC_HOST_API_LINUX
   typedef int id_type;
#elif ABC_HOST_API_WIN32
   typedef DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   /*! Constructor.

   @param thr
      Source thread.
   */
   thread();
   thread(thread && thr) :
      m_h(thr.m_h)
#if ABC_HOST_API_POSIX
      , m_id(thr.m_id)
#endif
   {
#if ABC_HOST_API_POSIX
      thr.m_id = 0;
      // pthreads does not provide a way to clear thr.m_h.
#else
      thr.m_h = nullptr;
#endif
   }

   //! Destructor.
   ~thread();

   /*! Assignment operator.

   @param thr
      Source thread.
   @return
      *this.
   */
   thread & operator=(thread && thr) {
      ABC_TRACE_FUNC(this/*, thr*/);

      native_handle_type h(thr.m_h);
#if ABC_HOST_API_POSIX
      id_type id(thr.m_id);
#endif
      detach();
      m_h = h;
#if ABC_HOST_API_POSIX
      // pthreads does not provide a way to clear thr.m_h.
      m_id = id;
      thr.m_id = 0;
#else
      thr.m_h = nullptr;
#endif
      return *this;
   }

   /*! Equality relational operator.

   @param thr
      Object to compare to *this.
   @return
      true if *this refers to the same thread as thr, or false otherwise.
   */
   bool operator==(thread const & thr) const;

   /*! Inequality relational operator.

   @param thr
      Object to compare to *this.
   @return
      true if *this refers to a different thread than thr, or false otherwise.
   */
   bool operator!=(thread const & thr) const {
      ABC_TRACE_FUNC(this/*, thr*/);

      return !operator==(thr);
   }

   /*! Releases the OS-dependent ID/handle, making *this reference no thread and invalidating the
   value returned by native_handle(). */
   void detach();

   /*! Returns a process-wide unique ID for the thread.

   @return
      Unique ID representing the thread.
   */
   id_type id() const;

   //! Waits for the thread to terminate.
   void join();

   /*! Returns true if calling join() on the object is allowed.

   @return
      true if the object is in a joinable state, or false otherwise.
   */
   bool joinable() const;

   /*! Returns the underlying ID/handle type.

   @return
      OS-dependent ID/handle.
   */
   native_handle_type native_handle() const {
      return m_h;
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_POSIX
   /*! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   Since there’s no “uninitialized” pthread_t value, also use this to track whether m_h is valid. */
   id_type m_id;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_THREAD_HXX

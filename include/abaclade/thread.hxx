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

#ifndef _ABACLADE_THREAD_HXX
#define _ABACLADE_THREAD_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <memory>
#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread

namespace abc {

/*! Thread of program execution. Replacement for std::thread supporting cooperation with
abc::coroutine. */
class ABACLADE_SYM thread : public noncopyable {
public:
   //! Underlying OS-dependent ID/handle type.
#if ABC_HOST_API_POSIX
   typedef ::pthread_t native_handle_type;
#elif ABC_HOST_API_WIN32
   typedef HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

   //! OS-dependent type for unique thread IDs.
#if ABC_HOST_API_DARWIN
   typedef std::uint64_t id_type;
#elif ABC_HOST_API_FREEBSD
   typedef int id_type;
#elif ABC_HOST_API_LINUX
   typedef int id_type;
#elif ABC_HOST_API_WIN32
   typedef DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

private:
   /*! Type used to exchange data between the thread owning the abc::thread instance and the thread
   owned by the abc::thread instance. */
   class shared_data;

public:
   /*! Constructor.

   @param fnMain
      Function that will act as the entry point for a new thread to be started immediately.
   @param thr
      Source thread.
   */
   thread() :
#if ABC_HOST_API_POSIX
      m_id(0) {
#elif ABC_HOST_API_WIN32
      m_h(nullptr) {
#else
   #error "TODO: HOST_API"
#endif
   }
   explicit thread(std::function<void ()> fnMain);
   thread(thread && thr);

   //! Destructor.
   ~thread();

   /*! Assignment operator.

   @param thr
      Source thread.
   @return
      *this.
   */
   thread & operator=(thread && thr);

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
   //! Creates a thread to run outer_main(), with inner_main() available in m_psd.
   void start();

   /*! Lower-level wrapper for the thread function passed to the constructor. Under Linux, this is
   also needed to assign the thread ID to the owning abc::thread instance.

   @param p
      *this, to be used to acquire a pointer to m_psd and, under POSIX, to set m_id.
   @return
      Unused.
   */
#if ABC_HOST_API_POSIX
   static void * outer_main(void * p);
#elif ABC_HOST_API_WIN32
   static DWORD WINAPI outer_main(void * p);
#else
   #error "TODO: HOST_API"
#endif

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
#if ABC_HOST_API_POSIX
   /*! OS-dependent ID for use with OS-specific API (pthread_*_np() functions and other native API).
   Since there’s no “uninitialized” pthread_t value, also use this to track whether m_h is valid. */
   id_type m_id;
#endif
   /*! Pointer to data that is shared between the thread owned by the abc::thread instance and the
   thread owning the abc::thread instance. */
   std::shared_ptr<shared_data> m_psd;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::thread

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<thread> {
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
   void write(thread const & thr, io::text::writer * ptwOut);

protected:
   //! Backend used to write strings.
   to_str_backend<istr> m_tsbStr;
   //! Backend used to write thread ID.
   to_str_backend<thread::id_type> m_tsbId;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::this_thread

namespace abc {

namespace detail {

// Forward declaration.
class coroutine_scheduler;

} //namespace detail

//! Functions that can only affect the current thread. Replacement for std::this_thread.
namespace this_thread {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization
required for the current thread to run coroutines.

@return
   Coroutine scheduler associated to this thread. If pcorosched was non-nullptr, this is the same as
   pcorosched.
*/
ABACLADE_SYM std::shared_ptr<detail::coroutine_scheduler> const & attach_coroutine_scheduler(
   std::shared_ptr<detail::coroutine_scheduler> pcorosched = nullptr
);

/*! Returns the coroutine scheduler associated to the current thread, if any.

@return
   Coroutine scheduler associated to this thread. May be nullptr if attach_coroutine_scheduler() was
   never called for the current thread.
*/
ABACLADE_SYM std::shared_ptr<detail::coroutine_scheduler> const & get_coroutine_scheduler();

/*! Returns a process-wide unique ID for the current thread.

@return
   Unique ID representing the current thread.
*/
ABACLADE_SYM thread::id_type id();

/*! Begins running scheduled coroutines on the current thread. Only returns after every coroutine
scheduled on the same thread or scheduler returns. */
ABACLADE_SYM void run_coroutines();

/*! Suspends execution of the current thread for at least the specified duration.

@return
   Duration for which the current thread should not execute, in milliseconds.
*/
ABACLADE_SYM void sleep_for_ms(unsigned iMilliseconds);

} //namespace this_thread
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_THREAD_HXX

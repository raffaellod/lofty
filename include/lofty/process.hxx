/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_PROCESS_HXX
#define _LOFTY_PROCESS_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Process (“task” on some platforms), typically a child spawned by the current process.
class LOFTY_SYM process : public noncopyable {
public:
   //! Underlying OS-dependent ID/handle type.
#if LOFTY_HOST_API_POSIX
   typedef int native_handle_type;
#elif LOFTY_HOST_API_WIN32
   typedef ::HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

   //! OS-dependent type for unique process IDs.
#if LOFTY_HOST_API_POSIX
   // ID == native handle.
   typedef native_handle_type id_type;
#elif LOFTY_HOST_API_WIN32
   typedef ::DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   //! Default constructor.
   process() :
      handle(null_handle) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   process(process && src) :
      handle(src.handle) {
      src.handle = null_handle;
   }

   /*! Constructor.

   @param pid
      PID of a running process to associate this lofty::process instance with.
   */
   explicit process(id_type pid);

   //! Destructor.
   ~process();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   process & operator=(process && src) {
      if (&src != this) {
         detach();
         handle = src.handle;
         src.handle = null_handle;
      }
      return *this;
   }

   /*! Equality relational operator.

   @param other
      Object to compare to *this.
   @return
      true if *this has the same id() as other, or false otherwise.
   */
   bool operator==(process const & other) const {
      return id() == other.id();
   }

   /*! Inequality relational operator.

   @param other
      Object to compare to *this.
   @return
      true if *this has a different id() than other, or false otherwise.
   */
   bool operator!=(process const & other) const {
      return !operator==(other);
   }

   /*! Releases the OS-dependent ID/handle, making *this reference no process and invalidating the value
   returned by native_handle(). */
   void detach();

   /*! Returns a system-wide unique ID for the process.

   @return
      Unique ID representing the process.
   */
   id_type id() const;

   /*! Waits for the process to terminate, returning its exit code.

   @return
      Exit code of the process. On POSIX, a negative value -N indicates that the process was terminated by
      signal N.
   */
   int join();

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
      return handle;
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type handle;
   //! Logically null ID/handle.
   static native_handle_type const null_handle;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<process> : public to_text_ostream<process::id_type> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes a process’ identifier, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(process const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Functions that can only affect the current process, analogous to this_thread.
namespace this_process {}

} //namespace lofty

namespace lofty { namespace this_process {

/*! Returns the value of an environment variable for the process.

@param name
   Name of the environment variable to retrieve.
@param ret
   Pointer to a string to receive the value of the environment variable.
@return
   true if the specified variable was found in the environment, or false if it wasn’t.
*/
LOFTY_SYM bool env_var(str const & name, str * ret);

/*! Returns the value of an environment variable for the process.

@param name
   Name of the environment variable to retrieve.
@return
   A tuple containing (value, true) if the specified variable was found in the environment, or (str::empty,
   false) if it wasn’t.
*/
LOFTY_SYM _std::tuple<str, bool> env_var(str const & name);

/*! Returns a system-wide unique ID for the current process.

@return
   Unique ID representing the current process.
*/
LOFTY_SYM process::id_type id();

}} //namespace lofty::this_process

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_PROCESS_HXX

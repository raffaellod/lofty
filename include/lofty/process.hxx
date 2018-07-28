/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_PROCESS_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_PROCESS_HXX
#endif

#ifndef _LOFTY_PROCESS_HXX_NOPUB
#define _LOFTY_PROCESS_HXX_NOPUB

#include <lofty/_std/tuple.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

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

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<_LOFTY_PUBNS process> :
   public to_text_ostream<_LOFTY_PUBNS process::id_type> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(text::_LOFTY_PUBNS str const & format);

   /*! Writes a process’ identifier, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(_LOFTY_PUBNS process const & src, io::text::_LOFTY_PUBNS ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Functions that can only affect the current process, analogous to this_thread.
namespace this_process {}

} //namespace lofty

namespace lofty { namespace this_process {
_LOFTY_PUBNS_BEGIN

/*! Returns the value of an environment variable for the process.

@param name
   Name of the environment variable to retrieve.
@param ret
   Pointer to a string to receive the value of the environment variable.
@return
   true if the specified variable was found in the environment, or false if it wasn’t.
*/
LOFTY_SYM bool env_var(text::_LOFTY_PUBNS str const & name, text::_LOFTY_PUBNS str * ret);

/*! Returns the value of an environment variable for the process.

@param name
   Name of the environment variable to retrieve.
@return
   A tuple containing (value, true) if the specified variable was found in the environment, or (str::empty,
   false) if it wasn’t.
*/
LOFTY_SYM _std::_LOFTY_PUBNS tuple<text::_LOFTY_PUBNS str, bool> env_var(text::_LOFTY_PUBNS str const & name);

/*! Returns a system-wide unique ID for the current process.

@return
   Unique ID representing the current process.
*/
LOFTY_SYM lofty::_LOFTY_PUBNS process::id_type id();

_LOFTY_PUBNS_END
}} //namespace lofty::this_process

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_PROCESS_HXX_NOPUB

#ifdef _LOFTY_PROCESS_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::process;

   }

   namespace lofty { namespace this_process {

   using _pub::env_var;
   using _pub::id;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_PROCESS_HXX

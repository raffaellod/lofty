/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_PROCESS_HXX
#define _ABACLADE_PROCESS_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Process (“task” on some platforms), typically a child spawned by the current process.
class ABACLADE_SYM process : public noncopyable {
public:
   //! Underlying OS-dependent ID/handle type.
#if ABC_HOST_API_POSIX
   typedef int native_handle_type;
#elif ABC_HOST_API_WIN32
   typedef ::HANDLE native_handle_type;
#else
   #error "TODO: HOST_API"
#endif

   //! OS-dependent type for unique process IDs.
#if ABC_HOST_API_POSIX
   // ID == native handle.
   typedef native_handle_type id_type;
#elif ABC_HOST_API_WIN32
   typedef ::DWORD id_type;
#else
   #error "TODO: HOST_API"
#endif

public:
   //! Default constructor.
   process() :
      m_h(smc_hNull) {
   }

   /*! Move constructor.

   @param proc
      Source object.
   */
   process(process && proc) :
      m_h(proc.m_h) {
      proc.m_h = smc_hNull;
   }

   /*! Constructor.

   @param pid
      PID of a running process to associate this abc::process instance with.
   */
   explicit process(id_type pid);

   //! Destructor.
   ~process();

   /*! Move-assignment operator.

   @param proc
      Source process.
   @return
      *this.
   */
   process & operator=(process && proc) {
      ABC_TRACE_FUNC(this/*, proc*/);

      native_handle_type h(proc.m_h);
      detach();
      m_h = h;
      proc.m_h = smc_hNull;
      return *this;
   }

   /*! Equality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has the same id() as proc, or false otherwise.
   */
   bool operator==(process const & proc) const {
      ABC_TRACE_FUNC(this/*, proc*/);

      return id() == proc.id();
   }

   /*! Inequality relational operator.

   @param proc
      Object to compare to *this.
   @return
      true if *this has a different id() than proc, or false otherwise.
   */
   bool operator!=(process const & proc) const {
      ABC_TRACE_FUNC(this/*, proc*/);

      return !operator==(proc);
   }

   /*! Releases the OS-dependent ID/handle, making *this reference no process and invalidating the
   value returned by native_handle(). */
   void detach();

   /*! Returns a system-wide unique ID for the process.

   @return
      Unique ID representing the process.
   */
   id_type id() const;

   /*! Waits for the process to terminate, returning its exit code.

   @return
      Exit code of the process. On POSIX, a negative value -N indicates that the process was
      terminated by signal N.
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
      ABC_TRACE_FUNC(this);

      return m_h;
   }

private:
   //! OS-dependent ID/handle.
   native_handle_type m_h;
   //! Logically null ID/handle.
   static native_handle_type const smc_hNull;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_str_backend<process> : public to_str_backend<process::id_type> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes a process’ identifier, applying the formatting options.

   @param proc
      Process to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(process const & proc, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Functions that can only affect the current process, analogous to this_thread.
namespace this_process {}

} //namespace abc

namespace abc { namespace this_process {

/*! Returns the value of an environment variable for the process.

@param sName
   Name of the environment variable to retrieve.
@param psRet
   Pointer to a string to receive the value of the environment variable.
@return
   true if the specified variable was found in the environment, or false if it wasn’t.
*/
ABACLADE_SYM bool env_var(str const & sName, str * psRet);

/*! Returns the value of an environment variable for the process.

@param sName
   Name of the environment variable to retrieve.
@return
   A tuple containing (value, true) if the specified variable was found in the environment, or
   (str::empty, false) if it wasn’t.
*/
ABACLADE_SYM _std::tuple<str, bool> env_var(str const & sName);

/*! Returns a system-wide unique ID for the current process.

@return
   Unique ID representing the current process.
*/
ABACLADE_SYM process::id_type id();

}} //namespace abc::this_process

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PROCESS_HXX

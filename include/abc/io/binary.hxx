﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABC_IO_BINARY_HXX
#define _ABC_IO_BINARY_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/enum.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals

namespace abc {

namespace io {

// Some C libraries (such as MS CRT) define these as macros.
#ifdef stdin
   #undef stdin
   #undef stdout
   #undef stderr
#endif

/** List of standard (OS-provided) files.
*/
ABC_ENUM(stdfile, \
   /** Internal identifier for stdin. */ \
   (stdin,  0), \
   /** Internal identifier for stdout. */ \
   (stdout, 1), \
   /** Internal identifier for stderr. */ \
   (stderr, 2) \
);


/** File access modes.
*/
ABC_ENUM(access_mode, \
   /** Read-only access. */ \
   (read,       1), \
   /** Write-only access. */ \
   (write,      2), \
   /** Read/write access. */ \
   (read_write, 3), \
   /** Append-only access. */ \
   (append,     4) \
);

} //namespace io

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_base


namespace abc {

namespace io {

/** Abstract base for binary (non-text) I/O classes.
*/
class ABCAPI binary_base :
   public noncopyable {
public:

   /** Destructor.
   */
   virtual ~binary_base();


protected:

   /** Constructor.
   */
   binary_base();
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_reader


namespace abc {

namespace io {

/** Abstract binary (non-text) input.
*/
class ABCAPI binary_reader :
   public virtual binary_base {
public:

   /** Destructor.
   */
   virtual ~binary_reader();


   /** Reads at most cbMax bytes.

   p
      Address of the destination buffer.
   cbMax
      Size of the destination buffer, in bytes.
   return
      Count of bytes read. For non-zero values of cb, a return value of 0 indicates that the end of
      the data was reached.
   */
   virtual size_t read(void * p, size_t cbMax) = 0;


protected:

   /** See binary_base::binary_base().
   */
   binary_reader();
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_writer


namespace abc {

namespace io {

/** Abstract binary (non-text) output.
*/
class ABCAPI binary_writer :
   public virtual binary_base {
public:

   /** Destructor.
   */
   virtual ~binary_writer();


   /** Forces writing any data in the write buffer.
   */
   virtual void flush() = 0;


   /** Writes an array of bytes.

   p
      Address of the source buffer.
   cb
      Size of the source buffer, in bytes.
   return
      Count of bytes written.
   */
   virtual size_t write(void const * p, size_t cb) = 0;


protected:

   /** See binary_base::binary_base().
   */
   binary_writer();
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_IO_BINARY_HXX


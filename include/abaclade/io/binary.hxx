/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::base


namespace abc {
namespace io {
namespace binary {

//! Base interface for binary (non-text) I/O.
class ABACLADE_SYM base {
public:

   //! Destructor. Also needed to make the class polymorphic (have a vtable).
   virtual ~base();
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::reader


namespace abc {
namespace io {
namespace binary {

//! Interface for binary (non-text) input.
class ABACLADE_SYM reader :
   public virtual base {
public:

   /*! Reads at most cbMax bytes.

   p
      Address of the destination buffer.
   cbMax
      Size of the destination buffer, in bytes.
   return
      Count of bytes read. For non-zero values of cbMax, a return value of 0 indicates that the end
      of the data was reached.
   */
   virtual std::size_t read(void * p, std::size_t cbMax) = 0;
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::writer


namespace abc {
namespace io {
namespace binary {

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer :
   public virtual base {
public:

   //! Forces writing any data in the write buffer.
   virtual void flush() = 0;


   /*! Writes an array of bytes.

   p
      Address of the source buffer.
   cb
      Size of the source buffer, in bytes.
   return
      Count of bytes written.
   */
   virtual std::size_t write(void const * p, std::size_t cb) = 0;
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::seekable


namespace abc {
namespace io {
namespace binary {

//! Interface for binary I/O classes that allow random access (e.g. seek/tell operations).
class ABACLADE_SYM seekable {
public:

   /*! Changes the current read/write position.

   iOffset
      New position, relative to sfWhence.
   sfWhence
      Indicates what position iOffset is relative to.
   return
      Resulting position.
   */
   virtual offset_t seek(offset_t ibOffset, seek_from sfWhence) = 0;


   /*! Returns the current read/write position.

   return
      Current position.
   */
   virtual offset_t tell() const = 0;
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary::sized


namespace abc {
namespace io {
namespace binary {

//! Interface for binary I/O classes that access data with a known size.
class ABACLADE_SYM sized {
public:

   /*! Returns the size of the data.

   return
      Data size, in bytes.
   */
   virtual full_size_t size() const = 0;
};

} //namespace binary
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


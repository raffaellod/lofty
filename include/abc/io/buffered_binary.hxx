/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#ifndef _ABC_IO_BUFFERED_BINARY_HXX
#define _ABC_IO_BUFFERED_BINARY_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/io/binary.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals

namespace abc {

namespace io {

/** Creates and returns a buffered wrapper for the specified binary I/O object.

pbb
   Pointer to a binary I/O object.
return
   Pointer to a buffered wrapper for *pbb.
*/
std::shared_ptr<binary_base> buffer_binary(std::shared_ptr<binary_base> pbb);


/** Creates and returns a buffered reader wrapper for the specified unbuffered binary reader.

pbr
   Pointer to an unbuffered binary reader.
return
   Pointer to a buffered wrapper for *pbr.
*/
inline std::shared_ptr<binary_reader> buffer_binary_reader(std::shared_ptr<binary_reader> pbr) {
   return std::dynamic_pointer_cast<binary_reader>(buffer_binary(std::move(pbr)));
}


/** Creates and returns a buffered writer wrapper for the specified unbuffered binary writer.

pbw
   Pointer to an unbuffered binary writer.
return
   Pointer to a buffered wrapper for *pbw.
*/
inline std::shared_ptr<binary_writer> buffer_binary_writer(std::shared_ptr<binary_writer> pbw) {
   return std::dynamic_pointer_cast<binary_writer>(buffer_binary(std::move(pbw)));
}

} //namespace io

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::buffered_binary_base


namespace abc {

namespace io {

/** Interface for buffering objects that wrap binary_* instances.
*/
class ABCAPI buffered_binary_base :
   public virtual binary_base {
public:

   /** Returns a pointer to the wrapper unbuffered binary I/O object.
   */
   virtual std::shared_ptr<binary_base> unbuffered() const = 0;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_reader


namespace abc {

namespace io {

/** Provides buffering on top of a binary_reader instance.
*/
class ABCAPI default_buffered_binary_reader :
   public virtual buffered_binary_base,
   public binary_reader {
public:

   /** See binary_reader::binary_reader().
   */
   default_buffered_binary_reader(std::shared_ptr<binary_reader> pbr);


   /** Destructor.
   */
   virtual ~default_buffered_binary_reader();


   /** See binary_reader::read().
   */
   virtual size_t read(void * p, size_t cbMax);


   /** See buffered_binary_base::unbuffered().
   */
   virtual std::shared_ptr<binary_base> unbuffered() const;


protected:

   /** Wrapped binary reader. */
   std::shared_ptr<binary_reader> m_pbr;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_writer


namespace abc {

namespace io {

/** Provides buffering on top of a binary_writer instance.
*/
class ABCAPI default_buffered_binary_writer :
   public virtual buffered_binary_base,
   public binary_writer {
public:

   /** See binary_writer::binary_writer().
   */
   default_buffered_binary_writer(std::shared_ptr<binary_writer> pbw);


   /** Destructor.
   */
   virtual ~default_buffered_binary_writer();


   /** See binary_writer::flush().
   */
   virtual void flush();


   /** See buffered_binary_base::unbuffered().
   */
   virtual std::shared_ptr<binary_base> unbuffered() const;


   /** See binary_writer::write().
   */
   virtual size_t write(void const * p, size_t cb);


protected:

   /** Wrapped binary writer. */
   std::shared_ptr<binary_writer> m_pbw;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_IO_BUFFERED_BINARY_HXX


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
// abc::io::buffered_binary_reader


namespace abc {

namespace io {

/** Provides buffering on top of a binary_reader instance.
*/
class ABCAPI buffered_binary_reader :
   public binary_reader {
public:

   /** See binary_reader::binary_reader().
   */
   buffered_binary_reader(std::shared_ptr<binary_reader> pbr);


   /** Destructor.
   */
   virtual ~buffered_binary_reader();


   /** See binary_reader::read().
   */
   virtual size_t read(void * p, size_t cbMax);


protected:

   /** Wrapped binary reader. */
   std::shared_ptr<binary_reader> m_pbr;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::buffered_binary_writer


namespace abc {

namespace io {

/** Provides buffering on top of a binary_writer instance.
*/
class ABCAPI buffered_binary_writer :
   public binary_writer {
public:

   /** See binary_writer::binary_writer().
   */
   buffered_binary_writer(std::shared_ptr<binary_writer> pbw);


   /** Destructor.
   */
   virtual ~buffered_binary_writer();


   /** See binary_writer::flush().
   */
   virtual void flush();


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


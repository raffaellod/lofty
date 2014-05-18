/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::str_istream


namespace abc {

namespace io {

/** Implementation of an read-only stream based on a string.
*/
class ABCAPI str_istream :
   public virtual istream {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit str_istream(istr const & s);
   explicit str_istream(istr && s);
   explicit str_istream(mstr && s);
   explicit str_istream(dmstr && s);


   /** Destructor.
   */
   virtual ~str_istream();


   /** See istream::read_line().
   */
   virtual istream & read_line(mstr * ps);


   /** See istream::read_raw().
   */
   virtual size_t read_raw(
      void * p, size_t cbMax, text::encoding enc = text::encoding::identity
   );


   /** See istream::unread_raw().
   */
   virtual void unread_raw(void const * p, size_t cb);


protected:

   /** Source string. */
   istr m_sBuf;
   /** Current read offset into the string, in bytes. Seeks can only change this in increments of a
   character, but internal code doesn’t have to. */
   size_t m_ibRead;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::str_ostream


namespace abc {

namespace io {

/** Implementation of an write-only stream based on a string.
*/
class ABCAPI str_ostream :
   public virtual ostream {
public:

   /** Type of the string used as buffer. */
   typedef dmstr str_type;

public:

   /** Constructor.
   */
   str_ostream();


   /** Destructor.
   */
   virtual ~str_ostream();


   /** Yields ownership of the string buffer.

   return
      Former content of the stream.
   */
   str_type release_content();


   /** See ostream::write_raw().
   */
   virtual void write_raw(
      void const * p, size_t cb, text::encoding enc = text::encoding::identity
   );


protected:

   /** Target string. */
   str_type m_sBuf;
   /** Current write offset into the string, in bytes. Seeks can only change this in increments of a
   character, but internal code doesn’t have to. */
   size_t m_ibWrite;
};


} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


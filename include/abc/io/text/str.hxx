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
// abc::io::text::str_base


namespace abc {
namespace io {
namespace text {

/** Implementation of text (character-based) I/O from/to a string.
*/
class ABCAPI str_base :
   public virtual base {
public:

   /** Destructor.
   */
   virtual ~str_base();


   /** See base::encoding().
   */
   virtual abc::text::encoding encoding() const;


   /** See base::line_terminator().
   */
   virtual abc::text::line_terminator line_terminator() const;


protected:

   /** Constructor.

   lterm
      Initial value for line_terminator().
   */
   explicit str_base(abc::text::line_terminator lterm = abc::text::line_terminator::host);


protected:

   /** Current read/write offset into the string. */
   uintptr_t m_ichOffset;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_reader


namespace abc {
namespace io {
namespace text {

/** Implementation of text (character-based) input from a string.
*/
class ABCAPI str_reader :
   public virtual str_base,
   public virtual reader {
public:

   /** Constructor.

   s
      Source string.
   lterm
      Initial value for line_terminator().
   */
   explicit str_reader(
      istr const & s, abc::text::line_terminator lterm = abc::text::line_terminator::host
   );
   explicit str_reader(
      istr && s, abc::text::line_terminator lterm = abc::text::line_terminator::host
   );
   explicit str_reader(
      mstr && s, abc::text::line_terminator lterm = abc::text::line_terminator::host
   );


   /** See reader::read_while().
   */
   virtual bool read_while(mstr * ps, std::function<char_t const * (
      char_t const * pchBegin, char_t const * pchLastReadBegin, char_t const * pchEnd
   )> fnGetConsumeEnd);


protected:

   /** Pointer to the source string. Normally points to m_sReadBuf, but subclasses may change that
   as needed. */
   istr const * m_psReadBuf;
   /** Target of m_psReadBuf, unless overridden by subclasses. */
   istr m_sReadBuf;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::str_writer


namespace abc {
namespace io {
namespace text {

/** Implementation of text (character-based) output into a string.
*/
class ABCAPI str_writer :
   public virtual str_base,
   public virtual writer {
public:

   /** Constructor.

   psBuf
      Pointer to a mutable string to use as the destination of all writes. If omitted, an internal
      string will be used.
   lterm
      Initial value for line_terminator().
   */
   explicit str_writer(
      mstr * psBuf = nullptr, abc::text::line_terminator lterm = abc::text::line_terminator::host
   );


   /** Truncates the internal buffer so that the next write will occur at offset 0.
   */
   void clear();


   /** Returns the internal string buffer as a read-only string.

   return
      Content of the writer.
   */
   istr const & get_str() const {
      return *m_psWriteBuf;
   }


   /** Yields ownership of the internal string buffer. If the str_writer instance was constructed
   based on an external string, all internal variables will be successfully reset, but the result
   will be an empty string; the accumulated data will only be accessible through the external
   string.

   return
      Former content of the writer.
   */
   dmstr release_content();


   /** See writer::write_binary().
   */
   virtual void write_binary(void const * p, size_t cb, abc::text::encoding enc);


protected:

   /** Pointer to the destination string. */
   mstr * m_psWriteBuf;
   /** Default target of m_psWriteBuf, if none is supplied via constructor. */
   dmstr m_sDefaultWriteBuf;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


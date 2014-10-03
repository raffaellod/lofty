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

#ifndef _ABACLADE_HXX_INTERNAL
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error

namespace abc {

//! The syntax for the specified expression is invalid.
class ABACLADE_SYM syntax_error : public virtual generic_error, public exception::extended_info {
public:
   /*! Constructor.

   x
      Source object.
   */
   syntax_error();
   syntax_error(syntax_error const & x);

   //! Assignment operator. See abc::generic_error::operator=().
   syntax_error & operator=(syntax_error const & x);

   /*! See abc::generic_error::init().

   All arguments are optional, and can be specified leaving defaulted gaps in between; the resulting
   exception message will not contain omitted arguments.

   The order of line and character is inverted, so that this single overload can be used to
   differentiate between cases in which pszSource is the single line containing the failing
   expression (the thrower would not pass iLine) and cases where pszSource is the source file
   containing the error (the thrower would pass the non-zero line number).

   Examples:

      syntax_error(ABC_SL("expression cannot be empty"))
      syntax_error(ABC_SL("unmatched '{'"), sExpr, iChar)
      syntax_error(ABC_SL("expected expression"), istr::empty, iChar, iLine)
      syntax_error(ABC_SL("unexpected end of file"), opSource, iChar, iLine)

   sDescription
      Description of the syntax error.
   sSource
      Source of the syntax error (whole or individual line).
   iChar
      Character at which the error is located.
   iLine
      Line where the error is located.
   */
   void init(
      istr const & sDescription = istr::empty, istr const & sSource = istr::empty,
      unsigned iChar = 0, unsigned iLine = 0, errint_t err = 0
   );

protected:
   //! See exception::extended_info::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Description of the syntax error.
   istr m_sDescription;
   //! Source of the syntax error (whole or individual line).
   istr m_sSource;
   //! Character at which the error is located.
   unsigned m_iChar;
   //! Line where the error is located.
   unsigned m_iLine;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::source_location

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<source_location> {
public:
   /*! Changes the output format.

   sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

   /*! Writes a source location, applying the formatting options.

   srcloc
      Source location to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(source_location const & srcloc, io::text::writer * ptwOut);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////


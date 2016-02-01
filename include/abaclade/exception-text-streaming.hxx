/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! The syntax for the specified expression is invalid.
class ABACLADE_SYM syntax_error : public generic_error {
public:
   /*! Constructor.

   Most arguments are optional, and can be specified leaving defaulted gaps in between; the resulting
   exception message will not contain omitted arguments.

   The order of line and character is inverted, so that this single overload can be used to
   differentiate between cases in which pszSource is the single line containing the failing
   expression (the thrower would not pass iLine) and cases where pszSource is the source file
   containing the error (the thrower would pass the non-zero line number).

   Examples:

      syntax_error(ABC_SL("expression cannot be empty"))
      syntax_error(ABC_SL("unmatched '{'"), sExpr, iChar)
      syntax_error(ABC_SL("expected expression"), str::empty, iChar, iLine)
      syntax_error(ABC_SL("unexpected end of file"), opSource, iChar, iLine)

   @param sDescription
      Description of the syntax error.
   @param sSource
      Source of the syntax error (whole or individual line).
   @param iChar
      Character at which the error is located.
   @param iLine
      Line where the error is located.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit syntax_error(
      str const & sDescription, str const & sSource = str::empty, unsigned iChar = 0,
      unsigned iLine = 0, errint_t err = 0
   );

   /*! Copy constructor.

   @param x
      Source object.
   */
   syntax_error(syntax_error const & x);

   //! Destructor.
   virtual ~syntax_error() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   syntax_error & operator=(syntax_error const & x);

private:
   //! Description of the syntax error.
   str m_sDescription;
   //! Source of the syntax error (whole or individual line).
   str m_sSource;
   //! Character at which the error is located.
   unsigned m_iChar;
   //! Line where the error is located.
   unsigned m_iLine;
};

} //namespace abc

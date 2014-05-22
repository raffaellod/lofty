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

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error


namespace abc {

/** The syntax for the specified expression is invalid.
*/
class ABCAPI syntax_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   syntax_error();
   syntax_error(syntax_error const & x);


   /** Assignment operator. See abc::generic_error::operator=().
   */
   syntax_error & operator=(syntax_error const & x);


   /** See abc::generic_error::init().

   All arguments are optional, and can be specified leaving defaulted gaps in between; the resulting
   exception message will not contain omitted arguments.

   The order of line and character is inverted, so that this single overload can be used to
   differentiate between cases in which pszSource is the single line containing the failing
   expression (the thrower would not pass iLine) and cases where pszSource is the source file
   containing the error (the thrower would pass the non-zero line number).

   Examples:

      syntax_error(SL("expression cannot be empty"))
      syntax_error(SL("unmatched '{'"), sExpr, iChar)
      syntax_error(SL("expected expression"), istr(), iChar, iLine)
      syntax_error(SL("unexpected end of file"), fpSource, iChar, iLine)

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
      istr const & sDescription = istr(), istr const & sSource = istr(), unsigned iChar = 0,
      unsigned iLine = 0, errint_t err = 0
   );


protected:

   /** See exception::_print_extended_info().
   */
   virtual void _print_extended_info(io::text::writer * ptw) const;


private:

   /** Description of the syntax error. */
   istr m_sDescription;
   /** Source of the syntax error (whole or individual line). */
   istr m_sSource;
   /** Character at which the error is located. */
   unsigned m_iChar;
   /** Line where the error is located. */
   unsigned m_iLine;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


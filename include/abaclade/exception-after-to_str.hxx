/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
   #error "Please #include <abaclade.hxx> instead of this file"
#endif

#if ABC_HOST_UTF > 8
   #include <abaclade/collections/vector.hxx>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Thrown when an instance of a class with a finalize() method was destructed before finalize() was
called on it. The owner ot the object should be changed to invoke finalize() before letting the
object go out of scope. */
class ABACLADE_SYM destructing_unfinalized_object : public exception {
public:
   //! Default constructor.
   destructing_unfinalized_object();

   /*! Copy constructor.

   @param x
      Source object.
   */
   destructing_unfinalized_object(destructing_unfinalized_object const & x);

   //! Destructor.
   virtual ~destructing_unfinalized_object();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   destructing_unfinalized_object & operator=(destructing_unfinalized_object const & x);

   /*! See abc::exception::init().

   @param ptObj
      Pointer to the object that was not finalized.
   */
   template <typename T>
   void init(T const * ptObj) {
      init(ptObj, typeid(*ptObj));
   }

   /*! See abc::exception::init().

   @param pObj
      Pointer to the object that was not finalized.
   @param pti
      Type of *pObj.
   */
   void init(void const * pObj, _std::type_info const & pti);

protected:
   //! See abc::exception::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
#if ABC_HOST_UTF > 8
   //! Same data as m_sWhat but in ASCII, pointed to by abc::exception::m_pszWhat.
   collections::vector<std::uint8_t> m_vchWhat;
#endif
   //! Pointer to and type of the object that was not finalized.
   str m_sWhat;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! The syntax for the specified expression is invalid.
class ABACLADE_SYM syntax_error : public generic_error {
public:
   //! Default constructor.
   syntax_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   syntax_error(syntax_error const & x);

   //! Destructor.
   virtual ~syntax_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
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
   */
   void init(
      str const & sDescription = str::empty, str const & sSource = str::empty,
      unsigned iChar = 0, unsigned iLine = 0, errint_t err = 0
   );

protected:
   //! See generic_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

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

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<source_location> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes a source location, applying the formatting options.

   @param srcloc
      Source location to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(source_location const & srcloc, io::text::writer * ptwOut);
};

} //namespace abc

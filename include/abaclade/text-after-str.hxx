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
// abc::text::error

namespace abc {
namespace text {

//! A text encoding or decoding error occurred.
class ABACLADE_SYM error : public virtual generic_error {
public:
   //! Constructor.
   error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::decode_error

namespace abc {
namespace text {

//! A text decoding error occurred.
class ABACLADE_SYM decode_error : public virtual error, public exception::extended_info {
public:
   /*! Constructor.

   @param x
      Source object.
   */
   decode_error();
   decode_error(decode_error const & x);

   //! Assignment operator. See abc::text::error::operator=().
   decode_error & operator=(decode_error const & x);

   /*! See abc::text::error::init().

   @param sDescription
      Description of the encountered problem.
   @param pbInvalidBegin
      Pointer to the start of the byte sequence that caused the error.
   @param pbInvalidEnd
      Pointer to the end of the byte sequence that caused the error.
   */
   void init(
      istr const & sDescription = istr::empty, std::uint8_t const * pbInvalidBegin = nullptr,
      std::uint8_t const * pbInvalidEnd = nullptr, errint_t err = 0
   );

protected:
   //! See exception::extended_info::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Description of the encountered problem.
   istr m_sDescription;
   //! Bytes that caused the error.
   smvector<std::uint8_t, 16> m_viInvalid;
};

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::encode_error

namespace abc {
namespace text {

//! A text encoding error occurred.
class ABACLADE_SYM encode_error : public virtual error, public exception::extended_info {
public:
   /*! Constructor.

   @param x
      Source object.
   */
   encode_error();
   encode_error(encode_error const & x);

   //! Assignment operator. See abc::text::error::operator=().
   encode_error & operator=(encode_error const & x);

   /*! See abc::text::error::init().

   @param sDescription
      Description of the encountered problem.
   @param chInvalid
      Code point that caused the error.
   */
   void init(
      istr const & sDescription = istr::empty, char32_t chInvalid = 0xffffff, errint_t err = 0
   );

protected:
   //! See exception::extended_info::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Description of the encountered problem.
   istr m_sDescription;
   /*! Code point that caused the error. Not a char32_t because if there’s anything wrong with it,
   we don’t want to find out when trying to print it in write_extended_info(). */
   std::uint32_t m_iInvalidCodePoint;
};

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

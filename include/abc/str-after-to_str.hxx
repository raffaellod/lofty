﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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
// abc::_str_to_str_backend


namespace abc {

/** Base class for the specializations of to_str_backend for string types. Not using templates, so
the implementation can be in a cxx file. This is used by string literal types as well (see below).
*/
class ABCAPI _str_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   _str_to_str_backend(istr const & sFormat);


protected:

   /** Writes a string, applying the formatting options.

   p
      Pointer to the string to write.
   cb
      Size of the string pointed to by p, in bytes.
   enc
      Text encoding of the string pointed to by p.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(void const * p, size_t cb, text::encoding enc, io::text::writer * ptwOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for character literal types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C) \
   /** Character literal. \
   */ \
   template <> \
   class ABCAPI to_str_backend<C> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Constructor.

      sFormat
         Formatting options.
      */ \
      to_str_backend(istr const & sFormat = istr()) : \
         _str_to_str_backend(sFormat) { \
      } \
   \
   \
      /** Writes a character, applying the formatting options.

      ch
         Character to write.
      ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C ch, io::text::writer * ptwOut) { \
         _str_to_str_backend::write(&ch, sizeof(C), text::utf_traits<C>::host_encoding, ptwOut); \
      } \
   };
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char)
// Specialization for wchar_t, if it’s what char16_t or char32_t map to.
#if ABC_CXX_CHAR16 == 1 || ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t)
#endif
// Specializations for char16/32_t, if they’re native types.
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t)
#endif
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for string literal types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C) \
   /** String literal. \
   */ \
   template <size_t t_cch> \
   class to_str_backend<C [t_cch]> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Constructor.

      sFormat
         Formatting options.
      */ \
      to_str_backend(istr const & sFormat = istr()) : \
         _str_to_str_backend(sFormat) { \
      } \
   \
   \
      /** Writes a string, applying the formatting options.

      ach
         String to write.
      ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C const (& ach)[t_cch], io::text::writer * ptwOut) { \
         ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated")); \
         _str_to_str_backend::write( \
            ach, sizeof(C) * (t_cch - 1 /*NUL*/), text::utf_traits<C>::host_encoding, ptwOut \
         ); \
      } \
   };
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char)
// Specialization for wchar_t, if it’s what char16_t or char32_t map to.
#if ABC_CXX_CHAR16 == 1 || ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t)
#endif
// Specializations for char16/32_t, if they’re native types.
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t)
#endif
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::str_base


namespace abc {

template <>
class ABCAPI to_str_backend<str_base> :
   public _str_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      _str_to_str_backend(sFormat) {
   }


   /** Writes a string, applying the formatting options.

   s
      String to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(str_base const & s, io::text::writer * ptwOut) {
      _str_to_str_backend::write(
         s.cbegin().base(),
         size_t(reinterpret_cast<int8_t const *>(s.cend().base()) -
            reinterpret_cast<int8_t const *>(s.cbegin().base())),
         text::encoding::host, ptwOut
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::istr


namespace abc {

template <>
class ABCAPI to_str_backend<istr> :
   public to_str_backend<str_base> {
public:

   /** Constructor. See to_str_backend<str_base>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<str_base>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::mstr


namespace abc {

template <>
class ABCAPI to_str_backend<mstr> :
   public to_str_backend<str_base> {
public:

   /** Constructor. See to_str_backend<str_base>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<str_base>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::dmstr


namespace abc {

template <>
class ABCAPI to_str_backend<dmstr> :
   public to_str_backend<str_base> {
public:

   /** Constructor. See to_str_backend<str_base>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<str_base>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::smstr


namespace abc {

template <size_t t_cchStatic>
class to_str_backend<smstr<t_cchStatic>> :
   public to_str_backend<str_base> {
public:

   /** Constructor. See to_str_backend<str_base>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<str_base>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend


namespace abc {

/** Base class for the specializations of to_str_backend for string types. Not using templates, so
the implementation can be in a cxx file. This is used by string literal types as well (see below).
*/
class ABACLADE_SYM _str_to_str_backend {
public:

   /** Changes the output format.

   sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);


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
// abc::to_str_backend – specialization for character and string literal types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C, enc) \
   /** Character literal. \
   */ \
   template <> \
   class to_str_backend<C> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Writes a character, applying the formatting options.

      ch
         Character to write.
      ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C ch, io::text::writer * ptwOut) { \
         _str_to_str_backend::write(&ch, sizeof(C), enc, ptwOut); \
      } \
   }; \
   \
   /** String literal. \
   */ \
   template <size_t t_cch> \
   class to_str_backend<C [t_cch]> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Writes a string, applying the formatting options.

      ach
         String to write.
      ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C const (& ach)[t_cch], io::text::writer * ptwOut) { \
         ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated")); \
         _str_to_str_backend::write(ach, sizeof(C) * (t_cch - 1 /*NUL*/), enc, ptwOut); \
      } \
   }; \
   \
   /** MSC16 BUG: this partial specialization is necessary. */ \
   template <size_t t_cch> \
   class to_str_backend<C const [t_cch]> : public to_str_backend<C [t_cch]> {};
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char, text::encoding::utf8)
// Specializations for wchar_t, if it’s what char16_t or char32_t map to, and for char16/32_t, if
// they’re native types.
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t, text::encoding::utf16_host)
#elif ABC_CXX_CHAR16 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t, text::encoding::utf16_host)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t, text::encoding::utf32_host)
#elif ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t, text::encoding::utf32_host)
#endif
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::str_base


namespace abc {

template <>
class ABACLADE_SYM to_str_backend<str_base> :
   public _str_to_str_backend {
public:

   /** Writes a string, applying the formatting options.

   s
      String to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(str_base const & s, io::text::writer * ptwOut) {
      _str_to_str_backend::write(
         s.chars_begin(),
         size_t(reinterpret_cast<int8_t const *>(s.chars_end()) -
            reinterpret_cast<int8_t const *>(s.chars_begin())),
         text::encoding::host, ptwOut
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::*str


namespace abc {

template <>
class to_str_backend<istr> : public to_str_backend<str_base> {};

template <>
class to_str_backend<mstr> : public to_str_backend<str_base> {};

template <>
class to_str_backend<dmstr> : public to_str_backend<str_base> {};

template <size_t t_cchStatic>
class to_str_backend<smstr<t_cchStatic>> : public to_str_backend<str_base> {};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


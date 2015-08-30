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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

/*! Base class for the specializations of to_str_backend for string types. Not using templates, so
the implementation can be in a cxx file. This is used by string literal types as well (see
below). */
class ABACLADE_SYM str_to_str_backend {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

protected:
   /*! Writes a string, applying the formatting options.

   @param p
      Pointer to the string to write.
   @param cb
      Size of the string pointed to by p, in bytes.
   @param enc
      Text encoding of the string pointed to by p.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(void const * p, std::size_t cb, encoding enc, io::text::writer * ptwOut);
};

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C, enc) \
   /*! Character literal. */ \
   template <> \
   class to_str_backend<C> : public text::detail::str_to_str_backend { \
   public: \
      /*! Writes a character, applying the formatting options.

      @param ch
         Character to write.
      @param ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C ch, io::text::writer * ptwOut) { \
         text::detail::str_to_str_backend::write(&ch, sizeof(C), enc, ptwOut); \
      } \
   }; \
   \
   /*! String literal. */ \
   template <std::size_t t_cch> \
   class to_str_backend<C [t_cch]> : public text::detail::str_to_str_backend { \
   public: \
      /*! Writes a string, applying the formatting options.

      @param ach
         String to write.
      @param ptwOut
         Pointer to the writer to output to.
      */ \
      void write(C const (& ach)[t_cch], io::text::writer * ptwOut) { \
         text::detail::str_to_str_backend::write(ach, sizeof(C) * ABC_SL_SIZE(ach), enc, ptwOut); \
      } \
   }; \
   \
   /*! MSC16 BUG: this partial specialization is necessary. */ \
   template <std::size_t t_cch> \
   class to_str_backend<C const [t_cch]> : public to_str_backend<C [t_cch]> {};
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char, text::encoding::utf8)
/* Specializations for wchar_t, if it’s what char16_t or char32_t map to, and for char16/32_t, if
they’re native types. */
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

namespace abc {

template <std::size_t t_cchEmbeddedCapacity>
class to_str_backend<text::sstr<t_cchEmbeddedCapacity>> : public text::detail::str_to_str_backend {
public:
   /*! Writes a string, applying the formatting options.

   @param s
      String to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::sstr<t_cchEmbeddedCapacity> const & s, io::text::writer * ptwOut) {
      text::detail::str_to_str_backend::write(s.chars_begin(), static_cast<std::size_t>(
         reinterpret_cast<std::uintptr_t>(s.chars_end()) -
         reinterpret_cast<std::uintptr_t>(s.chars_begin())
      ), text::encoding::host, ptwOut);
   }
};

template <>
class to_str_backend<text::str::const_codepoint_proxy> : public to_str_backend<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param cpp
      Code point to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::str::const_codepoint_proxy const & cpp, io::text::writer * ptwOut) {
      to_str_backend<char32_t>::write(cpp.operator char32_t(), ptwOut);
   }
};

template <>
class to_str_backend<text::str::codepoint_proxy> :
   public to_str_backend<text::str::const_codepoint_proxy> {
};

template <>
class to_str_backend<text::str::const_iterator> : public to_str_backend<std::size_t> {
public:
   /*! Writes a code point iterator as a character index, applying the formatting options.

   @param it
      Iterator to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(text::str::const_iterator const & it, io::text::writer * ptwOut) {
      to_str_backend<std::size_t>::write(reinterpret_cast<std::size_t>(it.base()), ptwOut);
   }
};

template <>
class to_str_backend<text::str::iterator> : public to_str_backend<text::str::const_iterator> {
};

} //namespace abc

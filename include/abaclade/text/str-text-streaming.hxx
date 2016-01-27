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

namespace abc { namespace text { namespace detail {

/*! Base class for the specializations of to_text_ostream for string types. Not using templates, so
the implementation can be in a cxx file. This is used by string literal types as well (see
below). */
class ABACLADE_SYM str_to_text_ostream {
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
   @param ptos
      Pointer to the stream to output to.
   */
   void write(void const * p, std::size_t cb, encoding enc, io::text::ostream * ptos);
};

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

#define ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(C, enc) \
   /*! Character literal. */ \
   template <> \
   class to_text_ostream<C> : public text::detail::str_to_text_ostream { \
   public: \
      /*! Writes a character, applying the formatting options.

      @param ch
         Character to write.
      @param ptos
         Pointer to the stream to output to.
      */ \
      void write(C ch, io::text::ostream * ptos) { \
         text::detail::str_to_text_ostream::write(&ch, sizeof(C), enc, ptos); \
      } \
   }; \
   \
   /*! String literal. */ \
   template <std::size_t t_cch> \
   class to_text_ostream<C [t_cch]> : public text::detail::str_to_text_ostream { \
   public: \
      /*! Writes a string, applying the formatting options.

      @param ach
         String to write.
      @param ptos
         Pointer to the stream to output to.
      */ \
      void write(C const (& ach)[t_cch], io::text::ostream * ptos) { \
         text::detail::str_to_text_ostream::write(ach, sizeof(C) * ABC_SL_SIZE(ach), enc, ptos); \
      } \
   }; \
   \
   /*! MSC16 BUG: this partial specialization is necessary. */ \
   template <std::size_t t_cch> \
   class to_text_ostream<C const [t_cch]> : public to_text_ostream<C [t_cch]> {};
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(char, text::encoding::utf8)
/* Specializations for wchar_t, if it’s what char16_t or char32_t map to, and for char16/32_t, if
they’re native types. */
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(char16_t, text::encoding::utf16_host)
#elif ABC_CXX_CHAR16 == 1
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::encoding::utf16_host)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(char32_t, text::encoding::utf32_host)
#elif ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::encoding::utf32_host)
#endif
#undef ABC_SPECIALIZE_to_text_ostream_FOR_TYPE

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_text_ostream<text::str> : public text::detail::str_to_text_ostream {
public:
   /*! Writes a string, applying the formatting options.

   @param s
      String to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::str const & s, io::text::ostream * ptos);
};

template <std::size_t t_cchEmbeddedCapacity>
class to_text_ostream<text::sstr<t_cchEmbeddedCapacity>> : public to_text_ostream<text::str> {
public:
   /*! Writes a string, applying the formatting options.

   @param s
      String to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::sstr<t_cchEmbeddedCapacity> const & s, io::text::ostream * ptos) {
      to_text_ostream<text::str>::write(s.str(), ptos);
   }
};

template <>
class to_text_ostream<text::str::const_codepoint_proxy> : public to_text_ostream<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param cpp
      Code point to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::str::const_codepoint_proxy const & cpp, io::text::ostream * ptos) {
      to_text_ostream<char32_t>::write(cpp.operator char32_t(), ptos);
   }
};

template <>
class to_text_ostream<text::str::codepoint_proxy> :
   public to_text_ostream<text::str::const_codepoint_proxy> {
};

template <>
class to_text_ostream<text::str::const_iterator> : public to_text_ostream<std::size_t> {
public:
   /*! Writes a code point iterator as a character index, applying the formatting options.

   @param it
      Iterator to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::str::const_iterator const & it, io::text::ostream * ptos) {
      to_text_ostream<std::size_t>::write(it.char_index(), ptos);
   }
};

template <>
class to_text_ostream<text::str::iterator> : public to_text_ostream<text::str::const_iterator> {
};

} //namespace abc
//! @endcond

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_STR_1_HXX

#include <lofty/text/str-0.hxx>

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_STR_1_HXX
#endif

#ifndef _LOFTY_TEXT_STR_1_HXX_NOPUB
#define _LOFTY_TEXT_STR_1_HXX_NOPUB

#include <lofty/from_text_istream.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM from_text_istream<text::_LOFTY_PUBNS str> {
public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, text::_LOFTY_PUBNS str * dst
   );

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(
      text::parsers::_LOFTY_PUBNS regex_capture_format const & format,
      text::parsers::_LOFTY_PUBNS dynamic * parser
   );
};

template <std::size_t dst_embedded_capacity>
class from_text_istream<text::_LOFTY_PUBNS sstr<dst_embedded_capacity>> :
   public from_text_istream<text::_LOFTY_PUBNS str> {
public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0,
      text::_LOFTY_PUBNS sstr<dst_embedded_capacity> * dst
   ) {
      from_text_istream<text::_pub::str>::convert_capture(capture0, dst->str_ptr());
   }
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace _pvt {

/*! Base class for the specializations of to_text_ostream for string types. Not using templates, so the
implementation can be in a cxx file. This is used by string literal types as well (see below). */
class LOFTY_SYM str_to_text_ostream {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(_LOFTY_PUBNS str const & format);

protected:
   /*! Writes a string, applying the formatting options.

   @param src
      Pointer to the string to write.
   @param src_byte_size
      Size of the string pointed to by p, in bytes.
   @param enc
      Text encoding of the string pointed to by p.
   @param dst
      Pointer to the stream to output to.
   */
   void write(
      void const * src, std::size_t src_byte_size, _LOFTY_PUBNS encoding enc,
      io::text::_LOFTY_PUBNS ostream * dst
   );
};

}}} //namespace lofty::text::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//!@cond
#define LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(C, enc) \
   /*! Character literal. */ \
   template <> \
   class to_text_ostream<C> : public text::_pvt::str_to_text_ostream { \
   public: \
      /*! Writes a character, applying the formatting options.

      @param src
         Object to write.
      @param dst
         Pointer to the stream to output to.
      */ \
      void write(C src, io::text::_LOFTY_PUBNS ostream * dst) { \
         text::_pvt::str_to_text_ostream::write(&src, sizeof(C), enc, dst); \
      } \
   }; \
   \
   /*! String literal. */ \
   template <std::size_t src_size> \
   class to_text_ostream<C[src_size]> : public text::_pvt::str_to_text_ostream { \
   public: \
      /*! Writes a string, applying the formatting options.

      @param src
         Object to write.
      @param dst
         Pointer to the stream to output to.
      */ \
      void write(C const (& src)[src_size], io::text::_LOFTY_PUBNS ostream * dst) { \
         text::_pvt::str_to_text_ostream::write(src, sizeof(C) * LOFTY_SL_SIZE(src), enc, dst); \
      } \
   }; \
   \
   /*! MSC16 BUG: this partial specialization is necessary. */ \
   template <std::size_t src_size> \
   class to_text_ostream<C const[src_size]> : public to_text_ostream<C[src_size]> {};
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char, text::_LOFTY_PUBNS encoding::utf8)
/* Specializations for wchar_t, if it’s what char16_t or char32_t map to, and for char16/32_t, if they’re
native types. */
#if LOFTY_CXX_CHAR16 == 2
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char16_t, text::_LOFTY_PUBNS encoding::utf16_host)
#elif LOFTY_CXX_CHAR16 == 1
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::_LOFTY_PUBNS encoding::utf16_host)
#endif
#if LOFTY_CXX_CHAR32 == 2
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char32_t, text::_LOFTY_PUBNS encoding::utf32_host)
#elif LOFTY_CXX_CHAR32 == 1
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::_LOFTY_PUBNS encoding::utf32_host)
#endif
#undef LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE
//!@endcond

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<text::_LOFTY_PUBNS str> : public text::_pvt::str_to_text_ostream {
public:
   /*! Writes a string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::_LOFTY_PUBNS str const & src, io::text::_LOFTY_PUBNS ostream * dst);
};

template <std::size_t src_embedded_capacity>
class to_text_ostream<text::_LOFTY_PUBNS sstr<src_embedded_capacity>> :
   public to_text_ostream<text::_LOFTY_PUBNS str> {
public:
   /*! Writes a string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(
      text::_LOFTY_PUBNS sstr<src_embedded_capacity> const & src, io::text::_LOFTY_PUBNS ostream * dst
   ) {
      to_text_ostream<text::_pub::str>::write(src.str(), dst);
   }
};

template <>
class to_text_ostream<text::_LOFTY_PUBNS str::const_codepoint_proxy> : public to_text_ostream<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(
      text::_LOFTY_PUBNS str::const_codepoint_proxy const & src, io::text::_LOFTY_PUBNS ostream * dst
   ) {
      to_text_ostream<char32_t>::write(src.operator char32_t(), dst);
   }
};

template <>
class to_text_ostream<text::_LOFTY_PUBNS str::codepoint_proxy> :
   public to_text_ostream<text::_LOFTY_PUBNS str::const_codepoint_proxy> {
};

template <>
class to_text_ostream<text::_LOFTY_PUBNS str::const_iterator> : public to_text_ostream<std::size_t> {
public:
   /*! Writes a code point iterator as a character index, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::_LOFTY_PUBNS str::const_iterator const & src, io::text::_LOFTY_PUBNS ostream * dst) {
      to_text_ostream<std::size_t>::write(src.char_index(), dst);
   }
};

template <>
class to_text_ostream<text::_LOFTY_PUBNS str::iterator> :
   public to_text_ostream<text::_LOFTY_PUBNS str::const_iterator> {
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_STR_1_HXX_NOPUB

#ifdef _LOFTY_TEXT_STR_1_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_STR_1_HXX

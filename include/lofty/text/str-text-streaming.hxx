/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM from_text_istream<text::str> {
public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(text::parsers::dynamic_match_capture const & capture0, text::str * dst);

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::dynamic_state const * format_to_parser_states(
      text::str const & format, text::parsers::dynamic * parser
   );
};

template <std::size_t dst_embedded_capacity>
class from_text_istream<text::sstr<dst_embedded_capacity>> : public from_text_istream<text::str> {
public:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(
      text::parsers::dynamic_match_capture const & capture0, text::sstr<dst_embedded_capacity> * dst
   ) {
      from_text_istream<text::str>::convert_capture(capture0, dst.str_ptr());
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
   void set_format(str const & format);

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
   void write(void const * src, std::size_t src_byte_size, encoding enc, io::text::ostream * dst);
};

}}} //namespace lofty::text::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

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
      void write(C src, io::text::ostream * dst) { \
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
      void write(C const (& src)[src_size], io::text::ostream * dst) { \
         text::_pvt::str_to_text_ostream::write(src, sizeof(C) * LOFTY_SL_SIZE(src), enc, dst); \
      } \
   }; \
   \
   /*! MSC16 BUG: this partial specialization is necessary. */ \
   template <std::size_t src_size> \
   class to_text_ostream<C const[src_size]> : public to_text_ostream<C[src_size]> {};
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char, text::encoding::utf8)
/* Specializations for wchar_t, if it’s what char16_t or char32_t map to, and for char16/32_t, if they’re
native types. */
#if LOFTY_CXX_CHAR16 == 2
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char16_t, text::encoding::utf16_host)
#elif LOFTY_CXX_CHAR16 == 1
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::encoding::utf16_host)
#endif
#if LOFTY_CXX_CHAR32 == 2
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(char32_t, text::encoding::utf32_host)
#elif LOFTY_CXX_CHAR32 == 1
LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE(wchar_t, text::encoding::utf32_host)
#endif
#undef LOFTY_SPECIALIZE_to_text_ostream_FOR_TYPE

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<text::str> : public text::_pvt::str_to_text_ostream {
public:
   /*! Writes a string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::str const & src, io::text::ostream * dst);
};

template <std::size_t src_embedded_capacity>
class to_text_ostream<text::sstr<src_embedded_capacity>> : public to_text_ostream<text::str> {
public:
   /*! Writes a string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::sstr<src_embedded_capacity> const & src, io::text::ostream * dst) {
      to_text_ostream<text::str>::write(src.str(), dst);
   }
};

template <>
class to_text_ostream<text::str::const_codepoint_proxy> : public to_text_ostream<char32_t> {
public:
   /*! Writes a code point proxy as a plain code point (char32_t), applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::str::const_codepoint_proxy const & src, io::text::ostream * dst) {
      to_text_ostream<char32_t>::write(src.operator char32_t(), dst);
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

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::str::const_iterator const & src, io::text::ostream * dst) {
      to_text_ostream<std::size_t>::write(src.char_index(), dst);
   }
};

template <>
class to_text_ostream<text::str::iterator> : public to_text_ostream<text::str::const_iterator> {
};

} //namespace lofty
//! @endcond

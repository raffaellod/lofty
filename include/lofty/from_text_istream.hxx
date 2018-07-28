/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_FROM_TEXT_ISTREAM_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_FROM_TEXT_ISTREAM_HXX
#endif

#ifndef _LOFTY_FROM_TEXT_ISTREAM_HXX_NOPUB
#define _LOFTY_FROM_TEXT_ISTREAM_HXX_NOPUB

#include <lofty/_std/memory.hxx>
#include <lofty/_std/type_traits.hxx>
#include <lofty/text/str-0.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.
namespace lofty { namespace text { namespace parsers {
_LOFTY_PUBNS_BEGIN

class regex_capture_format;
class dynamic;
struct dynamic_state;
class dynamic_match_capture;

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Throws a lofty::text::syntax_error if the first argument, referencing the end of a format string parsed by
a lofty::from_text_istream or lofty::to_text_ostream specialization, does not equal the end of the format
string.

Declared in both lofty/from_text_istream.hxx and lofty/to_text_ostream.hxx .

@param format_consumed_end
   Iterator to the end of the consumed/parsed portion of the format string.
@param format
   Format string.
*/
LOFTY_SYM void throw_on_unused_streaming_format_chars(
   text::_LOFTY_PUBNS str::const_iterator const & format_consumed_end, text::_LOFTY_PUBNS str const & format
);

_LOFTY_PUBNS_END
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Reads and parses a string representation of an object of type T, according to an optional format string.
Once constructed with the desired format specification, an instance must be able to convert into T instances
any number of strings.

This class template and its specializations are at the core of lofty::from_str() and 
lofty::io::text::istream::scan(). */
template <typename T>
class from_text_istream;

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Specialization for bool.
template <>
class LOFTY_SYM from_text_istream<bool> {
public:
   //! Default constructor.
   from_text_istream();

   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, bool * dst);

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

protected:
   //! String that will be translated to true.
   text::_LOFTY_PUBNS str true_str;
   //! String that will be translated to false.
   text::_LOFTY_PUBNS str false_str;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Base class for the specializations of from_text_istream for integer types.
class LOFTY_SYM int_from_text_istream_base {
public:
   /*! Constructor.

   @param is_signed
      true if the integer type is signed, or false otherwise.
   */
   explicit int_from_text_istream_base(bool is_signed);

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

protected:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the top-level capture.
   @param dst
      Pointer to the destination object.
   */
   template <typename I>
   void convert_capture_impl(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, I * dst
   ) const;

   //! Converts a capture into a 64-bit signed integer. See convert_capture_impl().
   void convert_capture_s64(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int64_t * dst
   ) const;

   //! Converts a capture into a 64-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u64(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint64_t * dst
   ) const;

   //! Converts a capture into a 32-bit signed integer. See convert_capture_impl().
   void convert_capture_s32(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int32_t * dst
   ) const;

   //! Converts a capture into a 32-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u32(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint32_t * dst
   ) const;

   //! Converts a capture into a 16-bit signed integer. See convert_capture_impl().
   void convert_capture_s16(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int16_t * dst
   ) const;

   //! Converts a capture into a 16-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u16(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint16_t * dst
   ) const;

   //! Converts a capture into an 8-bit signed integer. See convert_capture_impl().
   void convert_capture_s8(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int8_t * dst
   ) const {
      std::int16_t dst16;
      convert_capture_s16(capture0, &dst16);
      *dst = static_cast<std::int8_t>(dst16);
   }

   //! Converts an 8-bit unsigned integer to its string representation. See convert_capture_impl().
   void convert_capture_u8(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint8_t * dst
   ) const {
      std::uint16_t dst16;
      convert_capture_u16(capture0, &dst16);
      *dst = static_cast<std::uint8_t>(dst16);
   }

private:
   /*! Creates parser states for parsing numbers in base 2.

   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state * create_base2_parser_states(
      text::parsers::_LOFTY_PUBNS dynamic * parser
   );

   /*! Creates parser states for parsing numbers in base 8.

   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state * create_base8_parser_states(
      text::parsers::_LOFTY_PUBNS dynamic * parser
   );

   /*! Creates parser states for parsing numbers in base 10.

   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state * create_base10_parser_states(
      text::parsers::_LOFTY_PUBNS dynamic * parser
   );

   /*! Creates parser states for parsing numbers in base 16.

   @param parser
      Pointer to the parser instance to use to create non-static states.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state * create_base16_parser_states(
      text::parsers::_LOFTY_PUBNS dynamic * parser
   );

protected:
   //! true if the integer type is signed, or false otherwise.
   bool is_signed;
   //! true if multiple bases are supported via prefix on parsing.
   bool prefix;
   //! Base to be used if prefix == false. 10 for base 10, or log2(base) for other bases.
   unsigned unprefixed_base_or_shift;
};

#if LOFTY_HOST_WORD_SIZE >= 32
#if LOFTY_HOST_WORD_SIZE >= 64

// On a machine with 64-bit word size, convert_capture_64*() will be faster.

inline void int_from_text_istream_base::convert_capture_s32(
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int32_t * dst
) const {
   std::int64_t dst64;
   convert_capture_s64(capture0, &dst64);
   *dst = static_cast<std::int32_t>(dst64);
}

inline void int_from_text_istream_base::convert_capture_u32(
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint32_t * dst
) const {
   std::uint64_t dst64;
   convert_capture_u64(capture0, &dst64);
   *dst = static_cast<std::uint32_t>(dst64);
}

#endif //if LOFTY_HOST_WORD_SIZE >= 64

/* On a machine with 32-bit word size, convert_capture_32*() will be faster. Note that the latter might in
turn defer to convert_capture_64*() (see above). */

inline void int_from_text_istream_base::convert_capture_s16(
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::int16_t * dst
) const {
   std::int32_t dst32;
   convert_capture_s32(capture0, &dst32);
   *dst = static_cast<std::int16_t>(dst32);
}

inline void int_from_text_istream_base::convert_capture_u16(
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::uint16_t * dst
) const {
   std::uint32_t dst32;
   convert_capture_u32(capture0, &dst32);
   *dst = static_cast<std::uint16_t>(dst32);
}

#endif //if LOFTY_HOST_WORD_SIZE >= 32

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Implementation of the specializations of from_text_istream for integer types.
template <typename I>
class int_from_text_istream : public int_from_text_istream_base {
public:
   //! Default constructor.
   int_from_text_istream() :
      int_from_text_istream_base(_std::_pub::is_signed<I>::value) {
   }

   /*! Converts a capture into a value of the appropriate type.

   This design is rather tricky in the way one implementation calls another:

   1. int_from_text_istream<I>::convert_capture()
      Always inlined, dispatches to step 2. based on number of bits;
   2. int_from_text_istream_base::convert_capture_{s,u}{8,16,32,64}()
      Inlined to a bit-bigger variant or implemented in from_text_istream.cxx, depending on the host
      architecture’s word size;
   3. int_from_text_istream_base::convert_capture_impl()
      Always inlined, but only used in functions defined in from_text_istream.cxx, so it only generates as
      many copies as strictly necessary to have fastest performance for any integer size.

   The net result is that after all the inlining occurs, this will become a direct call to the fastest
   implementation for I of any given size.

   @param capture0
      Pointer to the captured string.
   @param dst
      Pointer to the destination object.
   */
   void convert_capture(text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, I * dst) {
      switch (sizeof *dst) {
         case sizeof(std::int8_t):
            if (_std::_pub::is_signed<I>::value) {
               convert_capture_s8(capture0, reinterpret_cast<std::int8_t *>(dst));
            } else {
               convert_capture_u8(capture0, reinterpret_cast<std::uint8_t *>(dst));
            }
            break;
         case sizeof(std::int16_t):
            if (_std::_pub::is_signed<I>::value) {
               convert_capture_s16(capture0, reinterpret_cast<std::int16_t *>(dst));
            } else {
               convert_capture_u16(capture0, reinterpret_cast<std::uint16_t *>(dst));
            }
            break;
         case sizeof(std::int32_t):
            if (_std::_pub::is_signed<I>::value) {
               convert_capture_s32(capture0, reinterpret_cast<std::int32_t *>(dst));
            } else {
               convert_capture_u32(capture0, reinterpret_cast<std::uint32_t *>(dst));
            }
            break;
         case sizeof(std::int64_t):
            if (_std::_pub::is_signed<I>::value) {
               convert_capture_s64(capture0, reinterpret_cast<std::int64_t *>(dst));
            } else {
               convert_capture_u64(capture0, reinterpret_cast<std::uint64_t *>(dst));
            }
            break;
         default:
            static_assert(sizeof *dst <= sizeof(std::int64_t), "unsupported integer size");
      }
   }
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

#define LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(I) \
   template <> \
   class from_text_istream<I> : public _pvt::int_from_text_istream<I> {};
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(  signed char)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(unsigned char)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(         short)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(unsigned short)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(     int)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(unsigned)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(         long)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(unsigned long)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(         long long)
LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE(unsigned long long)
#undef LOFTY_SPECIALIZE_from_text_istream_FOR_TYPE

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*! Base class for the specializations of from_text_istream for sequence types. Not using templates, so the
implementation can be in a .cxx file.

In order to ensure constant indices for the repetitions and captures generated, this class will wrap the
expressions for start, end and separators into {1} repetitions; in the end the resulting parser states will
look like the expression “(?:start_delim){1}(?:(elt)(?:(?:separator){1}(elt))*)?(?:end_delim){1}”. */
class LOFTY_SYM sequence_from_text_istream {
private:
   //! Stores members of types not defined at this point.
   struct impl;

public:
   /*! Constructor.

   @param start_delim
      Sequence start delimiter.
   @param end_delim
      Sequence end delimiter.
   */
   sequence_from_text_istream(
      text::_LOFTY_PUBNS str const & start_delim, text::_LOFTY_PUBNS str const & end_delim
   );

   //! Destructor.
   ~sequence_from_text_istream();

   /*! Returns the captured element at index i.

   @param capture0
      Top-level capture.
   @param i
      Index of the element to return.
   @return
      Capture containing the element.
   */
   text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture_at(
      text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0, std::size_t i
   );

   /*! Returns the count of captured elements.

   @param capture0
      Top-level capture.
   @return
      Count of captured elements.
   */
   std::size_t captures_count(text::parsers::_LOFTY_PUBNS dynamic_match_capture const & capture0) const;

   /*! Extracts the format in which elements should be matched.

   @param format
      Format for the collection as a whole.
   @return
      Format for individual elements.
   */
   text::parsers::_LOFTY_PUBNS regex_capture_format const & extract_elt_format(
      text::parsers::_LOFTY_PUBNS regex_capture_format const & format
   );

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   @param parser
      Pointer to the parser instance to use to create non-static states.
   @param elt_first_state
      Parser states that match a single element in the collection.
   @return
      First parser state.
   */
   text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(
      text::parsers::_LOFTY_PUBNS regex_capture_format const & format,
      text::parsers::_LOFTY_PUBNS dynamic * parser,
      text::parsers::_LOFTY_PUBNS dynamic_state const * elt_first_state
   );

protected:
   //! Separator to be output between elements.
   text::_LOFTY_PUBNS str separator;
   //! Sequence start delimiter.
   text::_LOFTY_PUBNS str start_delim;
   //! Sequence end delimiter.
   text::_LOFTY_PUBNS str end_delim;

private:
   //! Pointer to members of complex types that would require additional files to be #included.
   _std::_LOFTY_PUBNS unique_ptr<impl> pimpl;
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_FROM_TEXT_ISTREAM_HXX_NOPUB

#ifdef _LOFTY_FROM_TEXT_ISTREAM_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   #ifndef _LOFTY_TO_TEXT_OSTREAM_HXX
   using _pub::throw_on_unused_streaming_format_chars;
   #endif

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_FROM_TEXT_ISTREAM_HXX

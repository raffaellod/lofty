/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

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

namespace lofty {

/*! Throws a lofty::text::syntax_error if the first argument, referencing the end of a format string parsed by
a lofty::from_text_istream or lofty::from_text_istream specialization, does not equal the end of the format
string.

@param format_consumed_end
   Iterator to the end of the consumed/parsed portion of the format string.
@param format
   Format string.
*/
LOFTY_SYM void throw_on_unused_streaming_format_chars(
   str::const_iterator const & format_consumed_end, str const & format
);

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Reads and parses a string representation of an object of type T, according to an optional format string.
Once constructed with the desired format specification, an instance must be able to convert into T instances
any number of strings.

This class template and its specializations are at the core of lofty::from_str() and 
lofty::io::text::istream::scan(). */
template <typename T>
class from_text_istream;

} //namespace lofty

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
      Pointer to the captured string.
   @param dst
      Object to return into.
   */
   void convert_capture(text::parsers::dynamic_match_capture const & capture0, bool * dst);

   /*! Creates parser states for the specified input format.

   @param format
      Formatting options.
   */
   text::parsers::dynamic_state const * format_to_parser_states(
      str const & format, text::parsers::dynamic * parser
   );

protected:
   //! String that will be translated to true.
   str true_str;
   //! String that will be translated to false.
   str false_str;
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
   */
   text::parsers::dynamic_state const * format_to_parser_states(
      str const & format, text::parsers::dynamic * parser
   );

protected:
   /*! Converts a capture into a value of the appropriate type.

   @param capture0
      Pointer to the captured string.
   @param dst
      Object to return into.
   */
   template <typename I>
   void convert_capture_impl(text::parsers::dynamic_match_capture const & capture0, I * dst) const;

   //! Converts a capture into a 64-bit signed integer. See convert_capture_impl().
   void convert_capture_s64(text::parsers::dynamic_match_capture const & capture0, std::int64_t * dst) const;

   //! Converts a capture into a 64-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u64(text::parsers::dynamic_match_capture const & capture0, std::uint64_t * dst) const;

   //! Converts a capture into a 32-bit signed integer. See convert_capture_impl().
   void convert_capture_s32(text::parsers::dynamic_match_capture const & capture0, std::int32_t * dst) const;

   //! Converts a capture into a 32-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u32(text::parsers::dynamic_match_capture const & capture0, std::uint32_t * dst) const;

   //! Converts a capture into a 16-bit signed integer. See convert_capture_impl().
   void convert_capture_s16(text::parsers::dynamic_match_capture const & capture0, std::int16_t * dst) const;

   //! Converts a capture into a 16-bit unsigned integer. See convert_capture_impl().
   void convert_capture_u16(text::parsers::dynamic_match_capture const & capture0, std::uint16_t * dst) const;

   //! Converts a capture into an 8-bit signed integer. See convert_capture_impl().
   void convert_capture_s8(text::parsers::dynamic_match_capture const & capture0, std::int8_t * dst) const {
      std::int16_t dst16;
      convert_capture_s16(capture0, &dst16);
      *dst = static_cast<std::int8_t>(dst16);
   }

   //! Converts an 8-bit unsigned integer to its string representation. See convert_capture_impl().
   void convert_capture_u8(text::parsers::dynamic_match_capture const & capture0, std::uint8_t * dst) const {
      std::uint16_t dst16;
      convert_capture_u16(capture0, &dst16);
      *dst = static_cast<std::uint8_t>(dst16);
   }

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
   text::parsers::dynamic_match_capture const & capture0, std::int32_t * dst
) const {
   std::int64_t dst64;
   convert_capture_s64(capture0, &dst64);
   *dst = static_cast<std::int32_t>(dst64);
}

inline void int_from_text_istream_base::convert_capture_u32(
   text::parsers::dynamic_match_capture const & capture0, std::uint32_t * dst
) const {
   std::uint64_t dst64;
   convert_capture_u64(capture0, &dst64);
   *dst = static_cast<std::uint32_t>(dst64);
}

#endif //if LOFTY_HOST_WORD_SIZE >= 64

/* On a machine with 32-bit word size, convert_capture_32*() will be faster. Note that the latter might in
turn defer to convert_capture_64*() (see above). */

inline void int_from_text_istream_base::convert_capture_s16(
   text::parsers::dynamic_match_capture const & capture0, std::int16_t * dst
) const {
   std::int32_t dst32;
   convert_capture_s32(capture0, &dst32);
   *dst = static_cast<std::int16_t>(dst32);
}

inline void int_from_text_istream_base::convert_capture_u16(
   text::parsers::dynamic_match_capture const & capture0, std::uint16_t * dst
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
      int_from_text_istream_base(_std::is_signed<I>::value) {
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
      Object to return into.
   */
   void convert_capture(text::parsers::dynamic_match_capture const & capture0, I * dst) {
      switch (sizeof *dst) {
         case sizeof(std::int8_t):
            if (_std::is_signed<I>::value) {
               convert_capture_s8(capture0, reinterpret_cast<std::int8_t *>(dst));
            } else {
               convert_capture_u8(capture0, reinterpret_cast<std::uint8_t *>(dst));
            }
            break;
         case sizeof(std::int16_t):
            if (_std::is_signed<I>::value) {
               convert_capture_s16(capture0, reinterpret_cast<std::int16_t *>(dst));
            } else {
               convert_capture_u16(capture0, reinterpret_cast<std::uint16_t *>(dst));
            }
            break;
         case sizeof(std::int32_t):
            if (_std::is_signed<I>::value) {
               convert_capture_s32(capture0, reinterpret_cast<std::int32_t *>(dst));
            } else {
               convert_capture_u32(capture0, reinterpret_cast<std::uint32_t *>(dst));
            }
            break;
         case sizeof(std::int64_t):
            if (_std::is_signed<I>::value) {
               convert_capture_s64(capture0, reinterpret_cast<std::int64_t *>(dst));
            } else {
               convert_capture_u64(capture0, reinterpret_cast<std::uint64_t *>(dst));
            }
            break;
         default:
            static_assert(sizeof *dst > sizeof(std::int64_t), "unsupported integer size");
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

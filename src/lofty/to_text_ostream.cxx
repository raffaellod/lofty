/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/io/text.hxx>
#include <lofty/math.hxx>
#include <lofty/memory.hxx>
#include <lofty/numeric.hxx>
#include <lofty/_std/algorithm.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/typeinfo.hxx>
#include <lofty/text.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>
#include <lofty/text/str.hxx>
#include <lofty/to_text_ostream.hxx>
#include <climits> // CHAR_BIT
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   #include <cxxabi.h> // abi::__cxa_demangle()
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

void throw_on_unused_streaming_format_chars(
   text::str::const_iterator const & format_consumed_end, text::str const & format
) {
   if (format_consumed_end != format.cend()) {
      LOFTY_THROW(text::syntax_error, (
         LOFTY_SL("unexpected character in format string"), format,
         static_cast<unsigned>(format_consumed_end - format.cbegin())
      ));
   }
}

_LOFTY_PUBNS_END
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<bool>::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<bool>::write(bool src, io::text::ostream * dst) {
   if (src) {
      dst->write(LOFTY_SL("true"));
   } else {
      dst->write(LOFTY_SL("false"));
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

char const int_to_text_ostream_base::int_to_upper_str_map[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
char const int_to_text_ostream_base::int_to_lower_str_map[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

int_to_text_ostream_base::int_to_text_ostream_base(unsigned bytes_per_int_) :
   int_to_str_map(int_to_lower_str_map),
   // Default to generating at least a single zero.
   width(1),
   buf_size(1 /*possible sign*/ + 3 /*max base10 characters per byte*/ * bytes_per_int_),
   bytes_per_int(static_cast<std::uint8_t>(bytes_per_int_)),
   // Default to decimal notation.
   base_or_shift(10),
   // Default padding is with spaces (and won’t be applied by default).
   padding_char(' '),
   // A sign will only be displayed if the number is negative and no prefix is applied.
   positive_sign_char('\0'),
   prefix_char_0('\0'),
   prefix_char_1('\0') {
}

void int_to_text_ostream_base::set_format(text::str const & format) {
   bool prefix = false;
   auto itr(format.cbegin());
   char32_t ch;
   if (itr == format.cend()) {
      goto default_notation;
   }
   ch = *itr++;
   // Display a plus or a space in front of non-negative numbers.
   if (ch == '+' || ch == ' ') {
      // Force this character to be displayed for non-negative numbers.
      positive_sign_char = static_cast<char>(ch);
      if (itr == format.cend()) {
         goto default_notation;
      }
      ch = *itr++;
   }
   // Prefix with 0b, 0B, 0, 0x or 0X.
   if (ch == '#') {
      prefix = true;
      if (itr == format.cend()) {
         goto default_notation;
      }
      ch = *itr++;
   }
   // Pad with zeros instead of spaces.
   if (ch == '0') {
      padding_char = '0';
      if (itr == format.cend()) {
         goto default_notation;
      }
      ch = *itr++;
   }
   // “Width” - minimum number of digits.
   if (ch >= '1' && ch <= '9') {
      /* Undo the default; the following loop will yield at least 1 anyway (because we don’t get here for a 0
      – see if above). */
      width = 0;
      do {
         width = width * 10 + static_cast<unsigned>(ch - '0');
         if (itr == format.cend()) {
            goto default_notation;
         }
         ch = *itr++;
      } while (ch >= '0' && ch <= '9');
   }

   /* We jump in this impossible if to set the default notation when we run out of characters in any of the
   above blocks. If we do get here without jumping, the last character retrieved and stored in is the
   requested notation. */
   if (false) {
default_notation:
      ch = 'd';
   }

   // Determine which notation to use, which will also yield the approximate number of characters per byte.
   unsigned chars_per_byte;
   switch (ch) {
      case 'b':
      case 'B':
      case 'o':
      case 'x':
      case 'X':
         if (prefix) {
            prefix_char_0 = '0';
         }
         // Fall through.
      case 'd':
         switch (ch) {
            case 'b': // Binary notation, lowercase prefix.
            case 'B': // Binary notation, uppercase prefix.
               prefix_char_1 = static_cast<char>(ch);
               base_or_shift = 1;
               chars_per_byte = 8;
               break;
            case 'o': // Octal notation.
               base_or_shift = 3;
               chars_per_byte = 3;
               break;
            case 'X': // Hexadecimal notation, uppercase prefix and letters.
               int_to_str_map = int_to_upper_str_map;
               // Fall through.
            case 'x': // Hexadecimal notation, lowercase prefix and letters.
               prefix_char_1 = static_cast<char>(ch);
               base_or_shift = 4;
               chars_per_byte = 2;
               break;
            case 'd': // Decimal notation.
               base_or_shift = 10;
               chars_per_byte = 3;
               break;
         }
         if (itr == format.cend()) {
            break;
         }
         // If we still have any characters, they are garbage (fall through).
      default:
         LOFTY_THROW(text::syntax_error, (
            LOFTY_SL("unexpected character"), format, static_cast<unsigned>(itr - format.cbegin())
         ));
   }

   // Now we know enough to calculate the required buffer size.
   buf_size = 2 /*prefix or sign*/ + _std::max(width, chars_per_byte * bytes_per_int);
}

void int_to_text_ostream_base::add_prefixes_and_write(
   bool negative, io::text::ostream * dst, text::str * buf, text::str::iterator buf_first_used_itr
) const {
   auto buf_end(buf->cend());
   auto itr(buf_first_used_itr);
   // Ensure that at least one digit is generated.
   if (itr == buf_end) {
      *--itr = '0';
   }
   /* Determine the sign character: only if in decimal notation, and make it a minus sign if the number is
   negative. */
   char sign_char = base_or_shift == 10 ? negative ? '-' : positive_sign_char : '\0';
   // Decide whether we’ll put a sign last, after the padding.
   bool sign_at_end = sign_char && padding_char == '0';
   // Add the sign character if there’s no prefix and the padding is not zeros.
   if (sign_char && padding_char != '0') {
      *--itr = sign_char;
   }
   // Ensure that at least width characters are generated (but reserve a space for the sign).
   auto first_digit_itr(buf_end - static_cast<std::ptrdiff_t>(width - (sign_at_end ? 1 : 0)));
   while (itr > first_digit_itr) {
      *--itr = padding_char;
   }
   // Add prefix or sign (if padding with zeros), if any.
   if (prefix_char_0) {
      if (prefix_char_1) {
         *--itr = prefix_char_1;
      }
      *--itr = prefix_char_0;
   } else if (sign_at_end) {
      // Add the sign character.
      *--itr = sign_char;
   }
   // Write the constructed string.
   dst->write_binary(
      itr.ptr(), sizeof(text::char_t) * static_cast<std::size_t>(buf_end - itr), text::encoding::host
   );
}

template <typename I>
inline void int_to_text_ostream_base::write_impl(I i, io::text::ostream * dst) const {
   // Create a buffer of sufficient size for binary notation (the largest).
   text::sstr<2 /*prefix or sign*/ + sizeof(I) * CHAR_BIT> buf;
   /* Use clear = true since we need to iterate backwards on buf, which requires reading its otherwise
   uninitialized charactes. */
   buf.set_size_in_chars(buf_size, true);
   auto itr(buf.end());

   // Generate the digits.
   I rest(i);
   if (base_or_shift == 10) {
      // Base 10: must use % and /.
      I divider = static_cast<I>(base_or_shift);
      while (rest) {
         I mod(rest % divider);
         rest /= divider;
         *--itr = int_to_str_map[math::abs(mod)];
      }
   } else {
      // Base 2 ^ n: can use & and >>.
      I mask = (I(1) << base_or_shift) - 1;
      while (rest) {
         *--itr = int_to_str_map[rest & mask];
         rest >>= base_or_shift;
      }
   }

   // Add prefix or sign, and output to the stream.
   add_prefixes_and_write(numeric::is_negative<I>(i), dst, buf.str_ptr(), itr);
}

void int_to_text_ostream_base::write_s64(std::int64_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}

void int_to_text_ostream_base::write_u64(std::uint64_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}

#if LOFTY_HOST_WORD_SIZE < 64
void int_to_text_ostream_base::write_s32(std::int32_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}

void int_to_text_ostream_base::write_u32(std::uint32_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}

#if LOFTY_HOST_WORD_SIZE < 32
void int_to_text_ostream_base::write_s16(std::int16_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}

void int_to_text_ostream_base::write_u16(std::uint16_t i, io::text::ostream * dst) const {
   write_impl(i, dst);
}
#endif //if LOFTY_HOST_WORD_SIZE < 32
#endif //if LOFTY_HOST_WORD_SIZE < 64

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

ptr_to_text_ostream::ptr_to_text_ostream() {
   to_text_ostream<std::uintptr_t>::set_format(LOFTY_SL("#x"));
}

void ptr_to_text_ostream::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void ptr_to_text_ostream::_write_impl(std::uintptr_t src, io::text::ostream * dst) {
   if (src) {
      to_text_ostream<std::uintptr_t>::write(src, dst);
   } else {
      dst->write(LOFTY_SL("nullptr"));
   }
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

to_text_ostream<_std::type_info>::to_text_ostream() {
}

to_text_ostream<_std::type_info>::~to_text_ostream() {
}

void to_text_ostream<_std::type_info>::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<_std::type_info>::write(_std::type_info const & src, io::text::ostream * dst) {
   char const * name = src.name();
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   // Clang and G++ generate mangled names.
   int ret = 0;
   _std::unique_ptr<char const, memory::freeing_deleter> demangled_name(
      abi::__cxa_demangle(name, nullptr, nullptr, &ret)
   );
   if (ret >= 0 && demangled_name) {
      name = demangled_name.get();
   } else {
      name = "?";
   }
#elif LOFTY_HOST_CXX_MSC
   /* MSC prepends “class ”, “struct ” or “union ” to the type name; find the first space and strip anything
   preceding it. */
   for (auto ch = name; *ch; ++ch) {
      if (*ch == ' ') {
         name = ch + 1;
         break;
      }
   }
#endif
   to_text_ostream<text::char_ptr_to_str_adapter> char_ptr_adapter;
   char_ptr_adapter.write(text::char_ptr_to_str_adapter(name), dst);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

sequence_to_text_ostream::sequence_to_text_ostream(
   text::str const & start_delim_, text::str const & end_delim_
) :
   separator(LOFTY_SL(", ")),
   start_delim(start_delim_),
   end_delim(end_delim_) {
}

void sequence_to_text_ostream::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

sequence_to_text_ostream::~sequence_to_text_ostream() {
}

void sequence_to_text_ostream::_write_end(io::text::ostream * dst) {
   dst->write(end_delim);
}

void sequence_to_text_ostream::_write_separator(io::text::ostream * dst) {
   dst->write(separator);
}

void sequence_to_text_ostream::_write_start(io::text::ostream * dst) {
   dst->write(start_delim);
}

}} //namespace lofty::_pvt

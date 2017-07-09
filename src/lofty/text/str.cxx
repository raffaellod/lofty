/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/numeric.hxx>
#include <lofty/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/regex.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

external_buffer_t const external_buffer;

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

//! Single NUL terminator.
static char_t const nul_ch('\0');

//! Vextr referencing a static, empty, NUL-terminated raw C string.
static collections::_pvt::vextr_impl_data const empty_str_data = {
   /*begin_ptr                   =*/ const_cast<char_t *>(&nul_ch),
   /*end_ptr                     =*/ const_cast<char_t *>(&nul_ch),
   /*has_embedded_prefixed_array =*/ false,
   /*array_is_prefixed           =*/ false,
   /*dynamic                     =*/ false,
   /*has_nul_term                =*/ true
};

str const & str::empty = static_cast<str const &>(empty_str_data);


std::ptrdiff_t str::const_iterator::distance(std::size_t other_char_index) const {
   LOFTY_TRACE_FUNC(this, other_char_index);

   if (other_char_index == char_index_) {
      return 0;
   } else {
      char_t const * begin = owner_str->data();
      if (other_char_index < char_index_) {
         return static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(begin + other_char_index, begin + char_index_)
         );
      } else {
         return -static_cast<std::ptrdiff_t>(
            str_traits::size_in_codepoints(begin + char_index_, begin + other_char_index)
         );
      }
   }
}

std::size_t str::advance_index_by_codepoint_delta(
   std::size_t char_index, std::ptrdiff_t delta, bool allow_end
) const {
   char_t const * begin_ = data(), * ptr = begin_ + char_index, * end_ = data_end();

   // If i is positive, move forward.
   for (; delta > 0 && ptr < end_; --delta) {
      // Find the next code point start, skipping any trail characters.
      ptr += host_char_traits::lead_char_to_codepoint_size(*ptr);
   }
   // If i is negative, move backwards.
   for (; delta < 0 && ptr > begin_; ++delta) {
      // Moving to the previous code point requires finding the previous non-trail character.
      while (host_char_traits::is_trail_char(*--ptr)) {
         ;
      }
   }
   // Use the remainder of delta. If it was non-zero, this will bring ptr out of range.
   ptr += delta;
   /* Verify that ptr is still within range: that’s not the case if we left either for loop before delta
   reached 0, or if the pointer was invalid on entry (e.g. accessing str()[0]). */
   if (allow_end) {
      ++end_;
   }
   if (ptr < begin_ || ptr >= end_) {
      LOFTY_THROW(collections::out_of_range, (ptr, begin_, data_end()));
   }

   // Return the resulting index.
   return static_cast<std::size_t>(ptr - begin_);
}

/*static*/ std::size_t str::advance_index_by_codepoint_delta(
   str const * this_str, std::size_t char_index, std::ptrdiff_t delta, bool allow_end
) {
   if (!this_str) {
      LOFTY_THROW(collections::out_of_range, ());
   }
   return this_str->advance_index_by_codepoint_delta(char_index, delta, allow_end);
}

str::c_str_ptr str::c_str() {
   LOFTY_TRACE_FUNC(this);

   if (has_nul_term) {
      // The string already includes a NUL terminator, so we can simply return the same array.
   } else if (auto char_size = size_in_chars()) {
      // The string is not empty but lacks a NUL terminator: enlarge the string to include one.
      prepare_for_writing();
      set_capacity(char_size + 1, true);
      *data_end() = '\0';
      has_nul_term = true;
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_ptr(&nul_ch, false);
   }
   return c_str_ptr(data(), false);
}

str::c_str_ptr str::c_str() const {
   LOFTY_TRACE_FUNC(this);

   if (has_nul_term) {
      // The string already includes a NUL terminator, so we can simply return the same array.
      return c_str_ptr(data(), false);
   } else if (auto char_size = size_in_chars()) {
      /* The string is not empty but lacks a NUL terminator: create a temporary copy that includes a NUL, and
      return it. */
      auto array_copy(memory::alloc_unique<char_t[]>(char_size + 1 /*NUL*/));
      memory::copy(array_copy.get(), data(), char_size);
      array_copy[char_size] = '\0';
      return c_str_ptr(array_copy.release(), true);
   } else {
      // The string is empty, so a static NUL character will suffice.
      return c_str_ptr(&nul_ch, false);
   }
}

collections::vector<std::uint8_t> str::encode(encoding enc, bool add_nul_term) const {
   LOFTY_TRACE_FUNC(this, enc, add_nul_term);

   collections::vector<std::uint8_t> bytes;
   std::size_t char_byte_size, used_byte_size, str_byte_size = size_in_bytes();
   if (enc == encoding::host) {
      // Optimal case: no transcoding necessary.
      char_byte_size = sizeof(char_t);
      // Enlarge bytes as necessary, then copy to it the contents of the string buffer.
      bytes.set_capacity(str_byte_size + (add_nul_term ? sizeof(char_t) : 0), false);
      memory::copy(bytes.data(), vextr_impl::begin<std::uint8_t>(), str_byte_size);
      used_byte_size = str_byte_size;
   } else {
      char_byte_size = get_encoding_size(enc);
      void const * str_bytes_ptr = data();
      // Calculate the size required, then resize bytes accorgingly.
      used_byte_size = transcode(true, encoding::host, &str_bytes_ptr, &str_byte_size, enc);
      bytes.set_capacity(used_byte_size + (add_nul_term ? char_byte_size : 0), false);
      // Transcode the string into bytes.
      void * bytes_ptr = bytes.data();
      // Re-assign to used_byte_size because transcode() will set *(&used_byte_size) to 0.
      used_byte_size = transcode(
         true, encoding::host, &str_bytes_ptr, &str_byte_size, enc, &bytes_ptr, &used_byte_size
      );
   }
   if (add_nul_term) {
      memory::clear(bytes.data() + used_byte_size, char_byte_size);
      used_byte_size += char_byte_size;
   }
   // Assign the vector its size, and return it.
   bytes.set_size(used_byte_size);
   return _std::move(bytes);
}

bool str::ends_with(str const & s) const {
   LOFTY_TRACE_FUNC(this, s);

   char_t const * match_start = data_end() - s.size_in_chars();
   return match_start >= data() && str_traits::compare(match_start, data_end(), s.data(), s.data_end()) == 0;
}

str::const_iterator str::find(char_t ch, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, ch, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_char(whence_ptr, data_end(), ch);
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}
str::const_iterator str::find(char32_t cp, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, cp, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_char(whence_ptr, data_end(), cp);
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}
str::const_iterator str::find(str const & substr_, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, substr_, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_substr(whence_ptr, data_end(), substr_.data(), substr_.data_end());
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}

str::const_iterator str::find_last(char_t ch, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, ch, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_char_last(data(), whence_ptr, ch);
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}
str::const_iterator str::find_last(char32_t cp, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, cp, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_char_last(data(), whence_ptr, cp);
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}
str::const_iterator str::find_last(str const & substr_, const_iterator whence) const {
   LOFTY_TRACE_FUNC(this, substr_, whence);

   char_t const * whence_ptr = data() + whence.char_index_;
   validate_pointer(whence_ptr, true);
   auto ptr = str_traits::find_substr_last(data(), whence_ptr, substr_.data(), substr_.data_end());
   return const_iterator(this, static_cast<std::size_t>(ptr - data()));
}

void str::prepare_for_writing() {
   if (!array_is_prefixed) {
      /* Copying from itself is safe because the new character array will necessarily not overlap the old,
      non-prefixed one. */
      assign_copy(data(), data_end());
   }
}

void str::replace(char_t search, char_t replacement) {
   LOFTY_TRACE_FUNC(this, search, replacement);

   prepare_for_writing();
   for (char_t * ptr = data(), * end_ = data_end(); ptr != end_; ++ptr) {
      if (*ptr == search) {
         *ptr = replacement;
      }
   }
}

void str::replace(char32_t search, char32_t replacement) {
   LOFTY_TRACE_FUNC(this, search, replacement);

   prepare_for_writing();
   // TODO: optimize this. Using iterators requires little code but it’s not very efficient.
#if 0
   // TODO: FIXME: why does this iterate one too many times?
   LOFTY_FOR_EACH(auto cp, *this) {
      if (cp == search) {
         cp = replacement;
      }
   }
#else
   for (auto itr(begin()); itr != end(); ++itr) {
      if (*itr == search) {
         *itr = replacement;
      }
   }
#endif
}

void str::replace_codepoint(std::size_t char_index, char_t new_ch) {
   LOFTY_TRACE_FUNC(this, char_index, new_ch);

   std::size_t remove_byte_size = sizeof(char_t) * host_char_traits::lead_char_to_codepoint_size(
      data()[char_index]
   );
   // Note: either of these two calls may change data().
   prepare_for_writing();
   vextr_impl::insert_remove(char_index, nullptr, sizeof(char_t), remove_byte_size);
   data()[char_index] = new_ch;
}

void str::replace_codepoint(std::size_t char_index, char32_t new_cp) {
   LOFTY_TRACE_FUNC(this, char_index, new_cp);

   std::size_t insert_byte_size = sizeof(char_t) * host_char_traits::codepoint_size(new_cp);
   std::size_t remove_byte_size = sizeof(char_t) * host_char_traits::lead_char_to_codepoint_size(
      data()[char_index]
   );
   // Note: either of these two calls may change data().
   prepare_for_writing();
   vextr_impl::insert_remove(sizeof(char_t) * char_index, nullptr, insert_byte_size, remove_byte_size);
   // codepoint_size() validated new_cp, so nothing can go wrong here.
   host_char_traits::traits_base::codepoint_to_chars(new_cp, data() + char_index);
}

void str::set_from(_std::function<std::size_t (char_t * chars, std::size_t chars_max)> const & read_fn) {
   LOFTY_TRACE_FUNC(this/*, read_fn*/);

   prepare_for_writing();
   /* The initial size avoids a few reallocations (* growth_rate ** 2). Multiplying by growth_rate should
   guarantee that set_capacity() will allocate exactly the requested number of characters, eliminating the
   need to query back with capacity(). */
   std::size_t ret_char_size, chars_max = capacity_bytes_min * growth_rate;
   do {
      chars_max *= growth_rate;
      set_capacity(chars_max, false);
      ret_char_size = read_fn(data(), chars_max);
   } while (ret_char_size >= chars_max);
   // Finalize the length.
   set_size_in_chars(ret_char_size);
}

bool str::starts_with(str const & s) const {
   LOFTY_TRACE_FUNC(this, s);

   char_t const * end_ = data() + s.size_in_chars();
   return end_ <= data_end() && str_traits::compare(data(), end_, s.data(), s.data_end()) == 0;
}

char_t const * str::validate_index_to_pointer(std::size_t char_index, bool allow_end) const {
   auto begin_ = data(), end_ = data_end(), ptr = begin_ + char_index;
   if (allow_end) {
      ++end_;
   }
   if (ptr < begin_ || ptr >= end_) {
      LOFTY_THROW(collections::out_of_range, (ptr, begin_, end_));
   }
   return ptr;
}

/*static*/ char_t const * str::validate_index_to_pointer(
   str const * this_str, std::size_t char_index, bool allow_end
) {
   if (!this_str) {
      LOFTY_THROW(collections::out_of_range, ());
   }
   return this_str->validate_index_to_pointer(char_index, allow_end);
}

}} //namespace lofty::text

namespace std {

/* Implementation based on the Fowler/Noll/Vo variant 1a (FNV-1a) algorithm. See
<http://www.isthe.com/chongo/tech/comp/fnv/> for details.

The bases are calculated by src/fnv_hash_basis.py. */
std::size_t hash<lofty::text::str>::operator()(lofty::text::str const & s) const {
   LOFTY_TRACE_FUNC(this, s);

   static_assert(
      sizeof(std::size_t) * 8 == LOFTY_HOST_WORD_SIZE,
      "unexpected sizeof(std::size_t) will break FNV prime/basis selection"
   );
#if LOFTY_HOST_WORD_SIZE == 16
   static std::size_t const fnv_prime = 0x1135;
   static std::size_t const fnv_basis = 16635u;
#elif LOFTY_HOST_WORD_SIZE == 32
   static std::size_t const fnv_prime = 0x01000193;
   static std::size_t const fnv_basis = 2166136261u;
#elif LOFTY_HOST_WORD_SIZE == 64
   static std::size_t const fnv_prime = 0x00000100000001b3;
   static std::size_t const fnv_basis = 14695981039346656037u;
#endif
   std::size_t ret = fnv_basis;
   LOFTY_FOR_EACH(auto cp, s) {
      ret ^= static_cast<std::size_t>(cp);
      ret *= fnv_prime;
   }
   return ret;
}

} //namespace std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void from_text_istream<text::str>::convert_capture(
   text::parsers::dynamic_match_capture const & capture0, text::str * dst
) {
   LOFTY_TRACE_FUNC(this, /*capture0, */ dst);

   *dst = capture0.str_copy();
}

text::parsers::dynamic_state const * from_text_istream<text::str>::format_to_parser_states(
   text::parsers::regex_capture_format const & format, text::parsers::dynamic * parser
) {
   LOFTY_TRACE_FUNC(this/*, format*/, parser);

   // TODO: more format validation.

   if (format.expr) {
      text::parsers::regex regex(parser, format.expr);
      return regex.parse_with_no_captures();
   } else {
      // Default to “.*”.
      LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_RANGE_STATE(
         any_cp_state, nullptr, nullptr, numeric::min<char32_t>::value, numeric::max<char32_t>::value
      );
      LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(any_cp_rep_group, nullptr, nullptr, &any_cp_state.base, 0);
      return &any_cp_rep_group.base;
   }
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace _pvt {

void str_to_text_ostream::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void str_to_text_ostream::write(
   void const * src, std::size_t src_byte_size, encoding enc, io::text::ostream * dst
) {
   LOFTY_TRACE_FUNC(this, src, src_byte_size, enc, dst);

   dst->write_binary(src, src_byte_size, enc);
}

}}} //namespace lofty::text::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<text::str>::write(text::str const & src, io::text::ostream * dst) {
   text::_pvt::str_to_text_ostream::write(src.data(), static_cast<std::size_t>(
      reinterpret_cast<std::uintptr_t>(src.data_end()) - reinterpret_cast<std::uintptr_t>(src.data())
   ), text::encoding::host, dst);
}

} //namespace lofty

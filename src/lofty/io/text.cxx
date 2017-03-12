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

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/io/text.hxx>
#include <lofty/os/path.hxx>
#include <lofty/process.hxx>
#include <lofty/text.hxx>
#include "binary/file-subclasses.hxx"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

_std::shared_ptr<ostream> stderr;
_std::shared_ptr<istream> stdin;
_std::shared_ptr<ostream> stdout;

/*! Detects the encoding to use for a standard text stream, with the help of an optional environment variable.

TODO: document this behavior and the related environment variables.

TODO: make the below code only pick up variables meant for this PID. This should eventually be made more
general, as a way for a Lofty-based parent process to communicate with a Lofty-based child process. Thought
maybe a better way is to pass a command-line argument that triggers Lofty-specific behavior, so that it’s
inherently PID-specific.

@param bin_stream
   Pointer to the binary stream to analyze.
@param env_var_name
   Environment variable name that, if set, specifies the encoding to be used.
@return
   Encoding appropriate for the requested standard stream.
*/
static lofty::text::encoding get_stdio_encoding(binary::stream const * bin_stream, str const & env_var_name) {
   LOFTY_TRACE_FUNC(bin_stream, env_var_name);

   lofty::text::encoding enc;
   if (dynamic_cast<binary::tty_file_stream const *>(bin_stream)) {
      /* Console files can only perform I/O in the host platform’s encoding, so force the correct encoding
      here. */
      enc = lofty::text::encoding::host;
   } else {
      // In all other cases, allow selecting the encoding via environment variable.
      enc = lofty::text::encoding::unknown;
      sstr<64> enc_buf;
      if (this_process::env_var(env_var_name, enc_buf.str_ptr())) {
         try {
            enc = lofty::text::encoding(enc_buf.str());
         } catch (domain_error const &) {
            // Ignore this invalid encoding setting, and default to auto-detection.
            // TODO: display a warning about LOFTY_STD*_ENCODING being ignored.
         }
      }
   }
   return enc;
}


_std::shared_ptr<binbuf_istream> make_istream(
   _std::shared_ptr<binary::istream> bin_istream,
   lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) {
   LOFTY_TRACE_FUNC(bin_istream, enc);

   // See if *bin_istream is also a binary::buffered_istream.
   auto buf_bin_istream(_std::dynamic_pointer_cast<binary::buffered_istream>(bin_istream));
   if (!buf_bin_istream) {
      // Add a buffering wrapper to *bin_istream.
      buf_bin_istream = binary::buffer_istream(bin_istream);
   }
   return _std::make_shared<binbuf_istream>(_std::move(buf_bin_istream), enc);
}

_std::shared_ptr<binbuf_ostream> make_ostream(
   _std::shared_ptr<binary::ostream> bin_ostream,
   lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) {
   LOFTY_TRACE_FUNC(bin_ostream, enc);

   // See if *bin_ostream is also a binary::buffered_ostream.
   auto buf_bin_ostream(_std::dynamic_pointer_cast<binary::buffered_ostream>(bin_ostream));
   if (!buf_bin_ostream) {
      // Add a buffering wrapper to *bin_ostream.
      buf_bin_ostream = binary::buffer_ostream(bin_ostream);
   }
   return _std::make_shared<binbuf_ostream>(_std::move(buf_bin_ostream), enc);
}

_std::shared_ptr<binbuf_istream> open_istream(
   os::path const & path, lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) {
   LOFTY_TRACE_FUNC(path, enc);

   return make_istream(binary::open_istream(path));
}

_std::shared_ptr<binbuf_ostream> open_ostream(
   os::path const & path, lofty::text::encoding enc /*= lofty::text::encoding::unknown*/
) {
   LOFTY_TRACE_FUNC(path, enc);

   return make_ostream(binary::open_ostream(path));
}

}}} //namespace lofty::io::text

namespace lofty { namespace io { namespace text { namespace _pvt {

_std::shared_ptr<ostream> make_stderr() {
   LOFTY_TRACE_FUNC();

   auto bin_ostream(binary::stderr);
   // See if *bin_ostream is also a binary::buffered_ostream.
   auto buf_bin_ostream(_std::dynamic_pointer_cast<binary::buffered_ostream>(bin_ostream));
   if (!buf_bin_ostream) {
      // Add a buffering wrapper to *bin_ostream.
      buf_bin_ostream = binary::buffer_ostream(bin_ostream);
   }
   auto enc = get_stdio_encoding(bin_ostream.get(), LOFTY_SL("LOFTY_STDERR_ENCODING"));
   return _std::make_shared<binbuf_ostream>(_std::move(buf_bin_ostream), enc);
}

_std::shared_ptr<istream> make_stdin() {
   LOFTY_TRACE_FUNC();

   auto bin_istream(binary::stdin);
   // See if *bin_istream is also a binary::buffered_istream.
   auto buf_bin_istream(_std::dynamic_pointer_cast<binary::buffered_istream>(bin_istream));
   if (!buf_bin_istream) {
      // Add a buffering wrapper to *bin_istream.
      buf_bin_istream = binary::buffer_istream(bin_istream);
   }
   auto enc = get_stdio_encoding(bin_istream.get(), LOFTY_SL("LOFTY_STDIN_ENCODING"));
   return _std::make_shared<binbuf_istream>(_std::move(buf_bin_istream), enc);
}

_std::shared_ptr<ostream> make_stdout() {
   LOFTY_TRACE_FUNC();

   auto bin_ostream(binary::stdout);
   // See if *pbw is also a binary::buffered_ostream.
   auto buf_bin_ostream(_std::dynamic_pointer_cast<binary::buffered_ostream>(bin_ostream));
   if (!buf_bin_ostream) {
      // Add a buffering wrapper to *bin_ostream.
      buf_bin_ostream = binary::buffer_ostream(bin_ostream);
   }
   auto enc = get_stdio_encoding(bin_ostream.get(), LOFTY_SL("LOFTY_STDOUT_ENCODING"));
   return _std::make_shared<binbuf_ostream>(_std::move(buf_bin_ostream), enc);
}

}}}} //namespace lofty::io::text::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

stream::stream() :
   lterm(lofty::text::line_terminator::any) {
}

/*virtual*/ stream::~stream() {
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

istream::istream() :
   stream(),
   discard_next_lf(false) {
}

str istream::read_all() {
   LOFTY_TRACE_FUNC(this);

   str dst;
   read_all(&dst);
   return _std::move(dst);
}

/*virtual*/ void istream::read_all(str * dst) {
   LOFTY_TRACE_FUNC(this, dst);

   dst->clear();
   // Just ask for 1 character; that’s enough to distinguish between EOF and non-EOF.
   while (str src = peek_chars(1)) {
      std::size_t consumed_count = src.size_in_chars();
      *dst += src;
      consume_chars(consumed_count);
   }
}

/*virtual*/ bool istream::read_line(str * dst) {
   LOFTY_TRACE_FUNC(this, dst);

   dst->clear();
   std::size_t consumed_total = 0, dst_char_size = 0;
   bool lterm_found = false;
   str src;
   // Just ask for 1 character; that’s sufficient to distinguish between EOF and non-EOF.
   while (!lterm_found && (src = peek_chars(1))) {
      // Resize *dst to accommodate, potentially, all of src.
      dst->set_capacity(dst_char_size + src.size_in_chars(), true /*preserve*/);
      // Copy characters from src to *dst, stopping at the first line terminator.
      char_t const * src_chars = src.data(), * src_chars_end = src.data_end();
      char_t * dst_chars = dst->data() + dst_char_size;
      char_t const * dst_last_cr = nullptr;
      /* If the last character parsed by prior invocation of read_line() was a CR and this first character is
      a LF, skip past it. */
      if (discard_next_lf) {
         discard_next_lf = false;
         if (*src_chars == '\n' /*LF*/) {
            ++src_chars;
         }
      }
      while (src_chars != src_chars_end) {
         char_t src_ch = *src_chars++;
         if (src_ch == '\r' /*CR*/) {
            switch (lterm.base()) {
               case lofty::text::line_terminator::any:
                  // Make sure we’ll discard a possible following LF.
                  discard_next_lf = true;
                  // Fall through.
               case lofty::text::line_terminator::cr:
                  lterm_found = true;
                  goto break_inner_while;
               case lofty::text::line_terminator::cr_lf:
                  /* Mark where we’re about to write the CR, so we can rewind to this - 1 if the next source
                  character is LF. */
                  dst_last_cr = dst_chars;
                  break;
               case lofty::text::line_terminator::lf:
                  break;
            }
         } else if (src_ch == '\n' /*LF*/) {
            switch (lterm.base()) {
               case lofty::text::line_terminator::any:
               case lofty::text::line_terminator::lf:
                  lterm_found = true;
                  goto break_inner_while;
               case lofty::text::line_terminator::cr_lf: {
                  /* If the previous character was a CR, dst_last_cr was set to it; in that case don’t write
                  this LF and discard the already-written CR. */
                  char_t * dst_prev_char = dst_chars - 1;
                  if (dst_last_cr == dst_prev_char) {
                     dst_chars = dst_prev_char;
                     goto break_inner_while;
                  }
                  break;
               }
               case lofty::text::line_terminator::cr:
                  break;
            }
         }
         *dst_chars++ = src_ch;
      }
   break_inner_while:
      if (std::size_t consumed_count = static_cast<std::size_t>(src_chars - src.data())) {
         consume_chars(consumed_count);
         consumed_total += consumed_count;
      }
      // Save this, since the next iteration might reallocate *dst’s character array.
      dst_char_size = static_cast<std::size_t>(dst_chars - dst->data());
   }
   dst->set_size_in_chars(dst_char_size);
   return consumed_total > 0;
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {

ostream::ostream() :
   stream() {
}

void ostream::write(str const & s) {
   LOFTY_TRACE_FUNC(this, s);

   write_binary(s.data(), static_cast<std::size_t>(
      reinterpret_cast<std::uintptr_t>(s.data_end()) - reinterpret_cast<std::uintptr_t>(s.data())
   ), lofty::text::encoding::host);
}

void ostream::write_line(str const & s) {
   LOFTY_TRACE_FUNC(this, s);

   write(s);
   write(get_line_terminator_str(
      // If no line terminator sequence has been explicitly set, use the platform’s default.
      lterm != lofty::text::line_terminator::any ? lterm : lofty::text::line_terminator::host
   ));
}

}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text { namespace _pvt {

ostream_print_helper_impl::ostream_print_helper_impl(class ostream * ostream_, str const & format_) :
   ostream(ostream_),
   // write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
   last_used_arg_index(static_cast<unsigned>(-1)),
   format(format_),
   format_to_write_begin_itr(format.cbegin()) {
}

void ostream_print_helper_impl::run() {
   // Since this specialization has no replacements, verify that the format string doesn’t specify any either.
   if (write_format_up_to_next_repl()) {
      throw_collections_out_of_range();
   }
}

void ostream_print_helper_impl::throw_collections_out_of_range() {
   auto arg_index = static_cast<std::ptrdiff_t>(last_used_arg_index);
   LOFTY_THROW(collections::out_of_range, (arg_index, 0, arg_index - 1));
}

bool ostream_print_helper_impl::write_format_up_to_next_repl() {
   LOFTY_TRACE_FUNC(this);

   // Search for the next replacement, if any.
   str::const_iterator itr(format_to_write_begin_itr), repl_field_begin, end(format.cend());
   char32_t ch;
   for (;;) {
      if (itr >= end) {
         // The format string is over; write any characters not yet written.
         write_format_up_to(end);
         // Report that no more replacement fields were found.
         return false;
      }
      ch = *itr++;
      if (ch == '{' || ch == '}') {
         if (ch == '{') {
            // Mark the beginning of the replacement field.
            repl_field_begin = itr - 1;
            if (itr >= end) {
               throw_syntax_error(LOFTY_SL("unmatched '{' in format string"), repl_field_begin);
            }
            ch = *itr;
            if (ch != '{') {
               // We found the beginning of a replacement field.
               break;
            }
         } else if (ch == '}') {
            if (itr >= end || *itr != '}') {
               throw_syntax_error(LOFTY_SL("single '}' encountered in format string"), itr - 1);
            }
         }
         // Convert “{{” into “{” or “}}” into “}”.
         // Write up to and including the first brace.
         write_format_up_to(itr);
         // The next call to write_format_up_to() will skip the second brace.
         format_to_write_begin_itr = ++itr;
      }
   }

   // Check if we have an argument index.
   if (ch >= '0' && ch <= '9') {
      // Consume as many digits as there are, and convert them into the argument index.
      unsigned arg_index = 0;
      for (;;) {
         arg_index += static_cast<unsigned>(ch - '0');
         if (++itr >= end) {
            throw_syntax_error(LOFTY_SL("unmatched '{' in format string"), repl_field_begin);
         }
         ch = *itr;
         if (ch < '0' || ch > '9') {
            break;
         }
         arg_index *= 10;
      }
      // Save this index as the last used one.
      last_used_arg_index = arg_index;
   } else {
      // The argument index is missing, so just use the next one.
      ++last_used_arg_index;
   }

   // Check for a format specification.
   if (ch == ':') {
      if (++itr >= end) {
         throw_syntax_error(LOFTY_SL("expected format specification"), itr);
      }
      repl_format_spec_begin = itr.ptr();
      // Find the end of the replacement field.
      itr = format.find('}', itr);
      if (itr == end) {
         throw_syntax_error(LOFTY_SL("unmatched '{' in format string"), repl_field_begin);
      }
      repl_format_spec_end = itr.ptr();
   } else {
      // If there’s no format specification, it must be the end of the replacement field.
      if (ch != '}') {
         throw_syntax_error(LOFTY_SL("unmatched '{' in format string"), repl_field_begin);
      }
      // Set the format specification to nothing.
      repl_format_spec_begin = nullptr;
      repl_format_spec_end = nullptr;
   }

   // Write the format string characters up to the beginning of the replacement.
   write_format_up_to(repl_field_begin);
   // Update this, so the next call to write_format_up_to() will skip over this replacement field.
   format_to_write_begin_itr = itr + 1 /*'}'*/;
   // Report that a substitution must be written.
   return true;
}

void ostream_print_helper_impl::throw_syntax_error(str const & description, str::const_iterator itr) const {
   LOFTY_THROW(lofty::text::syntax_error, (
      // +1 because the first character is 1, to human beings.
      description, format, static_cast<unsigned>(itr - format.cbegin() + 1)
   ));
}

void ostream_print_helper_impl::write_format_up_to(str::const_iterator up_to) {
   LOFTY_TRACE_FUNC(this, up_to);

   if (up_to > format_to_write_begin_itr) {
      ostream->write_binary(
         format_to_write_begin_itr.ptr(),
         sizeof(char_t) * (up_to.char_index() - format_to_write_begin_itr.char_index()),
         lofty::text::encoding::host
      );
      format_to_write_begin_itr = up_to;
   }
}

}}}} //namespace lofty::io::text::_pvt

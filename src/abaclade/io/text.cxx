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

#include <abaclade.hxx>
#include <abaclade/collections.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/os/path.hxx>
#include <abaclade/process.hxx>
#include <abaclade/text.hxx>
#include "binary/file-subclasses.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

_std::shared_ptr<ostream> stderr;
_std::shared_ptr<istream> stdin;
_std::shared_ptr<ostream> stdout;

/*! Detects the encoding to use for a standard text stream, with the help of an optional environment
variable.

TODO: document this behavior and the related environment variables.

TODO: make the below code only pick up variables meant for this PID. This should eventually be made
more general, as a way for an Abaclade-based parent process to communicate with an Abaclade-based
child process. Thought maybe a better way is to pass a command-line argument that triggers Abaclade-
specific behavior, so that it’s inherently PID-specific.

@param pbs
   Pointer to the binary stream to analyze.
@param sEnvVarName
   Environment variable name that, if set, specifies the encoding to be used.
@return
   Encoding appropriate for the requested standard stream.
*/
static abc::text::encoding get_stdio_encoding(binary::stream const * pbs, str const & sEnvVarName) {
   ABC_TRACE_FUNC(pbs, sEnvVarName);

   abc::text::encoding enc;
   if (dynamic_cast<binary::tty_file_stream const *>(pbs)) {
      /* Console files can only perform I/O in the host platform’s encoding, so force the correct
      encoding here. */
      enc = abc::text::encoding::host;
   } else {
      // In all other cases, allow selecting the encoding via environment variable.
      enc = abc::text::encoding::unknown;
      sstr<64> sEnc;
      if (this_process::env_var(sEnvVarName, sEnc.str_ptr())) {
         try {
            enc = abc::text::encoding(sEnc.str());
         } catch (domain_error const &) {
            // Ignore this invalid encoding setting, and default to auto-detection.
            // TODO: display a warning about ABC_STD*_ENCODING being ignored.
         }
      }
   }
   return enc;
}


_std::shared_ptr<binbuf_istream> make_istream(
   _std::shared_ptr<binary::istream> pbis,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(pbis, enc);

   // See if *pbis is also a binary::buffered_istream.
   auto pbbis(_std::dynamic_pointer_cast<binary::buffered_istream>(pbis));
   if (!pbbis) {
      // Add a buffering wrapper to *pbis.
      pbbis = binary::buffer_istream(pbis);
   }
   return _std::make_shared<binbuf_istream>(_std::move(pbbis), enc);
}

_std::shared_ptr<binbuf_ostream> make_ostream(
   _std::shared_ptr<binary::ostream> pbos,
   abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(pbos, enc);

   // See if *pbos is also a binary::buffered_ostream.
   auto pbbos(_std::dynamic_pointer_cast<binary::buffered_ostream>(pbos));
   if (!pbbos) {
      // Add a buffering wrapper to *pbos.
      pbbos = binary::buffer_ostream(pbos);
   }
   return _std::make_shared<binbuf_ostream>(_std::move(pbbos), enc);
}

_std::shared_ptr<binbuf_istream> open_istream(
   os::path const & op, abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(op, enc);

   return make_istream(binary::open_istream(op));
}

_std::shared_ptr<binbuf_ostream> open_ostream(
   os::path const & op, abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(op, enc);

   return make_ostream(binary::open_ostream(op));
}

}}} //namespace abc::io::text

namespace abc { namespace io { namespace text { namespace _pvt {

_std::shared_ptr<ostream> make_stderr() {
   ABC_TRACE_FUNC();

   auto pbos(binary::stderr);
   // See if *pbos is also a binary::buffered_ostream.
   auto pbbos(_std::dynamic_pointer_cast<binary::buffered_ostream>(pbos));
   if (!pbbos) {
      // Add a buffering wrapper to *pbos.
      pbbos = binary::buffer_ostream(pbos);
   }
   auto enc = get_stdio_encoding(pbos.get(), ABC_SL("ABC_STDERR_ENCODING"));
   return _std::make_shared<binbuf_ostream>(_std::move(pbbos), enc);
}

_std::shared_ptr<istream> make_stdin() {
   ABC_TRACE_FUNC();

   auto pbis(binary::stdin);
   // See if *pbis is also a binary::buffered_istream.
   auto pbbis(_std::dynamic_pointer_cast<binary::buffered_istream>(pbis));
   if (!pbbis) {
      // Add a buffering wrapper to *pbis.
      pbbis = binary::buffer_istream(pbis);
   }
   auto enc = get_stdio_encoding(pbis.get(), ABC_SL("ABC_STDIN_ENCODING"));
   return _std::make_shared<binbuf_istream>(_std::move(pbbis), enc);
}

_std::shared_ptr<ostream> make_stdout() {
   ABC_TRACE_FUNC();

   auto pbos(binary::stdout);
   // See if *pbw is also a binary::buffered_ostream.
   auto pbbos(_std::dynamic_pointer_cast<binary::buffered_ostream>(pbos));
   if (!pbbos) {
      // Add a buffering wrapper to *pbos.
      pbbos = binary::buffer_ostream(pbos);
   }
   auto enc = get_stdio_encoding(pbos.get(), ABC_SL("ABC_STDOUT_ENCODING"));
   return _std::make_shared<binbuf_ostream>(_std::move(pbbos), enc);
}

}}}} //namespace abc::io::text::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

stream::stream() :
   m_lterm(abc::text::line_terminator::any) {
}

/*virtual*/ stream::~stream() {
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

istream::istream() :
   stream(),
   m_bDiscardNextLF(false) {
}

str istream::read_all() {
   ABC_TRACE_FUNC(this);

   str sDst;
   read_all(&sDst);
   return _std::move(sDst);
}

/*virtual*/ void istream::read_all(str * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   psDst->clear();
   // Just ask for 1 character; that’s enough to distinguish between EOF and non-EOF.
   while (str sSrc = peek_chars(1)) {
      std::size_t cchConsumed = sSrc.size_in_chars();
      *psDst += sSrc;
      consume_chars(cchConsumed);
   }
}

/*virtual*/ bool istream::read_line(str * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   psDst->clear();
   std::size_t cchConsumedTotal = 0, cchDst = 0;
   bool bFoundLTerm = false;
   str sSrc;
   // Just ask for 1 character; that’s sufficient to distinguish between EOF and non-EOF.
   while (!bFoundLTerm && (sSrc = peek_chars(1))) {
      // Resize *psDst to accommodate, potentially, all of sSrc.
      psDst->set_capacity(cchDst + sSrc.size_in_chars(), true /*preserve*/);
      // Copy characters from sSrc to *psDst, stopping at the first line terminator.
      char_t const * pchSrc = sSrc.data(), * pchSrcEnd = sSrc.data_end();
      char_t * pchDst = psDst->data() + cchDst;
      char_t const * pchDstLastCR = nullptr;
      /* If the last character parsed by prior invocation of read_line() was a CR and this first
      character is a LF, skip past it. */
      if (m_bDiscardNextLF) {
         m_bDiscardNextLF = false;
         if (*pchSrc == '\n' /*LF*/) {
            ++pchSrc;
         }
      }
      while (pchSrc != pchSrcEnd) {
         char_t chSrc = *pchSrc++;
         if (chSrc == '\r' /*CR*/) {
            switch (m_lterm.base()) {
               case abc::text::line_terminator::any:
                  // Make sure we’ll discard a possible following LF.
                  m_bDiscardNextLF = true;
                  // Fall through.
               case abc::text::line_terminator::cr:
                  bFoundLTerm = true;
                  goto break_inner_while;
               case abc::text::line_terminator::cr_lf:
                  /* Mark where we’re about to write the CR, so we can rewind to this - 1 if the
                  next source character is LF. */
                  pchDstLastCR = pchDst;
                  break;
               case abc::text::line_terminator::lf:
                  break;
            }
         } else if (chSrc == '\n' /*LF*/) {
            switch (m_lterm.base()) {
               case abc::text::line_terminator::any:
               case abc::text::line_terminator::lf:
                  bFoundLTerm = true;
                  goto break_inner_while;
               case abc::text::line_terminator::cr_lf: {
                  /* If the previous character was a CR, pchDstLastCR was set to it; in that case
                  don’t write this LF and discard the already-written CR. */
                  char_t * pchDstPrev = pchDst - 1;
                  if (pchDstLastCR == pchDstPrev) {
                     pchDst = pchDstPrev;
                     goto break_inner_while;
                  }
                  break;
               }
               case abc::text::line_terminator::cr:
                  break;
            }
         }
         *pchDst++ = chSrc;
      }
   break_inner_while:
      if (std::size_t cchConsumed = static_cast<std::size_t>(pchSrc - sSrc.data())) {
         consume_chars(cchConsumed);
         cchConsumedTotal += cchConsumed;
      }
      // Save this, since the next iteration might reallocate *psDst’s character array.
      cchDst = static_cast<std::size_t>(pchDst - psDst->data());
   }
   psDst->set_size_in_chars(cchDst);
   return cchConsumedTotal > 0;
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

ostream::ostream() :
   stream() {
}

void ostream::write(str const & s) {
   ABC_TRACE_FUNC(this, s);

   write_binary(s.data(), static_cast<std::size_t>(
      reinterpret_cast<std::uintptr_t>(s.data_end()) - reinterpret_cast<std::uintptr_t>(s.data())
   ), abc::text::encoding::host);
}

void ostream::write_line(str const & s) {
   ABC_TRACE_FUNC(this, s);

   write(s);
   abc::text::line_terminator lterm;
   // If no line terminator sequence has been explicitly set, use the platform’s default.
   if (m_lterm == abc::text::line_terminator::any) {
      lterm = abc::text::line_terminator::host;
   } else {
      lterm = m_lterm;
   }
   write(get_line_terminator_str(lterm));
}

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text { namespace _pvt {

ostream_print_helper_impl::ostream_print_helper_impl(ostream * ptos, str const & sFormat) :
   m_ptos(ptos),
   // write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
   m_iSubstArg(static_cast<unsigned>(-1)),
   m_sFormat(sFormat),
   m_itFormatToWriteBegin(sFormat.cbegin()) {
}

void ostream_print_helper_impl::run() {
   /* Since this specialization has no replacements, verify that the format string doesn’t specify
   any either. */
   if (write_format_up_to_next_repl()) {
      throw_collections_out_of_range();
   }
}

void ostream_print_helper_impl::throw_collections_out_of_range() {
   std::ptrdiff_t iSubstArg = static_cast<std::ptrdiff_t>(m_iSubstArg);
   ABC_THROW(collections::out_of_range, (iSubstArg, 0, iSubstArg - 1));
}

bool ostream_print_helper_impl::write_format_up_to_next_repl() {
   ABC_TRACE_FUNC(this);

   // Search for the next replacement, if any.
   str::const_iterator it(m_itFormatToWriteBegin), itReplFieldBegin, itEnd(m_sFormat.cend());
   char32_t ch;
   for (;;) {
      if (it >= itEnd) {
         // The format string is over; write any characters not yet written.
         write_format_up_to(itEnd);
         // Report that no more replacement fields were found.
         return false;
      }
      ch = *it++;
      if (ch == '{' || ch == '}') {
         if (ch == '{') {
            // Mark the beginning of the replacement field.
            itReplFieldBegin = it - 1;
            if (it >= itEnd) {
               throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
            }
            ch = *it;
            if (ch != '{') {
               // We found the beginning of a replacement field.
               break;
            }
         } else if (ch == '}') {
            if (it >= itEnd || *it != '}') {
               throw_syntax_error(ABC_SL("single '}' encountered in format string"), it - 1);
            }
         }
         // Convert “{{” into “{” or “}}” into “}”.
         // Write up to and including the first brace.
         write_format_up_to(it);
         // The next call to write_format_up_to() will skip the second brace.
         m_itFormatToWriteBegin = ++it;
      }
   }

   // Check if we have an argument index.
   if (ch >= '0' && ch <= '9') {
      // Consume as many digits as there are, and convert them into the argument index.
      unsigned iArg(0);
      for (;;) {
         iArg += static_cast<unsigned>(ch - '0');
         if (++it >= itEnd) {
            throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
         }
         ch = *it;
         if (ch < '0' || ch > '9') {
            break;
         }
         iArg *= 10;
      }
      // Save this index as the last used one.
      m_iSubstArg = iArg;
   } else {
      // The argument index is missing, so just use the next one.
      ++m_iSubstArg;
   }

   // Check for a format specification.
   if (ch == ':') {
      if (++it >= itEnd) {
         throw_syntax_error(ABC_SL("expected format specification"), it);
      }
      m_pchReplFormatSpecBegin = it.ptr();
      // Find the end of the replacement field.
      it = m_sFormat.find('}', it);
      if (it == itEnd) {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      m_pchReplFormatSpecEnd = it.ptr();
   } else {
      // If there’s no format specification, it must be the end of the replacement field.
      if (ch != '}') {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      // Set the format specification to nothing.
      m_pchReplFormatSpecBegin = nullptr;
      m_pchReplFormatSpecEnd = nullptr;
   }

   // Write the format string characters up to the beginning of the replacement.
   write_format_up_to(itReplFieldBegin);
   // Update this, so the next call to write_format_up_to() will skip over this replacement field.
   m_itFormatToWriteBegin = it + 1 /*'}'*/;
   // Report that a substitution must be written.
   return true;
}

void ostream_print_helper_impl::throw_syntax_error(
   str const & sDescription, str::const_iterator it
) const {
   // +1 because the first character is 1, to human beings.
   ABC_THROW(abc::text::syntax_error, (
      sDescription, m_sFormat, static_cast<unsigned>(it - m_sFormat.cbegin() + 1)
   ));
}

void ostream_print_helper_impl::write_format_up_to(str::const_iterator itUpTo) {
   ABC_TRACE_FUNC(this, itUpTo);

   if (itUpTo > m_itFormatToWriteBegin) {
      m_ptos->write_binary(
         m_itFormatToWriteBegin.ptr(),
         sizeof(char_t) * (itUpTo.char_index() - m_itFormatToWriteBegin.char_index()),
         abc::text::encoding::host
      );
      m_itFormatToWriteBegin = itUpTo;
   }
}

}}}} //namespace abc::io::text::_pvt

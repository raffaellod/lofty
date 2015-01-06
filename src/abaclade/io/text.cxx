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

#include <abaclade.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::base

namespace abc {
namespace io {
namespace text {

base::base() :
   m_lterm(abc::text::line_terminator::convert_any_to_lf) {
}

/*virtual*/ base::~base() {
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::reader

namespace abc {
namespace io {
namespace text {

reader::reader() :
   base() {
}

dmstr reader::read_all() {
   ABC_TRACE_FUNC(this);

   dmstr sDst;
   read_while(&sDst, false);
   return std::move(sDst);
}
void reader::read_all(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   read_while(psDst, false);
}

bool reader::read_line(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   bool bEOF = read_while(psDst, true);

   // Strip the line terminator, if any.
   std::size_t cchLTerm = 0;
   if (
      m_lterm == abc::text::line_terminator::any ||
      m_lterm == abc::text::line_terminator::convert_any_to_lf
   ) {
      // Reading stopped at the first CR or LF, so removing either from its end will cause it to
      // contain none.
      char_t const * pch = psDst->chars_end() - 1, * pchBegin = psDst->chars_begin();
      if (pch != pchBegin) {
         char_t ch = *pch;
         if (ch == '\n' || ch == '\r') {
            cchLTerm = 1;
         }
      }
   } else {
      // Pick the appropriate line terminator string; if the string ends in that, strip it off
      // before returning.
      istr const sLTerm(abc::text::get_line_terminator_str(m_lterm));
      if (psDst->ends_with(sLTerm)) {
         cchLTerm = sLTerm.size_in_chars();
      }
   }
   // Remove the line terminator from the end of the string, if we found one.
   if (cchLTerm) {
      psDst->set_size_in_chars(psDst->size_in_chars() - cchLTerm);
   }

   return bEOF;
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::writer

namespace abc {
namespace io {
namespace text {

writer::writer() :
   base() {
}

void writer::write_line(istr const & s) {
   ABC_TRACE_FUNC(this, s);

   to_str_backend<istr> tsb;
   tsb.write(s, this);
   abc::text::line_terminator lterm;
   // If no line terminator sequence has been explicitly set, use the platform’s default.
   if (m_lterm == abc::text::line_terminator::any) {
      lterm = abc::text::line_terminator::host;
   } else {
      lterm = m_lterm;
   }
   tsb.write(get_line_terminator_str(lterm), this);
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::detail::writer_print_helper

namespace abc {
namespace io {
namespace text {
namespace detail {

writer_print_helper_impl::writer_print_helper_impl(writer * ptw, istr const & sFormat) :
   m_ptw(ptw),
   // write_format_up_to_next_repl() will increment this to 0 or set it to a non-negative number.
   m_iSubstArg(static_cast<unsigned>(-1)),
   m_sFormat(sFormat),
   m_itFormatToWriteBegin(sFormat.cbegin()) {
}

void writer_print_helper_impl::run() {
   // Since this specialization has no replacements, verify that the format string doesn’t specify
   // any either.
   if (write_format_up_to_next_repl()) {
      ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
   }
}

void writer_print_helper_impl::throw_index_error() {
   ABC_THROW(index_error, (static_cast<std::ptrdiff_t>(m_iSubstArg)));
}

bool writer_print_helper_impl::write_format_up_to_next_repl() {
   ABC_TRACE_FUNC(this);

   // Search for the next replacement, if any.
   istr::const_iterator it(m_itFormatToWriteBegin), itReplFieldBegin, itEnd(m_sFormat.cend());
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
      m_pchReplFormatSpecBegin = it.base();
      // Find the end of the replacement field.
      it = m_sFormat.find('}', it);
      if (it == itEnd) {
         throw_syntax_error(ABC_SL("unmatched '{' in format string"), itReplFieldBegin);
      }
      m_pchReplFormatSpecEnd = it.base();
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

void writer_print_helper_impl::throw_syntax_error(
   istr const & sDescription, istr::const_iterator it
) const {
   // +1 because the first character is 1, to human beings.
   ABC_THROW(
      syntax_error, (sDescription, m_sFormat, static_cast<unsigned>(it - m_sFormat.cbegin() + 1))
   );
}

void writer_print_helper_impl::write_format_up_to(istr::const_iterator itUpTo) {
   ABC_TRACE_FUNC(this, itUpTo);

   if (itUpTo > m_itFormatToWriteBegin) {
      m_ptw->write_binary(
         m_itFormatToWriteBegin.base(),
         reinterpret_cast<std::size_t>(itUpTo.base()) -
            reinterpret_cast<std::size_t>(m_itFormatToWriteBegin.base()),
         abc::text::encoding::host
      );
      m_itFormatToWriteBegin = itUpTo;
   }
}

} //namespace detail
} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

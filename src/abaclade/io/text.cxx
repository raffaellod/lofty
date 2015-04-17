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

#if ABC_HOST_API_POSIX
   #include <cstdlib> // std::getenv()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text globals

namespace abc {
namespace io {
namespace text {

std::shared_ptr<binbuf_writer> stderr;
std::shared_ptr<binbuf_reader> stdin;
std::shared_ptr<binbuf_writer> stdout;

/*! Instantiates a text::base specialization appropriate for the specified binary I/O object,
returning a shared pointer to it. If the binary I/O object does not implement buffering, a buffered
I/O wrapper is instanciated as well.

pbb
   Pointer to a binary I/O object.
return
   Shared pointer to the newly created object.
*/
static std::shared_ptr<binbuf_base> _construct(
   std::shared_ptr<binary::base> pbb, abc::text::encoding enc
) {
   ABC_TRACE_FUNC(pbb, enc);

   // Choose what type of text I/O object to create based on what type of binary I/O object we got.

   // Check if it’s a buffered I/O object.
   auto pbbr(std::dynamic_pointer_cast<binary::buffered_reader>(pbb));
   auto pbbw(std::dynamic_pointer_cast<binary::buffered_writer>(pbb));
   if (!pbbr && !pbbw) {
      // Not a buffered I/O object? Get one then, and try again with the casts.
      auto pbbb(binary::buffer(pbb));
      pbbr = std::dynamic_pointer_cast<binary::buffered_reader>(pbbb);
      pbbw = std::dynamic_pointer_cast<binary::buffered_writer>(pbbb);
   }

   // Now we must have a buffered reader or writer, or pbb is not something we can use.
   if (pbbr) {
      return std::make_shared<binbuf_reader>(std::move(pbbr), enc);
   }
   if (pbbw) {
      return std::make_shared<binbuf_writer>(std::move(pbbw), enc);
   }
   // TODO: use a better exception class.
   ABC_THROW(argument_error, ());
}

/*! Detects the encoding to use for a standard text I/O file, with the help of an optional
environment variable.

TODO: document this behavior and the related enviroment variables.

TODO: change to use a global “environment” map object instead of this ad-hoc code.

TODO: make the below code only pick up variables meant for this PID. This should eventually be made
more general, as a way for an Abaclade-based parent process to communicate with an Abaclade-based
child process. Thought maybe a better way is to pass a command-line argument that triggers Abaclade-
specific behavior, so that it’s inherently PID-specific.

pszEnvVarName
   Environment variable name that, if set, specifies the encoding to be used.
return
   Encoding appropriate for the requested standard I/O file.
*/
static std::shared_ptr<binbuf_base> _construct_stdio(
   std::shared_ptr<binary::base> pbb, char_t const * pszEnvVarName
) {
   ABC_TRACE_FUNC(pbb, pszEnvVarName);

   abc::text::encoding enc;
   if (std::dynamic_pointer_cast<binary::console_file_base>(pbb)) {
      /* Console files can only perform I/O in the host platform’s encoding, so force the correct
      encoding here. */
      enc = abc::text::encoding::host;
   } else {
      // In all other cases, allow selecting the encoding via environment variable.
#if ABC_HOST_API_POSIX
      istr sEnc;
      if (char_t const * pszEnvVarValue = std::getenv(pszEnvVarName)) {
         sEnc = istr(external_buffer, pszEnvVarValue);
      }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
      smstr<64> sEnc;
      sEnc.set_from([pszEnvVarName] (char_t * pch, std::size_t cchMax) -> std::size_t {
         /* ::GetEnvironmentVariable() returns < cchMax (length without NUL) if the buffer was large
         enough, or the required size (length including NUL) otherwise. */
         return ::GetEnvironmentVariable(pszEnvVarName, pch, static_cast<DWORD>(cchMax));
      });
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
      enc = abc::text::encoding::unknown;
      if (sEnc) {
         try {
            enc = abc::text::encoding(sEnc);
         } catch (domain_error const &) {
            // Ignore this invalid encoding setting, and default to auto-detection.
            // TODO: display a warning about ABC_STD*_ENCODING being ignored.
         }
      }
   }
   return _construct(std::move(pbb), enc);
}


std::shared_ptr<reader> make_reader(
   std::shared_ptr<binary::reader> pbr, abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(pbr, enc);

   return std::make_shared<binbuf_reader>(
      std::make_shared<binary::default_buffered_reader>(std::move(pbr)), enc
   );
}

std::shared_ptr<writer> make_writer(
   std::shared_ptr<binary::writer> pbw, abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(pbw, enc);

   return std::make_shared<binbuf_writer>(
      std::make_shared<binary::default_buffered_writer>(std::move(pbw)), enc
   );
}

namespace detail {

std::shared_ptr<binbuf_writer> make_stderr() {
   ABC_TRACE_FUNC();

   return std::dynamic_pointer_cast<binbuf_writer>(
      _construct_stdio(binary::stderr, ABC_SL("ABC_STDERR_ENCODING"))
   );
}

std::shared_ptr<binbuf_reader> make_stdin() {
   ABC_TRACE_FUNC();

   return std::dynamic_pointer_cast<binbuf_reader>(
      _construct_stdio(binary::stdin, ABC_SL("ABC_STDIN_ENCODING"))
   );
}

std::shared_ptr<binbuf_writer> make_stdout() {
   ABC_TRACE_FUNC();

   return std::dynamic_pointer_cast<binbuf_writer>(
      _construct_stdio(binary::stdout, ABC_SL("ABC_STDOUT_ENCODING"))
   );
}

} //namespace detail

std::shared_ptr<binbuf_base> open(
   os::path const & op, access_mode am, abc::text::encoding enc /*= abc::text::encoding::unknown*/
) {
   ABC_TRACE_FUNC(op, am, enc);

   return _construct(binary::open(op, am), enc);
}

} //namespace text
} //namespace io
} //namespace abc

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
   read_line_or_all(&sDst, false);
   return std::move(sDst);
}
void reader::read_all(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   psDst->clear();
   read_line_or_all(psDst, false);
}

bool reader::read_line(mstr * psDst) {
   ABC_TRACE_FUNC(this, psDst);

   psDst->clear();
   return read_line_or_all(psDst, true);
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
   /* Since this specialization has no replacements, verify that the format string doesn’t specify
   any either. */
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

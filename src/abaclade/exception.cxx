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
   #include <errno.h> // E*
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

// Default translations between exception class to OS-specific error code.
#if ABC_HOST_API_POSIX
   ABC_MAP_ERROR_CLASS_TO_ERRINT(argument_error, EINVAL);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(domain_error, EDOM);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(file_not_found_error, ENOENT);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(io_error, EIO);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_address_error, EFAULT);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(overflow_error, EOVERFLOW);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(null_pointer_error, EFAULT);
#elif ABC_HOST_API_WIN32
   ABC_MAP_ERROR_CLASS_TO_ERRINT(file_not_found_error, ERROR_PATH_NOT_FOUND);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(invalid_path_error, ERROR_BAD_PATHNAME);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_address_error, ERROR_INVALID_ADDRESS);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_allocation_error, ERROR_NOT_ENOUGH_MEMORY);
   ABC_MAP_ERROR_CLASS_TO_ERRINT(null_pointer_error, ERROR_INVALID_ADDRESS);
#endif

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::source_location

namespace abc {

void to_str_backend<source_location>::set_format(istr const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_str_backend<source_location>::write(
   source_location const & srcloc, io::text::writer * ptwOut
) {
   ABC_TRACE_FUNC(this, srcloc, ptwOut);

   ptwOut->write(istr(external_buffer, srcloc.file_path()));
   ptwOut->write(ABC_SL(":"));
   ptwOut->write(srcloc.line_number());
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception

namespace abc {

exception::exception() :
   m_pszWhat("abc::exception"),
   m_pszSourceFunction(nullptr),
   m_bInFlight(false) {
}
exception::exception(exception const & x) :
   m_pszWhat(x.m_pszWhat),
   m_pszSourceFunction(x.m_pszSourceFunction),
   m_srcloc(x.m_srcloc),
   m_bInFlight(x.m_bInFlight) {
   // See [DOC:8503 Stack tracing].
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_addref();
   }
}

/*virtual*/ exception::~exception() {
   // See [DOC:8503 Stack tracing].
   if (m_bInFlight) {
      detail::scope_trace::trace_writer_release();
   }
}

exception & exception::operator=(exception const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   m_pszWhat = x.m_pszWhat;
   m_pszSourceFunction = x.m_pszSourceFunction;
   m_srcloc = x.m_srcloc;
   /* Adopt the source’s in-flight status. See [DOC:8503 Stack tracing]. If the in-flight status is
   not changing, avoid the pointless (and dangerous, if done in this sequence – it could delete the
   trace writer if *this was the last reference to it) release()/addref(). */
   if (m_bInFlight != x.m_bInFlight) {
      if (m_bInFlight) {
         detail::scope_trace::trace_writer_release();
      }
      m_bInFlight = x.m_bInFlight;
      if (m_bInFlight) {
         detail::scope_trace::trace_writer_addref();
      }
   }
   return *this;
}

void exception::_before_throw(source_location const & srcloc, char_t const * pszFunction) {
   m_pszSourceFunction = pszFunction;
   m_srcloc = srcloc;
   /* Clear any old trace writer buffer and create a new one with *this as its only reference. See
   [DOC:8503 Stack tracing]. */
   detail::scope_trace::trace_writer_clear();
   detail::scope_trace::trace_writer_addref();
   m_bInFlight = true;
}

char const * exception::what() const {
   return m_pszWhat;
}

/*virtual*/ void exception::write_extended_info(io::text::writer * ptwOut) const {
   ABC_UNUSED_ARG(ptwOut);
}

/*static*/ void exception::write_with_scope_trace(
   io::text::writer * ptwOut /*= nullptr*/, std::exception const * pstdx /*= nullptr*/
) {
   if (!ptwOut) {
      ptwOut = io::text::stderr().get();
   }
   exception const * pabcx;
   if (pstdx) {
      // We have an std::exception: print its what() and check if it’s also an abc::exception.
      ptwOut->print(ABC_SL("Exception: {}\n"), char_ptr_to_str_adapter(pstdx->what()));
      pabcx = dynamic_cast<exception const *>(pstdx);
      if (pabcx) {
         try {
            ptwOut->write(ABC_SL("Extended information:"));
            pabcx->write_extended_info(ptwOut);
            ptwOut->write_line();
         } catch (...) {
            /* The exception is not rethrown because we don’t want exception details to interfere
            with the display of the (more important) exception information. */
            // FIXME: EXC-SWALLOW
         }
      }
   } else {
      pabcx = nullptr;
   }

   ptwOut->write(ABC_SL("Stack trace (most recent call first):\n"));
   if (pabcx) {
      // Frame 0 is the location of the ABC_THROW() statement.
      ptwOut->print(
         ABC_SL("#0 {} at {}\n"), istr(external_buffer, pabcx->m_pszSourceFunction), pabcx->m_srcloc
      );
   }
   // Print the scope/stack trace collected via ABC_TRACE_FUNC().
   ptwOut->write(detail::scope_trace::get_trace_writer()->get_str());
   // Append any scope_trace instances that haven’t been destructed yet.
   detail::scope_trace::write_list(ptwOut);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error

namespace abc {

argument_error::argument_error() :
   generic_error() {
   m_pszWhat = "abc::argument_error";
}

void argument_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<argument_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error

namespace abc {

arithmetic_error::arithmetic_error() :
   generic_error() {
   m_pszWhat = "abc::arithmetic_error";
}

void arithmetic_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<arithmetic_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::assertion_error

namespace abc {

coroutine_local_value<bool> assertion_error::sm_bReentering /*= false*/;

/*static*/ void assertion_error::_assertion_failed(
   source_location const & srcloc, istr const & sFunction, istr const & sExpr, istr const & sMsg
) {
   if (!sm_bReentering) {
      sm_bReentering = true;
      try {
         io::text::stderr()->print(
            ABC_SL("Assertion failed: {} ( {} ) in file {}: in function {}\n"),
            sMsg, sExpr, srcloc, sFunction
         );
      } catch (...) {
         sm_bReentering = false;
         throw;
      }
      sm_bReentering = false;
   }
   ABC_THROW(assertion_error, ());
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::buffer_error

namespace abc {

buffer_error::buffer_error() :
   generic_error() {
   m_pszWhat = "abc::buffer_error";
}

void buffer_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<buffer_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error

namespace abc {

division_by_zero_error::division_by_zero_error() :
   arithmetic_error() {
   m_pszWhat = "abc::division_by_zero_error";
}

void division_by_zero_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err ? err : os_error_mapping<division_by_zero_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error

namespace abc {

domain_error::domain_error() :
   generic_error() {
   m_pszWhat = "abc::domain_error";
}

void domain_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<domain_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::environment_error

namespace abc {

environment_error::environment_error() :
   generic_error() {
   m_pszWhat = "abc::environment_error";
}

void environment_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<environment_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error

namespace abc {

floating_point_error::floating_point_error() :
   arithmetic_error() {
   m_pszWhat = "abc::floating_point_error";
}

void floating_point_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err ? err : os_error_mapping<floating_point_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error

namespace abc {

generic_error::generic_error() :
   exception() {
   m_pszWhat = "abc::generic_error";
}
generic_error::generic_error(generic_error const & x) :
   exception(x),
   m_err(x.m_err) {
}

generic_error & generic_error::operator=(generic_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   exception::operator=(x);
   m_err = x.m_err;
   return *this;
}

/*virtual*/ void generic_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   exception::write_extended_info(ptwOut);
   if (m_err) {
      ptwOut->print(ABC_SL(" OS error={}"), m_err);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error

namespace abc {

index_error::index_error() :
   lookup_error() {
   m_pszWhat = "abc::index_error";
}
index_error::index_error(index_error const & x) :
   generic_error(x),
   lookup_error(x),
   m_iInvalid(x.m_iInvalid) {
}

index_error & index_error::operator=(index_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   lookup_error::operator=(x);
   m_iInvalid = x.m_iInvalid;
   return *this;
}

void index_error::init(std::ptrdiff_t iInvalid, errint_t err /*= 0*/) {
   lookup_error::init(err ? err : os_error_mapping<index_error>::mapped_error);
   m_iInvalid = iInvalid;
}

/*virtual*/ void index_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   lookup_error::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL(" invalid index: {}"), m_iInvalid);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::invalid_path_error

namespace abc {

invalid_path_error::invalid_path_error() :
   generic_error() {
   m_pszWhat = "abc::invalid_path_error";
}

void invalid_path_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<invalid_path_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io_error

namespace abc {

io_error::io_error() :
   environment_error() {
   m_pszWhat = "abc::io_error";
}

void io_error::init(errint_t err /*= 0*/) {
   environment_error::init(err ? err : os_error_mapping<io_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::iterator_error

namespace abc {

iterator_error::iterator_error() :
   generic_error() {
   m_pszWhat = "abc::iterator_error";
}

void iterator_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<iterator_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::key_error

namespace abc {

key_error::key_error() :
   lookup_error() {
   m_pszWhat = "abc::key_error";
}

void key_error::init(errint_t err /*= 0*/) {
   lookup_error::init(err ? err : os_error_mapping<key_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error

namespace abc {

lookup_error::lookup_error() :
   generic_error() {
   m_pszWhat = "abc::lookup_error";
}

void lookup_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<lookup_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error

namespace abc {

memory_access_error::memory_access_error() :
   memory_address_error() {
   m_pszWhat = "abc::memory_access_error";
}

void memory_access_error::init(void const * pInvalid, errint_t err /*= 0*/) {
   memory_address_error::init(
      pInvalid, err ? err : os_error_mapping<memory_access_error>::mapped_error
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error

namespace abc {

char_t const memory_address_error::smc_achUnknownAddress[] = ABC_SL(" unknown memory address");

memory_address_error::memory_address_error() :
   generic_error() {
   m_pszWhat = "abc::memory_address_error";
}
memory_address_error::memory_address_error(memory_address_error const & x) :
   generic_error(x),
   m_pInvalid(x.m_pInvalid) {
}

memory_address_error & memory_address_error::operator=(memory_address_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   generic_error::operator=(x);
   m_pInvalid = x.m_pInvalid;
   return *this;
}

void memory_address_error::init(void const * pInvalid, errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<memory_address_error>::mapped_error);
   m_pInvalid = pInvalid;
}

/*virtual*/ void memory_address_error::write_extended_info(
   io::text::writer * ptwOut
) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   if (m_pInvalid != smc_achUnknownAddress) {
      ptwOut->print(ABC_SL(" invalid address: {}"), m_pInvalid);
   } else {
      ptwOut->write(smc_achUnknownAddress);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error

namespace abc {

memory_allocation_error::memory_allocation_error() :
   generic_error() {
   m_pszWhat = "abc::memory_allocation_error";
}

void memory_allocation_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<memory_allocation_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_error

namespace abc {

network_error::network_error() :
   environment_error() {
   m_pszWhat = "abc::network_error";
}

void network_error::init(errint_t err /*= 0*/) {
   environment_error::init(err ? err : os_error_mapping<network_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_io_error

namespace abc {

network_io_error::network_io_error() :
   io_error(),
   network_error() {
   m_pszWhat = "abc::network_io_error";
}

void network_io_error::init(errint_t err /*= 0*/) {
   if (!err) {
      err = os_error_mapping<network_io_error>::mapped_error;
   }
   io_error::init(err);
   network_error::init(err);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::not_implemented_error

namespace abc {

not_implemented_error::not_implemented_error() :
   generic_error() {
   m_pszWhat = "abc::not_implemented_error";
}

void not_implemented_error::init(errint_t err /*= 0*/) {
   generic_error::init(err ? err : os_error_mapping<not_implemented_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error

namespace abc {

null_pointer_error::null_pointer_error() :
   memory_address_error() {
   m_pszWhat = "abc::null_pointer_error";
}

void null_pointer_error::init(errint_t err /*= 0*/) {
   memory_address_error::init(
      nullptr, err ? err : os_error_mapping<null_pointer_error>::mapped_error
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error

namespace abc {

overflow_error::overflow_error() :
   arithmetic_error() {
   m_pszWhat = "abc::overflow_error";
}

void overflow_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err ? err : os_error_mapping<overflow_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::pointer_iterator_error

namespace abc {

pointer_iterator_error::pointer_iterator_error() :
   iterator_error() {
   m_pszWhat = "abc::pointer_iterator_error";
}
pointer_iterator_error::pointer_iterator_error(pointer_iterator_error const & x) :
   generic_error(x),
   iterator_error(x),
   m_pContBegin(x.m_pContBegin),
   m_pContEnd(x.m_pContEnd),
   m_pInvalid(x.m_pInvalid) {
}

pointer_iterator_error & pointer_iterator_error::operator=(pointer_iterator_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   iterator_error::operator=(x);
   m_pContBegin = x.m_pContBegin;
   m_pContEnd = x.m_pContEnd;
   m_pInvalid = x.m_pInvalid;
   return *this;
}

void pointer_iterator_error::init(
   void const * pContBegin, void const * pContEnd, void const * pInvalid, errint_t err /*= 0*/
) {
   iterator_error::init(err ? err : os_error_mapping<pointer_iterator_error>::mapped_error);
   m_pContBegin = pContBegin;
   m_pContEnd = pContEnd;
   m_pInvalid = pInvalid;
}

/*virtual*/ void pointer_iterator_error::write_extended_info(
   io::text::writer * ptwOut
) const /*override*/ {
   iterator_error::write_extended_info(ptwOut);
   ptwOut->print(
      ABC_SL(" invalid iterator: {} (container begin/end range: [{}, {}])"),
      m_pInvalid, m_pContBegin, m_pContEnd
   );
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::security_error

namespace abc {

security_error::security_error() :
   environment_error() {
   m_pszWhat = "abc::security_error";
}

void security_error::init(errint_t err /*= 0*/) {
   environment_error::init(err ? err : os_error_mapping<security_error>::mapped_error);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error

namespace abc {

syntax_error::syntax_error() :
   generic_error() {
   m_pszWhat = "abc::syntax_error";
}
syntax_error::syntax_error(syntax_error const & x) :
   generic_error(x),
   m_sDescription(x.m_sDescription),
   m_sSource(x.m_sSource),
   m_iChar(x.m_iChar),
   m_iLine(x.m_iLine) {
}

syntax_error & syntax_error::operator=(syntax_error const & x) {
   ABC_TRACE_FUNC(this/*, x*/);

   generic_error::operator=(x);
   m_sDescription = x.m_sDescription;
   m_sSource = x.m_sSource;
   m_iChar = x.m_iChar;
   m_iLine = x.m_iLine;
   return *this;
}

void syntax_error::init(
   istr const & sDescription /*= istr::empty*/, istr const & sSource /*= istr::empty*/,
   unsigned iChar /*= 0*/, unsigned iLine /*= 0*/, errint_t err /*= 0*/
) {
   generic_error::init(err ? err : os_error_mapping<syntax_error>::mapped_error);
   m_sDescription = sDescription;
   m_sSource = sSource;
   m_iChar = iChar;
   m_iLine = iLine;
}

/*virtual*/ void syntax_error::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   istr sFormat;
   if (m_sSource) {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in {1}:{2}:{3}");
         } else {
            sFormat = ABC_SL(" {0} in expression \"{1}\", character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in {1}:{2}");
         } else {
            sFormat = ABC_SL(" {0} in expression \"{1}\"");
         }
      }
   } else {
      if (m_iChar) {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in <input>:{2}:{3}");
         } else {
            sFormat = ABC_SL(" {0} in <expression>, character {3}");
         }
      } else {
         if (m_iLine) {
            sFormat = ABC_SL(" {0} in <input>:{2}");
         } else {
            sFormat = ABC_SL(" {0}");
         }
      }
   }

   ptwOut->print(sFormat, m_sDescription, m_sSource, m_iLine, m_iChar);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

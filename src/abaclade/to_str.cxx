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
#include <abaclade/math.hxx>
#include <abaclade/numeric.hxx>
#include <abaclade/text/char_ptr_to_str_adapter.hxx>

#include <algorithm>
#include <climits> // CHAR_BIT
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   #include <cxxabi.h> // abi::__cxa_demangle()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

void to_str_backend<bool>::set_format(str const & sFormat) {
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

void to_str_backend<bool>::write(bool b, io::text::ostream * ptos) {
   ABC_TRACE_FUNC(this/*, b*/, ptos);

   if (b) {
      ptos->write(ABC_SL("true"));
   } else {
      ptos->write(ABC_SL("false"));
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

char const int_to_str_backend_base::smc_achIntToStrU[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};
char const int_to_str_backend_base::smc_achIntToStrL[16] = {
   '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

int_to_str_backend_base::int_to_str_backend_base(unsigned cbInt) :
   m_pchIntToStr(smc_achIntToStrL),
   // Default to generating at least a single zero.
   m_cchWidth(1),
   m_cchBuf(1 /*possible sign*/ + 3 /*max base10 characters per byte*/ * cbInt),
   mc_cbInt(static_cast<std::uint8_t>(cbInt)),
   // Default to decimal notation.
   m_iBaseOrShift(10),
   // Default padding is with spaces (and won’t be applied by default).
   m_chPad(' '),
   // A sign will only be displayed if the number is negative and no prefix is applied.
   m_chSign('\0'),
   m_chPrefix0('\0'),
   m_chPrefix1('\0') {
}

void int_to_str_backend_base::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   bool bPrefix = false;
   auto it(sFormat.cbegin());
   char32_t ch;
   if (it == sFormat.cend()) {
      goto default_notation;
   }
   ch = *it++;
   // Display a plus or a space in front of non-negative numbers.
   if (ch == '+' || ch == ' ') {
      // Force this character to be displayed for non-negative numbers.
      m_chSign = static_cast<char>(ch);
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // Prefix with 0b, 0B, 0, 0x or 0X.
   if (ch == '#') {
      bPrefix = true;
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // Pad with zeros instead of spaces.
   if (ch == '0') {
      m_chPad = '0';
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // “Width” - minimum number of digits.
   if (ch >= '1' && ch <= '9') {
      /* Undo the default; the following loop will yield at least 1 anyway (because we don’t get
      here for a 0 – see if above). */
      m_cchWidth = 0;
      do {
         m_cchWidth = m_cchWidth * 10 + static_cast<unsigned>(ch - '0');
         if (it == sFormat.cend()) {
            goto default_notation;
         }
         ch = *it++;
      } while (ch >= '0' && ch <= '9');
   }

   /* We jump in this impossible if to set the default notation when we run out of characters in any
   of the above blocks. If we do get here without jumping, the last character retrieved and stored
   in is the requested notation. */
   if (false) {
default_notation:
      ch = 'd';
   }

   /* Determine which notation to use, which will also yield the approximate number of characters
   per byte. */
   unsigned cchByte;
   switch (ch) {
      case 'b':
      case 'B':
      case 'o':
      case 'x':
      case 'X':
         if (bPrefix) {
            m_chPrefix0 = '0';
         }
         // Fall through.
      case 'd':
         switch (ch) {
            case 'b': // Binary notation, lowercase prefix.
            case 'B': // Binary notation, uppercase prefix.
               m_chPrefix1 = static_cast<char>(ch);
               m_iBaseOrShift = 1;
               cchByte = 8;
               break;
            case 'o': // Octal notation.
               m_iBaseOrShift = 3;
               cchByte = 3;
               break;
            case 'X': // Hexadecimal notation, uppercase prefix and letters.
               m_pchIntToStr = smc_achIntToStrU;
               // Fall through.
            case 'x': // Hexadecimal notation, lowercase prefix and letters.
               m_chPrefix1 = static_cast<char>(ch);
               m_iBaseOrShift = 4;
               cchByte = 2;
               break;
            case 'd': // Decimal notation.
               m_iBaseOrShift = 10;
               cchByte = 3;
               break;
         }
         if (it == sFormat.cend()) {
            break;
         }
         // If we still have any characters, they are garbage (fall through).
      default:
         ABC_THROW(syntax_error, (
            ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
         ));
   }

   // Now we know enough to calculate the required buffer size.
   m_cchBuf = 2 /*prefix or sign*/ + std::max(m_cchWidth, cchByte * mc_cbInt);
}

void int_to_str_backend_base::add_prefixes_and_write(
   bool bNegative, io::text::ostream * ptos, str * psBuf, str::iterator itBufFirstUsed
) const {
   ABC_TRACE_FUNC(this, bNegative, ptos, psBuf, itBufFirstUsed);

   auto itEnd(psBuf->cend());
   auto it(itBufFirstUsed);
   // Ensure that at least one digit is generated.
   if (it == itEnd) {
      *--it = '0';
   }
   /* Determine the sign character: only if in decimal notation, and make it a minus sign if the
   number is negative. */
   char chSign = m_iBaseOrShift == 10 ? bNegative ? '-' : m_chSign : '\0';
   // Decide whether we’ll put a sign last, after the padding.
   bool bSignLast = chSign && m_chPad == '0';
   // Add the sign character if there’s no prefix and the padding is not zeros.
   if (chSign && m_chPad != '0') {
      *--it = chSign;
   }
   // Ensure that at least m_cchWidth characters are generated (but reserve a space for the sign).
   auto itFirstDigit(itEnd - static_cast<std::ptrdiff_t>(m_cchWidth - (bSignLast ? 1 : 0)));
   while (it > itFirstDigit) {
      *--it = m_chPad;
   }
   // Add prefix or sign (if padding with zeros), if any.
   if (m_chPrefix0) {
      if (m_chPrefix1) {
         *--it = m_chPrefix1;
      }
      *--it = m_chPrefix0;
   } else if (bSignLast) {
      // Add the sign character.
      *--it = chSign;
   }
   // Write the constructed string.
   ptos->write_binary(
      it.ptr(), sizeof(char_t) * static_cast<std::size_t>(itEnd - it), text::encoding::host
   );
}

template <typename I>
inline void int_to_str_backend_base::write_impl(I i, io::text::ostream * ptos) const {
   ABC_TRACE_FUNC(this/*, i*/, ptos);

   // Create a buffer of sufficient size for binary notation (the largest).
   sstr<2 /* prefix or sign */ + sizeof(I) * CHAR_BIT> sBuf;
   /* Use bClear = true since we need to iterate backwards on sBuf, which requires reading its
   otherwise uninitialized charactes. */
   sBuf.set_size_in_chars(m_cchBuf, true);
   auto it(sBuf.end());

   // Generate the digits.
   I iRest(i);
   if (m_iBaseOrShift == 10) {
      // Base 10: must use % and /.
      I iDivider = static_cast<I>(m_iBaseOrShift);
      while (iRest) {
         I iMod(iRest % iDivider);
         iRest /= iDivider;
         *--it = m_pchIntToStr[math::abs(iMod)];
      }
   } else {
      // Base 2 ^ n: can use & and >>.
      I iMask = (I(1) << m_iBaseOrShift) - 1;
      while (iRest) {
         *--it = m_pchIntToStr[iRest & iMask];
         iRest >>= m_iBaseOrShift;
      }
   }

   // Add prefix or sign, and output to the stream.
   add_prefixes_and_write(numeric::is_negative<I>(i), ptos, sBuf.str_ptr(), it);
}

void int_to_str_backend_base::write_s64(std::int64_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}

void int_to_str_backend_base::write_u64(std::uint64_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}

#if ABC_HOST_WORD_SIZE < 64
void int_to_str_backend_base::write_s32(std::int32_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}

void int_to_str_backend_base::write_u32(std::uint32_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}

#if ABC_HOST_WORD_SIZE < 32
void int_to_str_backend_base::write_s16(std::int16_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}

void int_to_str_backend_base::write_u16(std::uint16_t i, io::text::ostream * ptos) const {
   write_impl(i, ptos);
}
#endif //if ABC_HOST_WORD_SIZE < 32
#endif //if ABC_HOST_WORD_SIZE < 64

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

void ptr_to_str_backend::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   to_str_backend<std::uintptr_t>::set_format(ABC_SL("#x"));

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void ptr_to_str_backend::_write_impl(std::uintptr_t iPtr, io::text::ostream * ptos) {
   ABC_TRACE_FUNC(this/*, iPtr*/, ptos);

   if (iPtr) {
      to_str_backend<std::uintptr_t>::write(iPtr, ptos);
   } else {
      ptos->write(ABC_SL("nullptr"));
   }
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

to_str_backend<_std::type_info>::to_str_backend() {
}

to_str_backend<_std::type_info>::~to_str_backend() {
}

void to_str_backend<_std::type_info>::set_format(str const & sFormat) {
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

void to_str_backend<_std::type_info>::write(_std::type_info const & ti, io::text::ostream * ptos) {
   char const * psz = ti.name();
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   // Clang and G++ generate mangled names.
   int iRet = 0;
   _std::unique_ptr<char const, memory::freeing_deleter> pszDemangled(
      abi::__cxa_demangle(psz, nullptr, nullptr, &iRet)
   );
   if (iRet >= 0 && pszDemangled) {
      psz = pszDemangled.get();
   } else {
      psz = "?";
   }
#elif ABC_HOST_CXX_MSC
   /* MSC prepends “class ”, “struct ” or “union ” to the type name; find the first space and strip
   anything preceding it. */
   for (char const * pszSpace = psz; *pszSpace; ++pszSpace) {
      if (*pszSpace == ' ') {
         psz = pszSpace + 1;
         break;
      }
   }
#endif
   to_str_backend<text::char_ptr_to_str_adapter> tsbCStr;
   tsbCStr.write(text::char_ptr_to_str_adapter(psz), ptos);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

sequence_to_str_backend::sequence_to_str_backend(str const & sStart, str const & sEnd) :
   m_sSeparator(ABC_SL(", ")),
   m_sStart(sStart),
   m_sEnd(sEnd) {
}

void sequence_to_str_backend::set_format(str const & sFormat) {
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

sequence_to_str_backend::~sequence_to_str_backend() {
}

void sequence_to_str_backend::_write_end(io::text::ostream * ptos) {
   ptos->write(m_sEnd);
}

void sequence_to_str_backend::_write_separator(io::text::ostream * ptos) {
   ptos->write(m_sSeparator);
}

void sequence_to_str_backend::_write_start(io::text::ostream * ptos) {
   ptos->write(m_sStart);
}

}} //namespace abc::detail

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>
#include <abc/iostream.hxx>
#include <abc/math.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for bool


namespace abc {

ABCAPI to_str_backend<bool>::to_str_backend(istr const & sFormat /*= istr()*/) {
   ABC_TRACE_FN((this, sFormat));

   auto it(sFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), sFormat, unsigned(it - sFormat.cbegin())
      ));
   }
}


ABCAPI void to_str_backend<bool>::write(bool b, io::ostream * posOut) {
   ABC_TRACE_FN((this, b, posOut));

   // TODO: apply format options.
   if (b) {
      posOut->write(SL("true"));
   } else {
      posOut->write(SL("false"));
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend_base


namespace abc {

char_t const _int_to_str_backend_base::smc_achIntToStrU[16] = {
   CL('0'), CL('1'), CL('2'), CL('3'), CL('4'), CL('5'), CL('6'), CL('7'), CL('8'), CL('9'),
   CL('A'), CL('B'), CL('C'), CL('D'), CL('E'), CL('F')
};
char_t const _int_to_str_backend_base::smc_achIntToStrL[16] = {
   CL('0'), CL('1'), CL('2'), CL('3'), CL('4'), CL('5'), CL('6'), CL('7'), CL('8'), CL('9'),
   CL('a'), CL('b'), CL('c'), CL('d'), CL('e'), CL('f')
};


ABCAPI _int_to_str_backend_base::_int_to_str_backend_base(
   unsigned cbInt, istr const & sFormat
) :
   m_pchIntToStr(smc_achIntToStrL),
   m_iBaseOrShift(10),
   // Default to generating at least a single zero.
   m_cchWidth(1),
   m_chPad(CL(' ')),
   // A sign will only be displayed if the number is negative and no prefix is applied.
   m_chSign(CL('\0')),
   m_chPrefix0(CL('\0')),
   m_chPrefix1(CL('\0')) {
   ABC_TRACE_FN((this, cbInt, sFormat));

   bool bPrefix(false);
   auto it(sFormat.cbegin());
   char_t ch;
   if (it == sFormat.cend()) {
      goto default_notation;
   }
   ch = *it++;
   // Display a plus or a space in front of non-negative numbers.
   if (ch == CL('+') || ch == CL(' ')) {
      // Force this character to be displayed for non-negative numbers.
      m_chSign = ch;
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // Prefix with 0b, 0B, 0, 0x or 0X.
   if (ch == CL('#')) {
      bPrefix = true;
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // Pad with zeroes instead of spaces.
   if (ch == CL('0')) {
      m_chPad = CL('0');
      if (it == sFormat.cend()) {
         goto default_notation;
      }
      ch = *it++;
   }
   // “Width” - minimum number of digits.
   if (ch >= CL('1') && ch <= CL('9')) {
      // Undo the default; the following loop will yield at least 1 anyway (because we don’t get
      // here for a 0 - see if above).
      m_cchWidth = 0;
      do {
         m_cchWidth = m_cchWidth * 10 + unsigned(ch) - CL('0');
         if (it == sFormat.cend()) {
            goto default_notation;
         }
         ch = *it++;
      } while (ch >= CL('0') && ch <= CL('9'));
   }

   // We jump in this impossible if to set the default notation when we run out of characters in any
   // of the above blocks. If we do get here without jumping, the last character retrieved and
   // stored in is the requested notation.
   if (false) {
default_notation:
      ch = CL('d');
   }

   // Determine which notation to use, which will also yield the approximate number of characters
   // per byte.
   unsigned cchByte;
   switch (ch) {
      case CL('b'):
      case CL('B'):
      case CL('o'):
      case CL('x'):
      case CL('X'):
         if (bPrefix) {
            m_chPrefix0 = CL('0');
         }
         // Fall through.
      case CL('d'):
         switch (ch) {
            case CL('b'): // Binary notation, lowercase prefix.
            case CL('B'): // Binary notation, uppercase prefix.
               m_chPrefix1 = ch;
               m_iBaseOrShift = 1;
               cchByte = 8;
               break;
            case CL('o'): // Octal notation.
               m_iBaseOrShift = 3;
               cchByte = 3;
               break;
            case CL('X'): // Hexadecimal notation, uppercase prefix and letters.
               m_pchIntToStr = smc_achIntToStrU;
               // Fall through.
            case CL('x'): // Hexadecimal notation, lowercase prefix and letters.
               m_chPrefix1 = ch;
               m_iBaseOrShift = 4;
               cchByte = 2;
               break;
            case CL('d'): // Decimal notation.
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
            SL("unexpected character"), sFormat, unsigned(it - sFormat.cbegin())
         ));
   }

   // Now we know enough to calculate the required buffer size.
   m_cchBuf = 2 /*prefix or sign*/ + std::max(m_cchWidth, cchByte * cbInt);
}


ABCAPI void _int_to_str_backend_base::add_prefixes_and_write(
   bool bNegative, io::ostream * posOut, mstr * psBuf, mstr::iterator itBufFirstUsed
) const {
   ABC_TRACE_FN((this, bNegative, posOut, psBuf, itBufFirstUsed));

   auto itEnd(psBuf->cend());
   auto it(itBufFirstUsed);
   // Ensure that at least one digit is generated.
   if (it == itEnd) {
      *--it = CL('0');
   }
   // Determine the sign character: only if in decimal notation, and make it a minus sign if the
   // number is negative.
   char_t chSign(m_iBaseOrShift == 10 ? bNegative ? CL('-') : m_chSign : CL('\0'));
   // Decide whether we’ll put a sign last, after the padding.
   bool bSignLast(chSign && m_chPad == CL('0'));
   // Add the sign character if there’s no prefix and the padding is not zeroes.
   if (chSign && m_chPad != CL('0')) {
      *--it = chSign;
   }
   // Ensure that at least m_cchWidth characters are generated (but reserve a space for the sign).
   auto itFirstDigit(itEnd - (m_cchWidth - (bSignLast ? 1 : 0)));
   while (it > itFirstDigit) {
      *--it = m_chPad;
   }
   // Add prefix or sign (if padding with zeroes), if any.
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
   posOut->write_raw(it.base(), sizeof(char_t) * size_t(itEnd - it), text::encoding::host);
}


template <typename I>
inline void _int_to_str_backend_base::write_impl(I i, io::ostream * posOut) const {
   ABC_TRACE_FN((this, i, posOut));

   // Create a buffer of sufficient size for binary notation (the largest).
   smstr<2 /* prefix or sign */ + sizeof(I) * CHAR_BIT> sBuf;
   sBuf.set_size(m_cchBuf);
   auto it(sBuf.end());

   // Generate the digits.
   I iRest(i);
   if (m_iBaseOrShift == 10) {
      // Base 10: must use % and /.
      I iDivider((I(m_iBaseOrShift)));
      while (iRest) {
         I iMod(iRest % iDivider);
         iRest /= iDivider;
         *--it = m_pchIntToStr[math::abs(iMod)];
      }
   } else {
      // Base 2 ^ n: can use & and >>.
      I iMask((I(1) << m_iBaseOrShift) - 1);
      while (iRest) {
         *--it = m_pchIntToStr[iRest & iMask];
         iRest >>= m_iBaseOrShift;
      }
   }

   // Add prefix or sign, and write to the ostream.
   add_prefixes_and_write(numeric::is_negative<I>(i), posOut, &sBuf, it);
}


ABCAPI void _int_to_str_backend_base::write_s64(int64_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u64(uint64_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}


#if ABC_HOST_WORD_SIZE < 64

ABCAPI void _int_to_str_backend_base::write_s32(int32_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u32(uint32_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}


#if ABC_HOST_WORD_SIZE < 32

ABCAPI void _int_to_str_backend_base::write_s16(int16_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}


ABCAPI void _int_to_str_backend_base::write_u16(uint16_t i, io::ostream * posOut) const {
   write_impl(i, posOut);
}

#endif //if ABC_HOST_WORD_SIZE < 32
#endif //if ABC_HOST_WORD_SIZE < 64


} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend<void *>


namespace abc {

char_t const to_str_backend<void *>::smc_achFormat[] = SL("#x");


ABCAPI to_str_backend<void *>::to_str_backend(istr const & sFormat /*= istr()*/) :
   to_str_backend<uintptr_t>(smc_achFormat) {
   ABC_TRACE_FN((this, sFormat));

   auto it(sFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), sFormat, unsigned(it - sFormat.cbegin())
      ));
   }
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


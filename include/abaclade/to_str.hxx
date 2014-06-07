/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str()

namespace abc {

/** DOC:3984 abc::to_str() and abc::to_str_backend()

abc::to_str() is a more advanced counterpart to std::to_string() (see C++11 § 21.5 “Numeric
conversions”); here are the main differences when compared to the STL function:

•  It accepts an additional argument, controlling how the conversion to string is to be done;

•  Its default specialization relies on abc::to_str_backend(), which outputs its result to an
   abc::io::text::writer instance; this means that the complete specialization is shared with
   abc::io::text::writer::print() (see [DOC:7103 abc::text::writer::print()]);

•  Since the default implementation of abc::to_str() is a thin wrapper around abc::to_str_backend,
   implementors can provide a partial specialization for it (partial specializations of function are
   still not allowed in C++11), allowing to share parts of the implementation among convertible
   classes.

The format specification is provided to a to_str_backend specialization by passing it an abc::istr
const &, so a caller can specify a non-NUL-terminated substring of a larger string without the need
for temporary strings. Once a to_str_backend instance has been constructed, it must be able to
sequentially process an infinite number of conversions, i.e. instances of a to_str_backend
specialization must be reusable.

The interpretation of the format specification is up to the specialization of abc::to_str_backend.

When code causes the compiler to instantiate a specialization of abc::to_str_backend that has not
been defined, GCC will generate an error:

   error: ‘abc::to_str_backend<my_type> …’ has incomplete type

The only fix for this is to provide an explicit specialization of abc::to_str_backend for my_type.
*/

/** Returns the string representation of the specified value, optionally with a custom format.

t
   Object to generate a string representation for.
sFormat
   Type-specific format string.
return
   String representation of t according to sFormat.
*/
template <typename T>
dmstr to_str(T const & t, istr const & sFormat = istr());

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend

namespace abc {

/** Generates a string suitable for display from an object. Once constructed with the desired format
specification, an instance can convert to a string any number of T instances. */
template <typename T>
class to_str_backend;

// Partial specialization for const.
template <typename T>
class to_str_backend<T const> :
   public to_str_backend<T> {
public:

   /** Constructor. See to_str_backend<T>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<T>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for bool


namespace abc {

template <>
class ABACLADE_SYM to_str_backend<bool> {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr());


   /** Converts a boolean value to its string representation.

   b
      Boolean value to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(bool b, io::text::writer * ptwOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend_base


namespace abc {

/** Base class for the specializations of to_str_backend for integer types.
*/
class ABACLADE_SYM _int_to_str_backend_base {
public:

   /** Constructor.

   cbInt
      Size of the integer type.
   sFormat
      Formatting options.
   */
   _int_to_str_backend_base(unsigned cbInt, istr const & sFormat);


protected:

   /** Writes the provided buffer to *posOut, prefixed as necessary.

   bNegative
      true if the number is negative, or false otherwise.
   ptwOut
      Pointer to the writer to output to.
   psBuf
      Pointer to the string containing the characters to write.
   itBufFirstUsed
      Iterator to the first used character in *psBuf; the last used character is always the last
      character in *psBuf.
   */
   void add_prefixes_and_write(
      bool bNegative, io::text::writer * ptwOut, mstr * psBuf, mstr::iterator itBufFirstUsed
   ) const;


   /** Converts an integer to its string representation.

   i
      Integer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   template <typename I>
   void write_impl(I i, io::text::writer * ptwOut) const;


   /** Converts a 64-bit signed integer to its string representation. See write_impl().
   */
   void write_s64(int64_t i, io::text::writer * ptwOut) const;


   /** Converts a 64-bit unsigned integer to its string representation. See write_impl().
   */
   void write_u64(uint64_t i, io::text::writer * ptwOut) const;


   /** Converts a 32-bit signed integer to its string representation. See write_impl().
   */
   void write_s32(int32_t i, io::text::writer * ptwOut) const;


   /** Converts a 32-bit unsigned integer to its string representation. See write_impl().
   */
   void write_u32(uint32_t i, io::text::writer * ptwOut) const;


   /** Converts a 16-bit signed integer to its string representation. See write_impl().
   */
   void write_s16(int16_t i, io::text::writer * ptwOut) const;


   /** Converts a 16-bit unsigned integer to its string representation. See write_impl().
   */
   void write_u16(uint16_t i, io::text::writer * ptwOut) const;


   /** Converts an 8-bit signed integer to its string representation. See write_impl().
   */
   void write_s8(int8_t i, io::text::writer * ptwOut) const {
      if (m_iBaseOrShift == 10) {
         write_s16(i, ptwOut);
      } else {
         // Avoid extending the sign, as it would generate too many digits in any notation except
         // decimal.
         write_s16(uint8_t(i), ptwOut);
      }
   }


   /** Converts an 8-bit unsigned integer to its string representation. See write_impl().
   */
   void write_u8(uint8_t i, io::text::writer * ptwOut) const {
      write_u16(i, ptwOut);
   }


protected:

   /** Pointer to either smc_achIntToStrL or smc_achIntToStrU. */
   char_t const * m_pchIntToStr;
   /** 10 (for decimal notation) or log2(notation) (for power-of-two notations). */
   unsigned m_iBaseOrShift;
   /** Minimum number of digits to be generated. Always >= 1, to ensure the generation of at least a
   single zero. */
   unsigned m_cchWidth;
   /** Required buffer size. */
   unsigned m_cchBuf;
   /** Character to be used to pad the digits to m_cchWidth length. */
   char_t m_chPad;
   /** Character to be used as sign in case the number is not negative; NUL if none. */
   char_t m_chSign;
   /** First character of the prefix; NUL if none (which means that m_chPrefix1 is ignored). */
   char_t m_chPrefix0;
   /** Second character of the prefix; NUL if none. */
   char_t m_chPrefix1;

   /** Map from int [0-15] to its uppercase hexadecimal representation. */
   static char_t const smc_achIntToStrU[16];
   /** Map from int [0-15] to its lowercase hexadecimal representation. */
   static char_t const smc_achIntToStrL[16];
};



#if ABC_HOST_WORD_SIZE >= 32
#if ABC_HOST_WORD_SIZE >= 64

// On a machine with 64-bit word size, write_64*() will be faster.

inline void _int_to_str_backend_base::write_s32(int32_t i, io::text::writer * ptwOut) const {
   if (m_iBaseOrShift == 10) {
      write_s64(i, ptwOut);
   } else {
      // Avoid extending the sign in any notation except decimal, as it would generate too many
      // digits.
      write_s64(uint32_t(i), ptwOut);
   }
}


inline void _int_to_str_backend_base::write_u32(uint32_t i, io::text::writer * ptwOut) const {
   write_u64(i, ptwOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 64


// On a machine with 32-bit word size, write_32*() will be faster. Note that the latter might in
// turn defer to write_64*() (see above).

inline void _int_to_str_backend_base::write_s16(int16_t i, io::text::writer * ptwOut) const {
   if (m_iBaseOrShift == 10) {
      write_s32(i, ptwOut);
   } else {
      // Avoid extending the sign in any notation except decimal, as it would generate too many
      // digits.
      write_s32(uint16_t(i), ptwOut);
   }
}


inline void _int_to_str_backend_base::write_u16(uint16_t i, io::text::writer * ptwOut) const {
   write_u32(i, ptwOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 32

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_int_to_str_backend


namespace abc {

/** Implementation of the specializations of to_str_backend for integer types.
*/
template <typename I>
class _int_to_str_backend :
   public _int_to_str_backend_base {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   _int_to_str_backend(istr const & sFormat) :
      _int_to_str_backend_base(sizeof(I), sFormat) {
   }


   /** Converts an integer to its string representation.

   This design is rather tricky in the way one implementation calls another:

   1. _int_to_str_backend<I>::write()
      Always inlined, dispatches to step 2. based on number of bits;
   2. _int_to_str_backend_base::write_{s,u}{8,16,32,64}()
      Inlined to a bit-bigger variant or implemented in to_str_backend.cxx, depending on the host
      architecture’s word size;
   3. _int_to_str_backend_base::write_impl()
      Always inlined, but only used in functions defined in to_str_backend.cxx, so it only generates
      as many copies as strictly necessary to have fastest performance for any integer size.

   The net result is that after all the inlining occurs, this will become a direct call to the
   fastest implementation for I of any given size.

   i
      Integer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(I i, io::text::writer * ptwOut) {
      if (sizeof(i) <= sizeof(int8_t)) {
         if (std::is_signed<I>::value) {
            write_s8(int8_t(i), ptwOut);
         } else {
            write_u8(uint8_t(i), ptwOut);
         }
      } else if (sizeof(i) <= sizeof(int16_t)) {
         if (std::is_signed<I>::value) {
            write_s16(int16_t(i), ptwOut);
         } else {
            write_u16(uint16_t(i), ptwOut);
         }
      } else if (sizeof(i) <= sizeof(int32_t)) {
         if (std::is_signed<I>::value) {
            write_s32(int32_t(i), ptwOut);
         } else {
            write_u32(uint32_t(i), ptwOut);
         }
      } else {
         static_assert(sizeof(i) <= sizeof(int64_t), "unsupported integer size");
         if (std::is_signed<I>::value) {
            write_s64(int64_t(i), ptwOut);
         } else {
            write_u64(uint64_t(i), ptwOut);
         }
      }
   }


protected:

   /** Initial (static) buffer size sufficient to output the number in binary notation. */
   static size_t const smc_cchBufInitial = 2 /* prefix or sign */ + 8 * sizeof(I);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for integer types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(I) \
   template <> \
   class to_str_backend<I> : \
      public _int_to_str_backend<I> { \
   public: \
   \
      /** Constructor.

      sFormat
         Formatting options.
      */ \
      to_str_backend(istr const & sFormat = istr()) : \
         _int_to_str_backend<I>(sFormat) { \
      } \
   };
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(  signed char)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned char)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         short)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned short)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(     int)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(         long long)
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(unsigned long long)
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


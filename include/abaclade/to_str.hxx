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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Returns the string representation of the specified value, optionally with a custom format.

abc::to_str() is a more advanced alternative to std::to_string() (see C++11 § 21.5 “Numeric
conversions”); here are the main differences when compared to the STL function:

•  It accepts an additional argument, controlling how the conversion to string is to be done;

•  Its default specialization relies on abc::to_str_backend(), which outputs its result to an
   abc::io::text::writer instance; this means that the complete specialization is shared with
   abc::io::text::writer::print();

•  Since the default implementation of abc::to_str() is a thin wrapper around abc::to_str_backend,
   implementors can provide a partial specialization for it (partial specializations of function are
   still not allowed in C++11), allowing to share parts of the implementation among convertible
   classes.

The format specification is provided to an abc::to_str_backend specialization by passing it an
abc::text::str const &, so a caller can specify a non-NUL-terminated substring of a larger string
without the need for temporary strings. Once an abc::to_str_backend instance has been constructed,
it must be able to sequentially process an infinite number of conversions, i.e. instances of a
to_str_backend specialization must be reusable.

The interpretation of the format specification is up to the specialization of abc::to_str_backend.

When code causes the compiler to instantiate a specialization of abc::to_str_backend that has not
been defined, GCC will generate an error:

   @verbatim
   error: ‘abc::to_str_backend<my_type> …’ has incomplete type
   @endverbatim

The only fix for this is to provide an explicit specialization of abc::to_str_backend for my_type.

@param t
   Object to generate a string representation for.
@param sFormat
   Type-specific format string.
@return
   String representation of t according to sFormat.
*/
template <typename T>
str to_str(T const & t, str const & sFormat = str::empty);

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Generates a string suitable for display from an object.

This class template and its specializations are at the core of abc::to_str() and
abc::io::text::writer::print().

Once constructed with the desired format specification, an instance must be able to convert to
string any number of T instances. */
template <typename T>
class to_str_backend;

// Partial specializations for cv-qualified T.
template <typename T>
class to_str_backend<T const> : public to_str_backend<T> {};
template <typename T>
class to_str_backend<T volatile> : public to_str_backend<T> {};
template <typename T>
class to_str_backend<T const volatile> : public to_str_backend<T> {};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<bool> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Converts a boolean value to its string representation.

   @param b
      Boolean value to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(bool b, io::text::writer * ptwOut);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Base class for the specializations of to_str_backend for integer types.
class ABACLADE_SYM int_to_str_backend_base {
public:
   /*! Constructor.

   @param cbInt
      Size of the integer type.
   */
   int_to_str_backend_base(unsigned cbInt);

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

protected:
   /*! Writes the provided buffer to *posOut, prefixed as necessary.

   @param bNegative
      true if the number is negative, or false otherwise.
   @param ptwOut
      Pointer to the writer to output to.
   @param psBuf
      Pointer to the string containing the characters to write.
   @param itBufFirstUsed
      Iterator to the first used character in *psBuf; the last used character is always the last
      character in *psBuf.
   */
   void add_prefixes_and_write(
      bool bNegative, io::text::writer * ptwOut, str * psBuf, str::iterator itBufFirstUsed
   ) const;

   /*! Converts an integer to its string representation.

   @param i
      Integer to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   template <typename I>
   void write_impl(I i, io::text::writer * ptwOut) const;

   //! Converts a 64-bit signed integer to its string representation. See write_impl().
   void write_s64(std::int64_t i, io::text::writer * ptwOut) const;

   //! Converts a 64-bit unsigned integer to its string representation. See write_impl().
   void write_u64(std::uint64_t i, io::text::writer * ptwOut) const;

   //! Converts a 32-bit signed integer to its string representation. See write_impl().
   void write_s32(std::int32_t i, io::text::writer * ptwOut) const;

   //! Converts a 32-bit unsigned integer to its string representation. See write_impl().
   void write_u32(std::uint32_t i, io::text::writer * ptwOut) const;

   //! Converts a 16-bit signed integer to its string representation. See write_impl().
   void write_s16(std::int16_t i, io::text::writer * ptwOut) const;

   //! Converts a 16-bit unsigned integer to its string representation. See write_impl().
   void write_u16(std::uint16_t i, io::text::writer * ptwOut) const;

   //! Converts an 8-bit signed integer to its string representation. See write_impl().
   void write_s8(std::int8_t i, io::text::writer * ptwOut) const {
      if (m_iBaseOrShift == 10) {
         write_s16(i, ptwOut);
      } else {
         /* Avoid extending the sign, as it would generate too many digits in any notation except
         decimal. */
         write_s16(static_cast<std::uint8_t>(i), ptwOut);
      }
   }

   //! Converts an 8-bit unsigned integer to its string representation. See write_impl().
   void write_u8(std::uint8_t i, io::text::writer * ptwOut) const {
      write_u16(i, ptwOut);
   }

protected:
   //! Pointer to either smc_achIntToStrL or smc_achIntToStrU.
   char const * m_pchIntToStr;
   /*! Minimum number of digits to be generated. Always >= 1, to ensure the generation of at least a
   single zero. */
   unsigned m_cchWidth;
   //! Required buffer size.
   unsigned m_cchBuf;
   //! Integer size, in bytes.
   std::uint8_t const mc_cbInt;
   //! 10 (for decimal notation) or log2(notation) (for power-of-two notations).
   std::uint8_t m_iBaseOrShift;
   //! Character to be used to pad the digits to m_cchWidth length.
   char m_chPad;
   //! Character to be used as sign in case the number is not negative; NUL if none.
   char m_chSign;
   //! First character of the prefix; NUL if none (which means that m_chPrefix1 is ignored).
   char m_chPrefix0;
   //! Second character of the prefix; NUL if none.
   char m_chPrefix1;
   //! Map from int [0-15] to its uppercase hexadecimal representation.
   static char const smc_achIntToStrU[16];
   //! Map from int [0-15] to its lowercase hexadecimal representation.
   static char const smc_achIntToStrL[16];
};

#if ABC_HOST_WORD_SIZE >= 32
#if ABC_HOST_WORD_SIZE >= 64

// On a machine with 64-bit word size, write_64*() will be faster.

inline void int_to_str_backend_base::write_s32(std::int32_t i, io::text::writer * ptwOut) const {
   if (m_iBaseOrShift == 10) {
      write_s64(i, ptwOut);
   } else {
      /* Avoid extending the sign in any notation except decimal, as it would generate too many
      digits. */
      write_s64(static_cast<std::uint32_t>(i), ptwOut);
   }
}

inline void int_to_str_backend_base::write_u32(std::uint32_t i, io::text::writer * ptwOut) const {
   write_u64(i, ptwOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 64

/* On a machine with 32-bit word size, write_32*() will be faster. Note that the latter might in
turn defer to write_64*() (see above). */

inline void int_to_str_backend_base::write_s16(std::int16_t i, io::text::writer * ptwOut) const {
   if (m_iBaseOrShift == 10) {
      write_s32(i, ptwOut);
   } else {
      /* Avoid extending the sign in any notation except decimal, as it would generate too many
      digits. */
      write_s32(static_cast<std::uint16_t>(i), ptwOut);
   }
}

inline void int_to_str_backend_base::write_u16(std::uint16_t i, io::text::writer * ptwOut) const {
   write_u32(i, ptwOut);
}

#endif //if ABC_HOST_WORD_SIZE >= 32

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of the specializations of to_str_backend for integer types.
template <typename I>
class int_to_str_backend : public int_to_str_backend_base {
public:
   //! Default constructor.
   int_to_str_backend() :
      int_to_str_backend_base(sizeof(I)) {
   }

   /*! Converts an integer to its string representation.

   This design is rather tricky in the way one implementation calls another:

   1. int_to_str_backend<I>::write()
      Always inlined, dispatches to step 2. based on number of bits;
   2. int_to_str_backend_base::write_{s,u}{8,16,32,64}()
      Inlined to a bit-bigger variant or implemented in to_str_backend.cxx, depending on the host
      architecture’s word size;
   3. int_to_str_backend_base::write_impl()
      Always inlined, but only used in functions defined in to_str_backend.cxx, so it only generates
      as many copies as strictly necessary to have fastest performance for any integer size.

   The net result is that after all the inlining occurs, this will become a direct call to the
   fastest implementation for I of any given size.

   @param i
      Integer to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(I i, io::text::writer * ptwOut) {
      if (sizeof i <= sizeof(std::int8_t)) {
         if (_std::is_signed<I>::value) {
            write_s8(static_cast<std::int8_t>(i), ptwOut);
         } else {
            write_u8(static_cast<std::uint8_t>(i), ptwOut);
         }
      } else if (sizeof i <= sizeof(std::int16_t)) {
         if (_std::is_signed<I>::value) {
            write_s16(static_cast<std::int16_t>(i), ptwOut);
         } else {
            write_u16(static_cast<std::uint16_t>(i), ptwOut);
         }
      } else if (sizeof i <= sizeof(std::int32_t)) {
         if (_std::is_signed<I>::value) {
            write_s32(static_cast<std::int32_t>(i), ptwOut);
         } else {
            write_u32(static_cast<std::uint32_t>(i), ptwOut);
         }
      } else {
         static_assert(sizeof i <= sizeof(std::int64_t), "unsupported integer size");
         if (_std::is_signed<I>::value) {
            write_s64(static_cast<std::int64_t>(i), ptwOut);
         } else {
            write_u64(static_cast<std::uint64_t>(i), ptwOut);
         }
      }
   }

protected:
   //! Initial (static) buffer size sufficient to output the number in binary notation.
   static std::size_t const smc_cchBufInitial = 2 /* prefix or sign */ + 8 * sizeof(I);
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(I) \
   template <> \
   class to_str_backend<I> : public detail::int_to_str_backend<I> {};
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

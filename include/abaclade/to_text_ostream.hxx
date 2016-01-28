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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Defines a member named value that is true if “void T::to_text_ostream(io::text::ostream * ptos)
const” is declared, or false otherwise. */
template <typename T>
struct has_to_text_ostream_member {
   template <typename U, void (U::*)(io::text::ostream *) const>
   struct member_test {};
   template <typename U>
   static long test(member_test<U, &U::to_text_ostream> *);
   template <typename>
   static short test(...);

   static bool const value = (sizeof(test<T>(nullptr)) == sizeof(long));
};

}} //namespace abc::detail

namespace abc {

/*! Writes a string representation of one or more objects of the same type, according to a format
string. Once constructed with the desired format specification, an instance must be able to convert
to string any number of T instances.

The default implementation assumes that a public T member
“void T::to_text_ostream(io::text::ostream * ptos) const” is declared.

This class template and its specializations are at the core of abc::to_str() and
abc::io::text::ostream::print(). */
template <typename T>
class to_text_ostream {
public:
   static_assert(
      detail::has_to_text_ostream_member<T>::value,
      "specialization abc::to_text_ostream<T> must be provided, " \
      "or public “void T::to_text_ostream(io::text::ostream * ptos) const” must be declared"
   );

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat) {
      // TODO: ensure that sFormat is empty, since no format is expected.
      ABC_UNUSED_ARG(sFormat);
   }

   /*! Converts a T instance to its string representation.

   @param t
      T instance to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(T const & t, io::text::ostream * ptos) {
      t.to_text_ostream(ptos);
   }
};

//! @cond
// Partial specializations for cv-qualified T and T reference.
template <typename T>
class to_text_ostream<T const> : public to_text_ostream<T> {};
template <typename T>
class to_text_ostream<T volatile> : public to_text_ostream<T> {};
template <typename T>
class to_text_ostream<T const volatile> : public to_text_ostream<T> {};
template <typename T>
class to_text_ostream<T &> : public to_text_ostream<T> {};
//! @endcond

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_text_ostream<bool> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Converts a boolean value to its string representation.

   @param b
      Boolean value to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(bool b, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Base class for the specializations of to_text_ostream for integer types.
class ABACLADE_SYM int_to_text_ostream_base {
public:
   /*! Constructor.

   @param cbInt
      Size of the integer type.
   */
   int_to_text_ostream_base(unsigned cbInt);

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

protected:
   /*! Writes the provided buffer to *posOut, prefixed as necessary.

   @param bNegative
      true if the number is negative, or false otherwise.
   @param ptos
      Pointer to the stream to output to.
   @param psBuf
      Pointer to the string containing the characters to write.
   @param itBufFirstUsed
      Iterator to the first used character in *psBuf; the last used character is always the last
      character in *psBuf.
   */
   void add_prefixes_and_write(
      bool bNegative, io::text::ostream * ptos, str * psBuf, str::iterator itBufFirstUsed
   ) const;

   /*! Converts an integer to its string representation.

   @param i
      Integer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   template <typename I>
   void write_impl(I i, io::text::ostream * ptos) const;

   //! Converts a 64-bit signed integer to its string representation. See write_impl().
   void write_s64(std::int64_t i, io::text::ostream * ptos) const;

   //! Converts a 64-bit unsigned integer to its string representation. See write_impl().
   void write_u64(std::uint64_t i, io::text::ostream * ptos) const;

   //! Converts a 32-bit signed integer to its string representation. See write_impl().
   void write_s32(std::int32_t i, io::text::ostream * ptos) const;

   //! Converts a 32-bit unsigned integer to its string representation. See write_impl().
   void write_u32(std::uint32_t i, io::text::ostream * ptos) const;

   //! Converts a 16-bit signed integer to its string representation. See write_impl().
   void write_s16(std::int16_t i, io::text::ostream * ptos) const;

   //! Converts a 16-bit unsigned integer to its string representation. See write_impl().
   void write_u16(std::uint16_t i, io::text::ostream * ptos) const;

   //! Converts an 8-bit signed integer to its string representation. See write_impl().
   void write_s8(std::int8_t i, io::text::ostream * ptos) const {
      if (m_iBaseOrShift == 10) {
         write_s16(i, ptos);
      } else {
         /* Avoid extending the sign, as it would generate too many digits in any notation except
         decimal. */
         write_s16(static_cast<std::uint8_t>(i), ptos);
      }
   }

   //! Converts an 8-bit unsigned integer to its string representation. See write_impl().
   void write_u8(std::uint8_t i, io::text::ostream * ptos) const {
      write_u16(i, ptos);
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

inline void int_to_text_ostream_base::write_s32(std::int32_t i, io::text::ostream * ptos) const {
   if (m_iBaseOrShift == 10) {
      write_s64(i, ptos);
   } else {
      /* Avoid extending the sign in any notation except decimal, as it would generate too many
      digits. */
      write_s64(static_cast<std::uint32_t>(i), ptos);
   }
}

inline void int_to_text_ostream_base::write_u32(std::uint32_t i, io::text::ostream * ptos) const {
   write_u64(i, ptos);
}

#endif //if ABC_HOST_WORD_SIZE >= 64

/* On a machine with 32-bit word size, write_32*() will be faster. Note that the latter might in
turn defer to write_64*() (see above). */

inline void int_to_text_ostream_base::write_s16(std::int16_t i, io::text::ostream * ptos) const {
   if (m_iBaseOrShift == 10) {
      write_s32(i, ptos);
   } else {
      /* Avoid extending the sign in any notation except decimal, as it would generate too many
      digits. */
      write_s32(static_cast<std::uint16_t>(i), ptos);
   }
}

inline void int_to_text_ostream_base::write_u16(std::uint16_t i, io::text::ostream * ptos) const {
   write_u32(i, ptos);
}

#endif //if ABC_HOST_WORD_SIZE >= 32

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of the specializations of to_text_ostream for integer types.
template <typename I>
class int_to_text_ostream : public int_to_text_ostream_base {
public:
   //! Default constructor.
   int_to_text_ostream() :
      int_to_text_ostream_base(sizeof(I)) {
   }

   /*! Converts an integer to its string representation.

   This design is rather tricky in the way one implementation calls another:

   1. int_to_text_ostream<I>::write()
      Always inlined, dispatches to step 2. based on number of bits;
   2. int_to_text_ostream_base::write_{s,u}{8,16,32,64}()
      Inlined to a bit-bigger variant or implemented in to_text_ostream.cxx, depending on the host
      architecture’s word size;
   3. int_to_text_ostream_base::write_impl()
      Always inlined, but only used in functions defined in to_text_ostream.cxx, so it only
      generates as many copies as strictly necessary to have fastest performance for any integer
      size.

   The net result is that after all the inlining occurs, this will become a direct call to the
   fastest implementation for I of any given size.

   @param i
      Integer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(I i, io::text::ostream * ptos) {
      if (sizeof i <= sizeof(std::int8_t)) {
         if (_std::is_signed<I>::value) {
            write_s8(static_cast<std::int8_t>(i), ptos);
         } else {
            write_u8(static_cast<std::uint8_t>(i), ptos);
         }
      } else if (sizeof i <= sizeof(std::int16_t)) {
         if (_std::is_signed<I>::value) {
            write_s16(static_cast<std::int16_t>(i), ptos);
         } else {
            write_u16(static_cast<std::uint16_t>(i), ptos);
         }
      } else if (sizeof i <= sizeof(std::int32_t)) {
         if (_std::is_signed<I>::value) {
            write_s32(static_cast<std::int32_t>(i), ptos);
         } else {
            write_u32(static_cast<std::uint32_t>(i), ptos);
         }
      } else {
         static_assert(sizeof i <= sizeof(std::int64_t), "unsupported integer size");
         if (_std::is_signed<I>::value) {
            write_s64(static_cast<std::int64_t>(i), ptos);
         } else {
            write_u64(static_cast<std::uint64_t>(i), ptos);
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

#define ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(I) \
   template <> \
   class to_text_ostream<I> : public detail::int_to_text_ostream<I> {};
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(  signed char)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(unsigned char)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(         short)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(unsigned short)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(     int)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(unsigned)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(         long)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(unsigned long)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(         long long)
ABC_SPECIALIZE_to_text_ostream_FOR_TYPE(unsigned long long)
#undef ABC_SPECIALIZE_to_text_ostream_FOR_TYPE

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Base class for the specializations of to_text_ostream for integer types.
class ABACLADE_SYM ptr_to_text_ostream : public to_text_ostream<std::uintptr_t> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

protected:
   /*! Converts a pointer to a string representation.

   @param iPtr
      Pointer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void _write_impl(std::uintptr_t iPtr, io::text::ostream * ptos);
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

// Specialization for raw pointer types.
template <typename T>
class to_text_ostream<T *> : public detail::ptr_to_text_ostream {
public:
   /*! Converts a pointer to a string representation.

   @param p
      Pointer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(T * p, io::text::ostream * ptos) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p), ptos);
   }
};

// Specialization for _std::unique_ptr.
template <typename T, typename TDel>
class to_text_ostream<_std::unique_ptr<T, TDel>> : public detail::ptr_to_text_ostream {
public:
   //! See detail::ptr_to_text_ostream::write().
   void write(_std::unique_ptr<T, TDel> const & p, io::text::ostream * ptos) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.get()), ptos);
   }
};

// Specialization for _std::shared_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_text_ostream<_std::shared_ptr<T>> : public detail::ptr_to_text_ostream {
public:
   /*! Converts a pointer to a string representation.

   @param p
      Pointer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(_std::shared_ptr<T> const & p, io::text::ostream * ptos) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.get()), ptos);
   }
};

// Specialization for _std::weak_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_text_ostream<_std::weak_ptr<T>> : public detail::ptr_to_text_ostream {
public:
   /*! Converts a pointer to a string representation.

   @param p
      Pointer to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(_std::weak_ptr<T> const & p, io::text::ostream * ptos) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.lock().get()), ptos);
   }
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_text_ostream<_std::type_info> {
public:
   //! Default constructor.
   to_text_ostream();

   //! Constructor.
   ~to_text_ostream();

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes the name of a type, applying the formatting options.

   @param ti
      Type to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(_std::type_info const & ti, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Base class for the specializations of to_text_ostream for sequence types. Not using templates,
so the implementation can be in a .cxx file. */
class ABACLADE_SYM sequence_to_text_ostream {
public:
   /*! Constructor.

   @param sStart
      Sequence start delimiter.
   @param sEnd
      Sequence end delimiter.
   */
   sequence_to_text_ostream(str const & sStart, str const & sEnd);

   //! Destructor.
   ~sequence_to_text_ostream();

   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes the sequence end delimiter.

   @param ptos
      Pointer to the stream to output to.
   */
   void _write_end(io::text::ostream * ptos);

   /*! Writes an element separator.

   @param ptos
      Pointer to the stream to output to.
   */
   void _write_separator(io::text::ostream * ptos);

   /*! Writes the sequence start delimiter.

   @param ptos
      Pointer to the stream to output to.
   */
   void _write_start(io::text::ostream * ptos);

protected:
   //! Separator to be output between elements.
   str m_sSeparator;
   //! Sequence start delimiter.
   str m_sStart;
   //! Sequence end delimiter.
   str m_sEnd;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
#ifdef ABC_CXX_VARIADIC_TEMPLATES

namespace abc { namespace detail {

//! Helper to write a single element out of a tuple, recursing to print any remaining ones.
template <class TTuple, typename... Ts>
class tuple_to_text_ostream_element_writer;

// Base case for the template recursion.
template <class TTuple>
class tuple_to_text_ostream_element_writer<TTuple> {
public:
   /*! Writes the current element to the specified text stream, then recurses to write the rest.

   @param tpl
      Tuple from which to extract the element to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void _write_elements(TTuple const & tpl, io::text::ostream * ptos) {
      ABC_UNUSED_ARG(tpl);
      ABC_UNUSED_ARG(ptos);
   }
};

// Template recursion step.
template <class TTuple, typename T0, typename... Ts>
class tuple_to_text_ostream_element_writer<TTuple, T0, Ts ...> :
   public tuple_to_text_ostream_element_writer<TTuple, Ts ...> {
public:
   //! See tuple_to_text_ostream_element_writer<TTuple>::_write_elements().
   void _write_elements(TTuple const & tpl, io::text::ostream * ptos);

protected:
   //! Backend for the current element type.
   to_text_ostream<T0> m_ttosT0;
};

}} //namespace abc::detail

namespace abc {

template <typename... Ts>
class to_text_ostream<_std::tuple<Ts ...>> :
   public detail::sequence_to_text_ostream,
   public detail::tuple_to_text_ostream_element_writer<_std::tuple<Ts ...>, Ts ...> {
public:
   //! Default constructor.
   to_text_ostream() :
      detail::sequence_to_text_ostream(ABC_SL("("), ABC_SL(")")) {
   }

   /*! Converts a tuple into its string representation.

   @param tpl
      Tuple to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(_std::tuple<Ts ...> const & tpl, io::text::ostream * ptos) {
      _write_start(ptos);
      this->_write_elements(tpl, ptos);
      _write_end(ptos);
   }
};

} //namespace abc

namespace abc { namespace detail {

// Now this can be defined.

template <class TTuple, typename T0, typename... Ts>
inline void tuple_to_text_ostream_element_writer<TTuple, T0, Ts ...>::_write_elements(
   TTuple const & tpl, io::text::ostream * ptos
) {
   m_ttosT0.write(_std::get<
      _std::tuple_size<TTuple>::value - (1 /*Ts*/ + sizeof ...(Ts))
   >(tpl), ptos);
   // If there are any remaining elements, write a separator and recurse to write the rest.
   if (sizeof ...(Ts)) {
      static_cast<to_text_ostream<TTuple> *>(this)->_write_separator(ptos);
      tuple_to_text_ostream_element_writer<TTuple, Ts ...>::_write_elements(tpl, ptos);
   }
}

}} //namespace abc::detail

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

namespace abc { namespace detail {

//! Helper to write the elements of a tuple.
// Template recursion step.
template <
   class TTuple, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
class tuple_to_text_ostream_element_writer :
   public tuple_to_text_ostream_element_writer<
      TTuple, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::detail::tuple_void
   > {
public:
   //! See tuple_to_text_ostream_element_writer<TTuple>::_write_elements().
   void _write_elements(TTuple const & tpl, io::text::ostream * ptos);

protected:
   //! Backend for the current element type.
   to_text_ostream<T0> m_ttosT0;
};

// Base case for the template recursion.
template <class TTuple>
class tuple_to_text_ostream_element_writer<
   TTuple,
   _std::detail::tuple_void, _std::detail::tuple_void, _std::detail::tuple_void,
   _std::detail::tuple_void, _std::detail::tuple_void, _std::detail::tuple_void,
   _std::detail::tuple_void, _std::detail::tuple_void, _std::detail::tuple_void,
   _std::detail::tuple_void
> {
public:
   /*! Writes the current element to the specified text stream, then recurses to write the rest.

   @param tpl
      Tuple from which to extract the element to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void _write_elements(TTuple const & tpl, io::text::ostream * ptos) {
      ABC_UNUSED_ARG(tpl);
      ABC_UNUSED_ARG(ptos);
   }
};

}} //namespace abc::detail

namespace abc {

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
class to_text_ostream<_std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> :
   public detail::sequence_to_text_ostream,
   public detail::tuple_to_text_ostream_element_writer<
      _std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
   > {
public:
   //! Constructor.
   to_text_ostream() :
      detail::sequence_to_text_ostream(ABC_SL("("), ABC_SL(")")) {
   }

   /*! Converts a tuple into its string representation.

   @param tpl
      Tuple to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(
      _std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl, io::text::ostream * ptos
   ) {
      _write_start(ptos);
      this->_write_elements(tpl, ptos);
      _write_end(ptos);
   }
};

} //namespace abc

// Now this can be defined.

namespace abc { namespace detail {

template <
   class TTuple, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline void tuple_to_text_ostream_element_writer<
   TTuple, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
>::_write_elements(TTuple const & tpl, io::text::ostream * ptos) {
   static std::size_t const sc_cTs(
      _std::tuple_size<_std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>>::value
   );
   m_ttosT0.write(_std::get<_std::tuple_size<TTuple>::value - (1 /*T0*/ + sc_cTs)>(tpl), ptos);
   // If there are any remaining elements, write a separator and recurse to write the rest.
   if (sc_cTs) {
      static_cast<to_text_ostream<TTuple> *>(this)->_write_separator(ptos);
      tuple_to_text_ostream_element_writer<
         TTuple, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::detail::tuple_void
      >::_write_elements(tpl, ptos);
   }
}

}} //namespace abc::detail

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else
//! @endcond

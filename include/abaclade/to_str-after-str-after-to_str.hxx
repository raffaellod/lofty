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

/*! Base class for the specializations of to_text_ostream for sequence types. Not using templates, so
the implementation can be in a .cxx file. */
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

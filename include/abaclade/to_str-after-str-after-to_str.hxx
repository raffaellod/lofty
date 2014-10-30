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

#ifndef _ABACLADE_HXX_INTERNAL
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::char_ptr_to_str_adapter

namespace abc {

/*! Adapter to allow printing of C-style NUL-terminated char * strings via to_str_backend. Use this
for compatibility with STL methods such as std::exception::what(). Without this, C strings are
printed only as pointers, which is often undesirable.

Instances of this class don’t own the memory object they point to. */
class char_ptr_to_str_adapter {
private:
   friend class to_str_backend<char_ptr_to_str_adapter>;

public:
   /*! Constructor.

   psz
      C-style NUL-terminated string.
   */
   char_ptr_to_str_adapter(char const * psz) :
      m_psz(psz) {
   }

protected:
   //! Wrapped C-style string.
   char const * m_psz;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for char_ptr_to_str_adapter

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<char_ptr_to_str_adapter> : public detail::str_to_str_backend {
public:
   /*! Writes a C-style NUL-terminated string, applying the formatting options.

   cs
      C string to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(char_ptr_to_str_adapter const & cs, io::text::writer * ptwOut);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::ptr_to_str_backend

namespace abc {
namespace detail {

//! Base class for the specializations of to_str_backend for integer types.
class ABACLADE_SYM ptr_to_str_backend {
public:
   //! Constructor.
   ptr_to_str_backend();

   /*! Changes the output format.

   sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

protected:
   /*! Converts a pointer to a string representation.

   iPtr
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void _write_impl(std::uintptr_t iPtr, io::text::writer * ptwOut);

protected:
   //! Backend used to write the pointer as an integer.
   to_str_backend<std::uintptr_t> m_tsbInt;
   //! Backend used to write a nullptr.
   to_str_backend<istr> m_tsbStr;
   //! Format string used to display the address.
   static char_t const smc_achFormat[];
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specializations for pointer types

namespace abc {

// Specialization for raw pointer types.
template <typename T>
class to_str_backend<T *> : public detail::ptr_to_str_backend {
public:
   /*! Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(T * p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p), ptwOut);
   }
};

// Specialization for std::unique_ptr.
template <typename T, typename TDel>
class to_str_backend<std::unique_ptr<T, TDel>> : public detail::ptr_to_str_backend {
public:
   //! See detail::ptr_to_str_backend::write().
   void write(std::unique_ptr<T, TDel> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.get()), ptwOut);
   }
};

// Specialization for std::shared_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_str_backend<std::shared_ptr<T>> : public detail::ptr_to_str_backend {
public:
   /*! Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(std::shared_ptr<T> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.get()), ptwOut);
   }
};

// Specialization for std::weak_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_str_backend<std::weak_ptr<T>> : public detail::ptr_to_str_backend {
public:
   /*! Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(std::weak_ptr<T> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<std::uintptr_t>(p.lock().get()), ptwOut);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_sequence_to_str_backend

namespace abc {

/*! Base class for the specializations of to_str_backend for sequence types. Not using templates, so
the implementation can be in a cxx file. */
class ABACLADE_SYM _sequence_to_str_backend {
public:
   /*! Constructor.

   sStart
      Sequence start delimiter.
   sEnd
      Sequence end delimiter.
   */
   _sequence_to_str_backend(istr const & sStart, istr const & sEnd);

   //! Destructor.
   ~_sequence_to_str_backend();

   /*! Changes the output format.

   sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

   /*! Writes the sequence end delimiter.

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_end(io::text::writer * ptwOut) {
      m_tsbStr.write(m_sEnd, ptwOut);
   }

   /*! Writes an element separator (typically a comma).

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_separator(io::text::writer * ptwOut) {
      m_tsbStr.write(m_sSeparator, ptwOut);
   }

   /*! Writes the sequence start delimiter.

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_start(io::text::writer * ptwOut) {
      m_tsbStr.write(m_sStart, ptwOut);
   }

protected:
   //! Separator to be output between elements.
   istr m_sSeparator;
   //! Sequence start delimiter.
   istr m_sStart;
   //! Sequence end delimiter.
   istr m_sEnd;
   //! Backend for strings.
   to_str_backend<istr> m_tsbStr;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for std::tuple or abc::_std::tuple

namespace abc {

#ifdef ABC_CXX_VARIADIC_TEMPLATES

//! Helper to write a single element out of a tuple, recursing to print any remaining ones.
template <class TTuple, typename... Ts>
class _tuple_to_str_backend_element_writer;

// Base case for the template recursion.
template <class TTuple>
class _tuple_to_str_backend_element_writer<TTuple> {
public:
   /*! Writes the current element to the specified text writer, then recurses to write the rest.

   tpl
      Tuple from which to extract the element to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void _write_elements(TTuple const & tpl, io::text::writer * ptwOut) {
      ABC_UNUSED_ARG(tpl);
      ABC_UNUSED_ARG(ptwOut);
   }
};

// Template recursion step.
template <class TTuple, typename T0, typename... Ts>
class _tuple_to_str_backend_element_writer<TTuple, T0, Ts ...> :
   public _tuple_to_str_backend_element_writer<TTuple, Ts ...> {
public:
   //! See _tuple_to_str_backend_element_writer<TTuple>::_write_elements().
   void _write_elements(TTuple const & tpl, io::text::writer * ptwOut);

protected:
   //! Backend for the current element type.
   to_str_backend<T0> m_tsbt0;
};

template <typename... Ts>
class to_str_backend<std::tuple<Ts ...>> :
   public _sequence_to_str_backend,
   public _tuple_to_str_backend_element_writer<std::tuple<Ts ...>, Ts ...> {
public:
   //! Constructor.
   to_str_backend() :
      _sequence_to_str_backend(ABC_SL("("), ABC_SL(")")) {
   }

   /*! Converts a tuple into its string representation.

   tpl
      Tuple to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(std::tuple<Ts ...> const & tpl, io::text::writer * ptwOut) {
      _write_start(ptwOut);
      this->_write_elements(tpl, ptwOut);
      _write_end(ptwOut);
   }
};


// Now this can be implemented.

template <class TTuple, typename T0, typename... Ts>
inline void _tuple_to_str_backend_element_writer<TTuple, T0, Ts ...>::_write_elements(
   TTuple const & tpl, io::text::writer * ptwOut
) {
   m_tsbt0.write(std::get<
      std::tuple_size<TTuple>::value - (1 /*Ts*/ + sizeof ...(Ts))
   >(tpl), ptwOut);
   // If there are any remaining elements, write a separator and recurse to write the rest.
   if (sizeof ...(Ts)) {
      static_cast<to_str_backend<TTuple> *>(this)->_write_separator(ptwOut);
      _tuple_to_str_backend_element_writer<TTuple, Ts ...>::_write_elements(tpl, ptwOut);
   }
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

//! Helper to write the elements of a tuple.
// Template recursion step.
template <
   class TTuple, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
class _tuple_to_str_backend_element_writer :
   public _tuple_to_str_backend_element_writer<
      TTuple, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::_tuple_void
   > {
public:
   //! See _tuple_to_str_backend_element_writer<TTuple>::_write_elements().
   void _write_elements(TTuple const & tpl, io::text::writer * ptwOut);

protected:
   //! Backend for the current element type.
   to_str_backend<T0> m_tsbt0;
};

// Base case for the template recursion.
template <class TTuple>
class _tuple_to_str_backend_element_writer<
   TTuple, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void,
   _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void, _std::_tuple_void,
   _std::_tuple_void
> {
public:
   /*! Writes the current element to the specified text writer, then recurses to write the rest.

   tpl
      Tuple from which to extract the element to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void _write_elements(TTuple const & tpl, io::text::writer * ptwOut) {
      ABC_UNUSED_ARG(tpl);
      ABC_UNUSED_ARG(ptwOut);
   }
};


template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
class to_str_backend<_std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> :
   public _sequence_to_str_backend,
   public _tuple_to_str_backend_element_writer<
      _std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
   > {
public:
   //! Constructor.
   to_str_backend() :
      _sequence_to_str_backend(ABC_SL("("), ABC_SL(")")) {
   }

   /*! Converts a tuple into its string representation.

   tpl
      Tuple to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(
      _std::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl, io::text::writer * ptwOut
   ) {
      _write_start(ptwOut);
      this->_write_elements(tpl, ptwOut);
      _write_end(ptwOut);
   }
};


// Now this can be implemented.

template <
   class TTuple, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline void _tuple_to_str_backend_element_writer<
   TTuple, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9
>::_write_elements(TTuple const & tpl, io::text::writer * ptwOut) {
   static std::size_t const sc_cTs(
      _std::tuple_size<_std::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>>::value
   );
   m_tsbt0.write(_std::get<_std::tuple_size<TTuple>::value - (1 /*T0*/ + sc_cTs)>(tpl), ptwOut);
   // If there are any remaining elements, write a separator and recurse to write the rest.
   if (sc_cTs) {
      static_cast<to_str_backend<TTuple> *>(this)->_write_separator(ptwOut);
      _tuple_to_str_backend_element_writer<
         TTuple, T1, T2, T3, T4, T5, T6, T7, T8, T9, _std::_tuple_void
      >::_write_elements(tpl, ptwOut);
   }
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////


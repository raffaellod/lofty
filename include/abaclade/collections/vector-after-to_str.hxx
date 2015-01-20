/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
// abc::detail::vector_to_str_backend

namespace abc {
namespace detail {

/*! Base class for the specializations of to_str_backend for vector types. Not using templates, so
the implementation can be in a cxx file. */
class ABACLADE_SYM vector_to_str_backend : public detail::sequence_to_str_backend {
public:
   //! Constructor.
   vector_to_str_backend();

   //! Destructor.
   ~vector_to_str_backend();

protected:
   /*! Formatting options to be applied to the individual elements, obtained from the constructor
   argument sFormat. */
   istr m_sEltFormat;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::vector_base

namespace abc {

// Specialization of to_str_backend.
template <typename T>
class to_str_backend<vector_base<T>> : public detail::vector_to_str_backend {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat) {
//    ABC_TRACE_FUNC(this, sFormat);

      detail::vector_to_str_backend::set_format(sFormat);
      m_tsbElt.set_format(m_sEltFormat);
   }

   /*! Writes a vector, applying the formatting options.

   @param v
      Vector to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(vector_base<T> const & v, io::text::writer * ptwOut) {
//    ABC_TRACE_FUNC(this, v, ptwOut);

      _write_start(ptwOut);
      auto it(v.cbegin()), itEnd(v.cend());
      if (it != itEnd) {
         m_tsbElt.write(*it, ptwOut);
         while (++it != itEnd) {
            _write_separator(ptwOut);
            m_tsbElt.write(*it, ptwOut);
         }
      }
      _write_end(ptwOut);
   }

protected:
   //! Backend for the individual elements.
   to_str_backend<T> m_tsbElt;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::*vector

namespace abc {

template <typename T>
class to_str_backend<mvector<T>> : public to_str_backend<vector_base<T>> {};

template <typename T>
class to_str_backend<dmvector<T>> : public to_str_backend<vector_base<T>> {};

template <typename T, std::size_t t_ciStatic>
class to_str_backend<smvector<T, t_ciStatic>> : public to_str_backend<vector_base<T>> {};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

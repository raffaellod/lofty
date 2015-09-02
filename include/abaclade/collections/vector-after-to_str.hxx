/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

namespace abc { namespace collections { namespace detail {

/*! Base class for the specializations of to_str_backend for vector types. Not using templates, so
the implementation can be in a cxx file. */
class ABACLADE_SYM vector_to_str_backend : public abc::detail::sequence_to_str_backend {
public:
   //! Default constructor.
   vector_to_str_backend();

   //! Destructor.
   ~vector_to_str_backend();

protected:
   /*! Formatting options to be applied to the individual elements, obtained from the constructor
   argument sFormat. */
   str m_sEltFormat;
};

}}} //namespace abc::collections::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

// Specialization of to_str_backend.
template <typename T, std::size_t t_ciEmbeddedCapacity>
class to_str_backend<collections::vector<T, t_ciEmbeddedCapacity>> :
   public collections::detail::vector_to_str_backend {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat) {
//    ABC_TRACE_FUNC(this, sFormat);

      collections::detail::vector_to_str_backend::set_format(sFormat);
      m_tsbElt.set_format(m_sEltFormat);
   }

   /*! Writes a vector, applying the formatting options.

   @param v
      Vector to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(collections::vector<T, t_ciEmbeddedCapacity> const & v, io::text::writer * ptwOut) {
//    ABC_TRACE_FUNC(this/*, v*/, ptwOut);

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

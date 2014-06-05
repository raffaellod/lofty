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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_vector_to_str_backend


namespace abc {

/** Base class for the specializations of to_str_backend for vector types. Not using templates, so
the implementation can be in a cxx file.
*/
class ABACLADE_SYM _vector_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   _vector_to_str_backend(istr const & sFormat);


   /** Destructor.
   */
   ~_vector_to_str_backend();


protected:

   /** Writes a list end delimiter (typically a closed brace).

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_end(io::text::writer * ptwOut);


   /** Writes an element separator (typically a comma).

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_separator(io::text::writer * ptwOut);


   /** Writes a list start delimiter (typically an open brace).

   ptwOut
      Pointer to the writer to output to.
   */
   void _write_start(io::text::writer * ptwOut);


protected:

   /** Formatting options to be applied to the individual elements, obtained from the constructor
   argument sFormat. */
   istr m_sEltFormat;
   /** Separator to be output between elements. */
   istr m_sSeparator;
   /** Backend for strings. */
   to_str_backend<istr> m_tsbStr;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::vector_base


namespace abc {

// Specialization of to_str_backend.
template <typename T>
class to_str_backend<vector_base<T>> :
   public _vector_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      _vector_to_str_backend(sFormat),
      m_tsbElt(m_sEltFormat) {
   }


   /** Writes a vector, applying the formatting options.

   v
      Vector to write.
   ptwOut
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

   /** Backend for the individual elements. */
   to_str_backend<T> m_tsbElt;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::mvector


namespace abc {

template <typename T>
class to_str_backend<mvector<T>> :
   public to_str_backend<vector_base<T>> {
public:

   /** Constructor. See to_str_backend<vector_base<T>>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<vector_base<T>>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::dmvector


namespace abc {

template <typename T>
class to_str_backend<dmvector<T>> :
   public to_str_backend<vector_base<T>> {
public:

   /** Constructor. See to_str_backend<vector_base<T>>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<vector_base<T>>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::smvector


namespace abc {

template <typename T, size_t t_ciStatic>
class to_str_backend<smvector<T, t_ciStatic>> :
   public to_str_backend<vector_base<T>> {
public:

   /** Constructor. See to_str_backend<vector_base<T>>::to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<vector_base<T>>(sFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


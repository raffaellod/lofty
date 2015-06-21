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
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

template <class T>
inline istr enum_impl<T>::name() const {
   detail::enum_member const * pem = _member();
   return istr(external_buffer, pem->pszName, pem->cchName);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of the specializations of to_str_backend for enum_impl specializations.
class ABACLADE_SYM enum_to_str_backend_impl {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

protected:
   /*! Writes an enumeration value, applying the formatting options.

   @param e
      Enumeration value to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write_impl(int i, enum_member const * pem, io::text::writer * ptwOut);
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

// Specialization of to_str_backend.
template <class T>
class to_str_backend<enum_impl<T>> : public detail::enum_to_str_backend_impl {
public:
   //! See abc::detail::enum_to_str_backend_impl::write().
   void write(enum_impl<T> e, io::text::writer * ptwOut) {
      detail::enum_to_str_backend_impl::write_impl(e.base(), e._get_map(), ptwOut);
   }
};

} //namespace abc

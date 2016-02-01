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

#include <abaclade.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

/*static*/ enum_member const * enum_member::find_in_map(enum_member const * pem, int iValue) {
   ABC_TRACE_FUNC(pem, iValue);

   for (; pem->pszName; ++pem) {
      if (iValue == pem->iValue) {
         return pem;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}
/*static*/ enum_member const * enum_member::find_in_map(
   enum_member const * pem, str const & sName
) {
   ABC_TRACE_FUNC(pem, sName);

   for (; pem->pszName; ++pem) {
      if (sName == str(external_buffer, pem->pszName, pem->cchName)) {
         return pem;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}

}} //namespace abc::_pvt

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _pvt {

void enum_to_text_ostream_impl::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(it, sFormat);
}

void enum_to_text_ostream_impl::write_impl(
   int i, enum_member const * pem, io::text::ostream * ptos
) {
   ABC_TRACE_FUNC(this, i, pem, ptos);

   enum_member const * petvp = enum_member::find_in_map(pem, i);
   ptos->write(str(external_buffer, petvp->pszName));
}

}} //namespace abc::_pvt

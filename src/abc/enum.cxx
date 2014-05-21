/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

/*static*/ enum_member const * enum_member::find_in_map(enum_member const * pem, int iValue) {
   ABC_TRACE_FN((pem, iValue));

   for (; pem->pszName; ++pem) {
      if (iValue == pem->iValue) {
         return pem;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}
/*static*/ enum_member const * enum_member::find_in_map(
   enum_member const * pem, istr const & sName
) {
   ABC_TRACE_FN((pem, sName));

   for (; pem->pszName; ++pem) {
      if (sName == istr(unsafe, pem->pszName, pem->cchName)) {
         return pem;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_enum_to_str_backend_impl


namespace abc {

_enum_to_str_backend_impl::_enum_to_str_backend_impl(istr const & sFormat) {
   ABC_TRACE_FN((this, sFormat));

   auto it(sFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), sFormat, unsigned(it - sFormat.cbegin())
      ));
   }
}


void _enum_to_str_backend_impl::write_impl(
   int i, enum_member const * pem, io::text::writer * ptwOut
) {
   ABC_TRACE_FN((this, i, pem, ptwOut));

   enum_member const * petvp(enum_member::find_in_map(pem, i));
   // TODO: apply format options.
   ptwOut->write(istr(unsafe, petvp->pszName));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


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
#include <abc/utf_traits.hxx>
#include <abc/iostream.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

/*static*/ enum_member const * enum_member::find_in_map(enum_member const * pem, int i) {
   ABC_TRACE_FN((pem, i));

   for (; pem->pszName; ++pem) {
      if (i == pem->iValue) {
         return pem;
      }
   }
   // TODO: provide more information in the exception.
   ABC_THROW(domain_error, ());
}
/*static*/ enum_member const * enum_member::find_in_map(
   enum_member const * pem, char_t const * psz
) {
   ABC_TRACE_FN((pem, psz));

   for (; pem->pszName; ++pem) {
      if (text::utf_traits<>::str_cmp(psz, pem->pszName) == 0) {
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

_enum_to_str_backend_impl::_enum_to_str_backend_impl(char_range const & crFormat) {
   ABC_TRACE_FN((this, crFormat));

   auto it(crFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != crFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
      ));
   }
}


void _enum_to_str_backend_impl::write_impl(int i, enum_member const * pem, io::ostream * posOut) {
   ABC_TRACE_FN((this, i, pem, posOut));

   enum_member const * petvp(enum_member::find_in_map(pem, i));
   // TODO: apply format options.
   posOut->write(istr(unsafe, petvp->pszName));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


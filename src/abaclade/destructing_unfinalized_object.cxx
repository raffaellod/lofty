/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

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
#include <abaclade/destructing_unfinalized_object.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

destructing_unfinalized_object::destructing_unfinalized_object(
   destructing_unfinalized_object const & x
) :
   exception(x) {
}

/*virtual*/ destructing_unfinalized_object::~destructing_unfinalized_object(
) ABC_STL_NOEXCEPT_TRUE() {
}

destructing_unfinalized_object & destructing_unfinalized_object::operator=(
   destructing_unfinalized_object const & x
) {
   exception::operator=(x);
   return *this;
}

void destructing_unfinalized_object::write_what(void const * pObj, _std::type_info const & ti) {
   what_ostream().print(
      ABC_SL("instance of {} @ {} being destructed before finalize() was invoked on it"), ti, pObj
   );
}

} //namespace abc

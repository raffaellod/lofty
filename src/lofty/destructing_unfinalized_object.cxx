/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/destructing_unfinalized_object.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

destructing_unfinalized_object::destructing_unfinalized_object(destructing_unfinalized_object const & src) :
   exception(src) {
}

/*virtual*/ destructing_unfinalized_object::~destructing_unfinalized_object() LOFTY_STL_NOEXCEPT_TRUE() {
}

destructing_unfinalized_object & destructing_unfinalized_object::operator=(
   destructing_unfinalized_object const & src
) {
   exception::operator=(src);
   return *this;
}

void destructing_unfinalized_object::write_what(void const * o, _std::type_info const & type) {
   what_ostream().print(
      LOFTY_SL("instance of {} @ {} being destructed before finalize() was invoked on it"), type, o
   );
}

} //namespace lofty

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

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
#include <lofty/collections.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ bad_access::bad_access(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

bad_access::bad_access(bad_access const & src) :
   generic_error(src) {
}

/*virtual*/ bad_access::~bad_access() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_access & bad_access::operator=(bad_access const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ bad_key::bad_key(errint_t err_ /*= 0*/) :
   bad_access(err_) {
}

bad_key::bad_key(bad_key const & src) :
   bad_access(src) {
}

/*virtual*/ bad_key::~bad_key() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_key & bad_key::operator=(bad_key const & src) {
   bad_access::operator=(src);
   return *this;
}

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

/*explicit*/ out_of_range::out_of_range(errint_t err_ /*= 0*/) :
   bad_access(err_) {
}

out_of_range::out_of_range(
   std::ptrdiff_t invalid, std::ptrdiff_t min, std::ptrdiff_t max, errint_t err_ /*= 0*/
) :
   bad_access(err_) {
   what_ostream().print(LOFTY_SL("invalid value={} valid range=[{}, {}]"), invalid, min, max);
}

out_of_range::out_of_range(void const * invalid, void const * min, void const * max, errint_t err_ /*= 0*/) :
   bad_access(err_) {
   what_ostream().print(LOFTY_SL("invalid value={} valid range=[{}, {}]"), invalid, min, max);
}

out_of_range::out_of_range(out_of_range const & src) :
   bad_access(src) {
}

/*virtual*/ out_of_range::~out_of_range() LOFTY_STL_NOEXCEPT_TRUE() {
}

out_of_range & out_of_range::operator=(out_of_range const & src) {
   bad_access::operator=(src);
   return *this;
}

}} //namespace lofty::collections

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
#include <abaclade/collections.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*explicit*/ bad_access::bad_access(errint_t err /*= 0*/) :
   generic_error(err) {
}

bad_access::bad_access(bad_access const & x) :
   generic_error(x) {
}

/*virtual*/ bad_access::~bad_access() ABC_STL_NOEXCEPT_TRUE() {
}

bad_access & bad_access::operator=(bad_access const & x) {
   generic_error::operator=(x);
   return *this;
}

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*explicit*/ bad_key::bad_key(errint_t err /*= 0*/) :
   bad_access(err) {
}

bad_key::bad_key(bad_key const & x) :
   bad_access(x) {
}

/*virtual*/ bad_key::~bad_key() ABC_STL_NOEXCEPT_TRUE() {
}

bad_key & bad_key::operator=(bad_key const & x) {
   bad_access::operator=(x);
   return *this;
}

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*explicit*/ out_of_range::out_of_range(errint_t err /*= 0*/) :
   bad_access(err) {
}

out_of_range::out_of_range(
   std::ptrdiff_t iInvalid, std::ptrdiff_t iMin, std::ptrdiff_t iMax, errint_t err /*= 0*/
) :
   bad_access(err) {
   what_writer().print(ABC_SL("invalid value={} valid range=[{}, {}]"), iInvalid, iMin, iMax);
}

out_of_range::out_of_range(
   void const * pInvalid, void const * pMin, void const * pMax, errint_t err /*= 0*/
) :
   bad_access(err) {
   what_writer().print(ABC_SL("invalid value={} valid range=[{}, {}]"), pInvalid, pMin, pMax);
}

out_of_range::out_of_range(out_of_range const & x) :
   bad_access(x) {
}

/*virtual*/ out_of_range::~out_of_range() ABC_STL_NOEXCEPT_TRUE() {
}

out_of_range & out_of_range::operator=(out_of_range const & x) {
   bad_access::operator=(x);
   return *this;
}

}} //namespace abc::collections

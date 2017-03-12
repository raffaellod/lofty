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
#include <lofty/math.hxx>

#include <cstdlib> // std::free() std::malloc() std::realloc()
#if LOFTY_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ arithmetic_error::arithmetic_error(errint_t err_ /*= 0*/) :
   generic_error(err_) {
}

arithmetic_error::arithmetic_error(arithmetic_error const & src) :
   generic_error(src) {
}

/*virtual*/ arithmetic_error::~arithmetic_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

arithmetic_error & arithmetic_error::operator=(arithmetic_error const & src) {
   generic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ division_by_zero::division_by_zero(errint_t err_ /*= 0*/) :
   arithmetic_error(err_) {
}

division_by_zero::division_by_zero(division_by_zero const & src) :
   arithmetic_error(src) {
}

/*virtual*/ division_by_zero::~division_by_zero() LOFTY_STL_NOEXCEPT_TRUE() {
}

division_by_zero & division_by_zero::operator=(division_by_zero const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ floating_point_error::floating_point_error(errint_t err_ /*= 0*/) :
   arithmetic_error(err_) {
}

floating_point_error::floating_point_error(floating_point_error const & src) :
   arithmetic_error(src) {
}

/*virtual*/ floating_point_error::~floating_point_error() LOFTY_STL_NOEXCEPT_TRUE() {
}

floating_point_error & floating_point_error::operator=(floating_point_error const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace math {

/*explicit*/ overflow::overflow(errint_t err_ /*= 0*/) :
   arithmetic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EOVERFLOW
#else
      0
#endif
   ) {
}

overflow::overflow(overflow const & src) :
   arithmetic_error(src) {
}

/*virtual*/ overflow::~overflow() LOFTY_STL_NOEXCEPT_TRUE() {
}

overflow & overflow::operator=(overflow const & src) {
   arithmetic_error::operator=(src);
   return *this;
}

}} //namespace lofty::math

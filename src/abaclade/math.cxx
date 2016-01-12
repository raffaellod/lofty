/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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
#include <abaclade/math.hxx>

#include <cstdlib> // std::free() std::malloc() std::realloc()
#if ABC_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

/*explicit*/ arithmetic_error::arithmetic_error(errint_t err /*= 0*/) :
   generic_error(err) {
}

arithmetic_error::arithmetic_error(arithmetic_error const & x) :
   generic_error(x) {
}

/*virtual*/ arithmetic_error::~arithmetic_error() ABC_STL_NOEXCEPT_TRUE() {
}

arithmetic_error & arithmetic_error::operator=(arithmetic_error const & x) {
   generic_error::operator=(x);
   return *this;
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

/*explicit*/ division_by_zero::division_by_zero(errint_t err /*= 0*/) :
   arithmetic_error(err) {
}

division_by_zero::division_by_zero(division_by_zero const & x) :
   arithmetic_error(x) {
}

/*virtual*/ division_by_zero::~division_by_zero() ABC_STL_NOEXCEPT_TRUE() {
}

division_by_zero & division_by_zero::operator=(division_by_zero const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

/*explicit*/ floating_point_error::floating_point_error(errint_t err /*= 0*/) :
   arithmetic_error(err) {
}

floating_point_error::floating_point_error(floating_point_error const & x) :
   arithmetic_error(x) {
}

/*virtual*/ floating_point_error::~floating_point_error() ABC_STL_NOEXCEPT_TRUE() {
}

floating_point_error & floating_point_error::operator=(floating_point_error const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

/*explicit*/ overflow::overflow(errint_t err /*= 0*/) :
   arithmetic_error(err ? err :
#if ABC_HOST_API_POSIX
      EOVERFLOW
#else
      0
#endif
   ) {
}

overflow::overflow(overflow const & x) :
   arithmetic_error(x) {
}

/*virtual*/ overflow::~overflow() ABC_STL_NOEXCEPT_TRUE() {
}

overflow & overflow::operator=(overflow const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

}} //namespace abc::math

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#include <abaclade.hxx>
#include <abaclade/math.hxx>

#include <cstdlib> // std::free() std::malloc() std::realloc()
#if ABC_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

arithmetic_error::arithmetic_error() :
   generic_error() {
   m_pszWhat = "abc::math::arithmetic_error";
}

arithmetic_error::arithmetic_error(arithmetic_error const & x) :
   generic_error(x) {
}

/*virtual*/ arithmetic_error::~arithmetic_error() {
}

arithmetic_error & arithmetic_error::operator=(arithmetic_error const & x) {
   generic_error::operator=(x);
   return *this;
}

void arithmetic_error::init(errint_t err /*= 0*/) {
   generic_error::init(err);
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

division_by_zero::division_by_zero() :
   arithmetic_error() {
   m_pszWhat = "abc::math::division_by_zero";
}

division_by_zero::division_by_zero(division_by_zero const & x) :
   arithmetic_error(x) {
}

/*virtual*/ division_by_zero::~division_by_zero() {
}

division_by_zero & division_by_zero::operator=(division_by_zero const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void division_by_zero::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err);
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

floating_point_error::floating_point_error() :
   arithmetic_error() {
   m_pszWhat = "abc::math::floating_point_error";
}

floating_point_error::floating_point_error(floating_point_error const & x) :
   arithmetic_error(x) {
}

/*virtual*/ floating_point_error::~floating_point_error() {
}

floating_point_error & floating_point_error::operator=(floating_point_error const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void floating_point_error::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err);
}

}} //namespace abc::math

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace math {

overflow::overflow() :
   arithmetic_error() {
   m_pszWhat = "abc::math::overflow";
}

overflow::overflow(overflow const & x) :
   arithmetic_error(x) {
}

/*virtual*/ overflow::~overflow() {
}

overflow & overflow::operator=(overflow const & x) {
   arithmetic_error::operator=(x);
   return *this;
}

void overflow::init(errint_t err /*= 0*/) {
   arithmetic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EOVERFLOW
#else
      0
#endif
   );
}

}} //namespace abc::math

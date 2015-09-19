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
#include <abaclade/collections.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

bad_access::bad_access() {
   m_pszWhat = "abc::collections::bad_access";
}

bad_access::bad_access(bad_access const & x) :
   generic_error(x) {
}

/*virtual*/ bad_access::~bad_access() {
}

bad_access & bad_access::operator=(bad_access const & x) {
   generic_error::operator=(x);
   return *this;
}

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

bad_key::bad_key() {
   m_pszWhat = "abc::collections::bad_key";
}

bad_key::bad_key(bad_key const & x) :
   bad_access(x) {
}

/*virtual*/ bad_key::~bad_key() {
}

bad_key & bad_key::operator=(bad_key const & x) {
   bad_access::operator=(x);
   return *this;
}

void bad_key::init(errint_t err /*= 0*/) {
   bad_access::init(err);
}

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

out_of_range::out_of_range() {
   m_pszWhat = "abc::collections::out_of_range";
}
out_of_range::out_of_range(out_of_range const & x) :
   bad_access(x),
   m_pInvalid(x.m_pInvalid),
   m_pMin(x.m_pMin),
   m_pMax(x.m_pMax),
   m_bRangeProvided(x.m_bRangeProvided),
   m_bWriteAsInts(x.m_bWriteAsInts) {
}

/*virtual*/ out_of_range::~out_of_range() {
}

out_of_range & out_of_range::operator=(out_of_range const & x) {
   bad_access::operator=(x);
   m_pInvalid = x.m_pInvalid;
   m_pMin = x.m_pMin;
   m_pMax = x.m_pMax;
   m_bRangeProvided = x.m_bRangeProvided;
   m_bWriteAsInts = x.m_bWriteAsInts;
   return *this;
}

void out_of_range::init(errint_t err /*= 0*/) {
   bad_access::init(err);
   m_bRangeProvided = false;
   m_bWriteAsInts = false;
}

void out_of_range::init(
   std::ptrdiff_t iInvalid, std::ptrdiff_t iMin, std::ptrdiff_t iMax, errint_t err /*= 0*/
) {
   bad_access::init(err);
   m_pInvalid = reinterpret_cast<void *>(iInvalid);
   m_pMin = reinterpret_cast<void *>(iMin);
   m_pMax = reinterpret_cast<void *>(iMax);
   m_bRangeProvided = true;
   m_bWriteAsInts = true;
}

void out_of_range::init(
   void const * pInvalid, void const * pMin, void const * pMax, errint_t err /*= 0*/
) {
   bad_access::init(err);
   m_pInvalid = pInvalid;
   m_pMin = pMin;
   m_pMax = pMax;
   m_bRangeProvided = true;
   m_bWriteAsInts = false;
}

/*virtual*/ void out_of_range::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   bad_access::write_extended_info(ptwOut);
   if (m_bRangeProvided) {
      str sFormat(ABC_SL(" invalid value={}; valid range=[{}, {}]"));
      if (m_bWriteAsInts) {
         ptwOut->print(
            sFormat,
            reinterpret_cast<std::ptrdiff_t>(m_pInvalid), reinterpret_cast<std::ptrdiff_t>(m_pMin),
            reinterpret_cast<std::ptrdiff_t>(m_pMax)
         );
      } else {
         ptwOut->print(sFormat, m_pInvalid, m_pMin, m_pMax);
      }
   }
}

}} //namespace abc::collections

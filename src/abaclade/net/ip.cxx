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
#include <abaclade/net/ip.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip {

static_assert(sizeof(address::v4_type) <= sizeof(detail::raw_address::m_ab), "v4_type is too big");
static_assert(sizeof(address::v6_type) <= sizeof(detail::raw_address::m_ab), "v6_type is too big");

static detail::raw_address const gc_abAny4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v4
};
static detail::raw_address const gc_abAny6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v6
};

address const & address::any_v4 = static_cast<address const &>(gc_abAny4);
address const & address::any_v6 = static_cast<address const &>(gc_abAny6);

}}} //namespace abc::net::ip

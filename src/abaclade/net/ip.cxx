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

#if ABC_HOST_API_POSIX
   //#include <sys/types.h> // sockaddr sockaddr_in
   //#if ABC_HOST_API_FREEBSD
   //   #include <netinet/in.h>
   //#endif
#elif ABC_HOST_API_WIN32
   //#include <winsock2.h>
   //#if ABC_HOST_CXX_MSC
   //   // Silence warnings from system header files.
   //   #pragma warning(push)
   //
   //   // “'id' : conversion from 'type1' to 'type2', signed / unsigned mismatch”
   //   #pragma warning(disable: 4365)
   //#endif
   //#include <ws2tcpip.h>
   //#include <mstcpip.h>
   //#if ABC_HOST_CXX_MSC
   //   #pragma warning(pop)
   //#endif
   //#if _WIN32_WINNT == 0x0500
   //   // Additional header required for Windows 2000 IPv6 Tech Preview.
   //   #include <tpipv6.h>
   //#endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip {

static_assert(sizeof(address::v4_type) <= sizeof(detail::raw_address::m_ab), "v4_type is too big");
static_assert(sizeof(address::v6_type) <= sizeof(detail::raw_address::m_ab), "v6_type is too big");

static detail::raw_address const gc_abAny4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 4
};
static detail::raw_address const gc_abAny6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 6
};

address const & address::any_v4 = static_cast<address const &>(gc_abAny4);
address const & address::any_v6 = static_cast<address const &>(gc_abAny6);

}}} //namespace abc::net::ip

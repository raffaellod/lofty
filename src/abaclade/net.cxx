/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
#include <abaclade/net.hxx>

#if ABC_HOST_API_POSIX
   #include <arpa/inet.h> // inet_addr()
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // accept() bind() socket()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::tcp_server

namespace abc {
namespace net {

tcp_server::tcp_server(istr const & sAddress, std::uint16_t iPort, unsigned cBacklog /*= 5*/) :
   m_fdSocket(::socket(AF_INET, SOCK_STREAM, 0)) {
   ABC_TRACE_FUNC(sAddress, iPort, cBacklog);

   if (!m_fdSocket) {
      exception::throw_os_error();
   }

   ::sockaddr_in saServer;
   memory::clear(&saServer);
   saServer.sin_family = AF_INET;
   if (sAddress == ABC_SL("*")) {
      saServer.sin_addr.s_addr = INADDR_ANY;
   } else {
      saServer.sin_addr.s_addr = ::inet_addr(sAddress.c_str());
      if (saServer.sin_addr.s_addr == ::in_addr_t(-1)) {
         // TODO: FIXME: ::inet_addr() doesn’t set errno!
         exception::throw_os_error();
      }
   }
   saServer.sin_port = ::htons(iPort);
   if (::bind(m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saServer), sizeof saServer) < 0) {
      exception::throw_os_error();
   }

   if (::listen(m_fdSocket.get(), static_cast<int>(cBacklog)) < 0) {
      exception::throw_os_error();
   }
}

tcp_server::~tcp_server() {
}

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

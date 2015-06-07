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
#include <abaclade/coroutine.hxx>
#include <abaclade/net.hxx>
#include <abaclade/thread.hxx>

#if ABC_HOST_API_POSIX
   #include <arpa/inet.h> // inet_addr()
   #include <errno.h> // EINTR errno
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // accept4() bind() socket()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::ip_address

namespace abc {
namespace net {

static detail::raw_ip_address const gc_abAny4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 4
};
static detail::raw_ip_address const gc_abAny6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 6
};

ip_address const & ip_address::any_ipv4 = static_cast<ip_address const &>(gc_abAny4);
ip_address const & ip_address::any_ipv6 = static_cast<ip_address const &>(gc_abAny6);

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::connection

namespace abc {
namespace net {

connection::connection(io::filedesc fd, ip_address && ipaddrRemote, port_t portRemote) :
   m_bfrw(io::binary::make_readwriter(std::move(fd))),
   m_ipaddrRemote(std::move(ipaddrRemote)),
   m_portRemote(portRemote) {
}

connection::~connection() {
}

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::tcp_server

namespace abc {
namespace net {

tcp_server::tcp_server(ip_address const & ipaddr, port_t port, unsigned cBacklog /*= 5*/) :
   m_fdSocket(create_socket(ipaddr.version())),
   m_iIPVersion(ipaddr.version()) {
   ABC_TRACE_FUNC(this/*, ipaddr*/, port, cBacklog);

#if ABC_HOST_API_POSIX
   ::sockaddr * psaServer;
   ::socklen_t cbServer;
   ::sockaddr_in saServer4;
   ::sockaddr_in6 saServer6;
   switch (m_iIPVersion) {
      case 4:
         psaServer = reinterpret_cast< ::sockaddr *>(&saServer4);
         cbServer = sizeof saServer4;
         memory::clear(&saServer4);
         saServer4.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&saServer4.sin_addr.s_addr),
            ipaddr.raw(),
            sizeof(ip_address::ipv4_type)
         );
         saServer4.sin_addr.s_addr = htonl(saServer4.sin_addr.s_addr);
         saServer4.sin_port = htons(port);
         break;
      case 6:
         psaServer = reinterpret_cast< ::sockaddr *>(&saServer6);
         cbServer = sizeof saServer6;
         memory::clear(&saServer6);
         //saServer6.sin6_flowinfo = 0;
         saServer6.sin6_family = AF_INET6;
         memory::copy(saServer6.sin6_addr.s6_addr, ipaddr.raw(), sizeof(ip_address::ipv6_type));
         saServer6.sin6_port = htons(port);
         break;
   }
   if (::bind(m_fdSocket.get(), psaServer, cbServer) < 0) {
      exception::throw_os_error();
   }
   if (::listen(m_fdSocket.get(), static_cast<int>(cBacklog)) < 0) {
      exception::throw_os_error();
   }
#else //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … else
}

tcp_server::~tcp_server() {
}

std::shared_ptr<connection> tcp_server::accept() {
   ABC_TRACE_FUNC(this);

   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
#if ABC_HOST_API_POSIX
   ::sockaddr * psaClient;
   ::socklen_t cbClient;
   ::sockaddr_in saClient4;
   ::sockaddr_in6 saClient6;
   if (m_iIPVersion == 4) {
      psaClient = reinterpret_cast< ::sockaddr *>(&saClient4);
      cbClient = sizeof saClient4;
   } else {
      psaClient = reinterpret_cast< ::sockaddr *>(&saClient6);
      cbClient = sizeof saClient6;
   }
   int iFd;
#if !ABC_HOST_API_DARWIN
   int iFlags = SOCK_CLOEXEC;
   if (bAsync) {
      // Using coroutines, so make the client socket non-blocking.
      iFlags |= SOCK_NONBLOCK;
   }
#endif
   for (;;) {
      ::socklen_t cb = cbClient;
#if ABC_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      iFd = ::accept(m_fdSocket.get(), psaClient, &cb);
#else
      iFd = ::accept4(m_fdSocket.get(), psaClient, &cb, iFlags);
#endif
      if (iFd >= 0) {
         cbClient = cb;
         break;
      }
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            // Wait for m_fdSocket. Accepting a connection is considered a read event.
            this_coroutine::sleep_until_fd_ready(m_fdSocket, false);
            break;
         default:
            exception::throw_os_error(iErr);
      }
   }
   io::filedesc fd(iFd);
#if ABC_HOST_API_DARWIN
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread
   won’t leak the file descriptor. That’s the whole point of accept4(). */
   fd.set_close_on_exec(true);
   if (bAsync) {
      fd.set_nonblocking(true);
   }
#endif
   ip_address ipaddrClient;
   port_t portClient;
   // TODO: validate cbClient and set ipaddrClient/portClient using saClient4/6.sin_addr.
   ipaddrClient = ip_address(0);
   portClient = 0;
   return std::make_shared<connection>(std::move(fd), std::move(ipaddrClient), portClient);

#else //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … else
}

/*static*/ io::filedesc tcp_server::create_socket(std::uint8_t iIPVersion) {
   ABC_TRACE_FUNC(iIPVersion);

   if (iIPVersion != 4 && iIPVersion != 6) {
      // TODO: provide more information in the exception.
      ABC_THROW(domain_error, ());
   }
   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
#if ABC_HOST_API_POSIX
   int iType = SOCK_STREAM;
#if !ABC_HOST_API_DARWIN
   iType |= SOCK_CLOEXEC;
   if (bAsync) {
      // Using coroutines, so make this socket non-blocking.
      iType |= SOCK_NONBLOCK;
   }
#endif
   io::filedesc fd(::socket(iIPVersion == 4 ? AF_INET : AF_INET6, iType, 0));
   if (!fd) {
      exception::throw_os_error();
   }
#if ABC_HOST_API_DARWIN
   /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread
   won’t leak the file descriptor. That’s the whole point of the extra SOCK_* flags. */
   fd.set_close_on_exec(true);
   if (bAsync) {
      fd.set_nonblocking(true);
   }
#endif
#else //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … else
   return std::move(fd);
}

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

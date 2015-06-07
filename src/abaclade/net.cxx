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

tcp_server::tcp_server(
   ip_address const & ipaddr, port_t port, unsigned cBacklog /*= 5*/
) :
   m_fdSocket(create_socket()),
   m_iIPVersion(ipaddr.version()) {
   ABC_TRACE_FUNC(this/*, ipaddr*/, port, cBacklog);

   switch (m_iIPVersion) {
      case 4: {
#if ABC_HOST_API_POSIX
         ::sockaddr_in saServer;
         memory::clear(&saServer);
         saServer.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&saServer.sin_addr.s_addr),
            ipaddr.raw(),
            sizeof(ip_address::ipv4_type)
         );
         saServer.sin_addr.s_addr = htonl(saServer.sin_addr.s_addr);
         saServer.sin_port = htons(port);
         if (::bind(
            m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saServer), sizeof saServer
         ) < 0) {
            exception::throw_os_error();
         }

         if (::listen(m_fdSocket.get(), static_cast<int>(cBacklog)) < 0) {
            exception::throw_os_error();
         }
#else //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … else
         break;
      }
   }
}

tcp_server::~tcp_server() {
}

std::shared_ptr<connection> tcp_server::accept() {
   ABC_TRACE_FUNC(this);

   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
#if ABC_HOST_API_POSIX
   ::sockaddr_in saClient;
   ::socklen_t cbClient;
   int iFd;
#if !ABC_HOST_API_DARWIN
   int iFlags = SOCK_CLOEXEC;
   if (bAsync) {
      // Using coroutines, so make the client socket non-blocking.
      iFlags |= SOCK_NONBLOCK;
   }
#endif
   for (;;) {
      cbClient = sizeof saClient;
#if ABC_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      iFd = ::accept(m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saClient), &cbClient);
#else
      iFd = ::accept4(
         m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saClient), &cbClient, iFlags
      );
#endif
      if (iFd >= 0) {
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
   // TODO: render saClient.sin_addr into ipaddrClient and portClient.
   ipaddrClient = ip_address(0);
   portClient = 0;
   return std::make_shared<connection>(std::move(fd), std::move(ipaddrClient), portClient);

#else //if ABC_HOST_API_POSIX
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … else
}

/*static*/ io::filedesc tcp_server::create_socket() {
   ABC_TRACE_FUNC();

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
   io::filedesc fd(::socket(AF_INET, iType, 0));
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

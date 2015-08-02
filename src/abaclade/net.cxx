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
   #if ABC_HOST_API_FREEBSD
      #include <netinet/in.h>
   #endif
#elif ABC_HOST_API_WIN32
   #include <winsock2.h>
   #if ABC_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)

      // “'id' : conversion from 'type1' to 'type2', signed / unsigned mismatch”
      #pragma warning(disable: 4365)
   #endif
   #include <ws2tcpip.h>
   #include <mstcpip.h>
   #if ABC_HOST_CXX_MSC
      #pragma warning(pop)
   #endif
   #if _WIN32_WINNT == 0x0500
      // Additional header required for Windows 2000 IPv6 Tech Preview.
      #include <tpipv6.h>
   #endif
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

static detail::raw_ip_address const gc_abAny4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 4
};
static detail::raw_ip_address const gc_abAny6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, 6
};

ip_address const & ip_address::any_ipv4 = static_cast<ip_address const &>(gc_abAny4);
ip_address const & ip_address::any_ipv6 = static_cast<ip_address const &>(gc_abAny6);

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

connection::connection(io::filedesc fd, ip_address && ipaddrRemote, port_t portRemote) :
   m_bfrw(io::binary::make_readwriter(std::move(fd))),
   m_ipaddrRemote(std::move(ipaddrRemote)),
   m_portRemote(portRemote) {
}

connection::~connection() {
}

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

tcp_server::tcp_server(ip_address const & ipaddr, port_t port, unsigned cBacklog /*= 5*/) :
   m_fdSocket(create_socket(ipaddr.version())),
   m_iIPVersion(ipaddr.version()) {
   ABC_TRACE_FUNC(this/*, ipaddr*/, port, cBacklog);

#if ABC_HOST_API_POSIX
   typedef ::sockaddr sockaddr_type;
   typedef ::socklen_t socklen_type;
#elif ABC_HOST_API_WIN32
   typedef ::SOCKADDR sockaddr_type;
   typedef int socklen_type;
#else
   #error "TODO: HOST_API"
#endif
   sockaddr_type * psaServer;
   socklen_type cbServer;
   ::sockaddr_in saServer4;
   ::sockaddr_in6 saServer6;
   switch (m_iIPVersion) {
      case 4:
         psaServer = reinterpret_cast<sockaddr_type *>(&saServer4);
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
         psaServer = reinterpret_cast<sockaddr_type *>(&saServer6);
         cbServer = sizeof saServer6;
         memory::clear(&saServer6);
         //saServer6.sin6_flowinfo = 0;
         saServer6.sin6_family = AF_INET6;
         memory::copy(saServer6.sin6_addr.s6_addr, ipaddr.raw(), sizeof(ip_address::ipv6_type));
         saServer6.sin6_port = htons(port);
         break;
      ABC_SWITCH_WITHOUT_DEFAULT
   }
#if ABC_HOST_API_WIN32
   if (::bind(reinterpret_cast< ::SOCKET>(m_fdSocket.get()), psaServer, cbServer) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
   if (::listen(reinterpret_cast< ::SOCKET>(m_fdSocket.get()), static_cast<int>(cBacklog)) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (::bind(m_fdSocket.get(), psaServer, cbServer) < 0) {
      exception::throw_os_error();
   }
   if (::listen(m_fdSocket.get(), static_cast<int>(cBacklog)) < 0) {
      exception::throw_os_error();
   }
#endif
}

tcp_server::~tcp_server() {
#if ABC_HOST_API_WIN32
   ::WSACleanup();
#endif
}

std::shared_ptr<connection> tcp_server::accept() {
   ABC_TRACE_FUNC(this);

#if ABC_HOST_API_POSIX
   typedef ::sockaddr sockaddr_type;
   typedef ::socklen_t socklen_type;
   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
#elif ABC_HOST_API_WIN32
   typedef ::SOCKADDR sockaddr_type;
   typedef int socklen_type;
#else
   #error "TODO: HOST_API"
#endif
   io::filedesc fd;
   sockaddr_type * psaClient;
   socklen_type cbClient;
   ::sockaddr_in saClient4;
   ::sockaddr_in6 saClient6;
   if (m_iIPVersion == 4) {
      psaClient = reinterpret_cast<sockaddr_type *>(&saClient4);
      cbClient = sizeof saClient4;
   } else {
      psaClient = reinterpret_cast<sockaddr_type *>(&saClient6);
      cbClient = sizeof saClient6;
   }
   for (;;) {
      socklen_type cb = cbClient;
#if ABC_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      fd = io::filedesc(::accept(m_fdSocket.get(), psaClient, &cb));
      if (fd) {
         /* Note that at this point there’s no hack that will ensure a fork()/exec() from another
         thread won’t leak the file descriptor. That’s the whole point of accept4(). */
         fd.set_close_on_exec(true);
         if (bAsync) {
            fd.set_nonblocking(true);
         }
      }
#elif ABC_HOST_API_POSIX
      int iFlags = SOCK_CLOEXEC;
      if (bAsync) {
         // Using coroutines, so make the client socket non-blocking.
         iFlags |= SOCK_NONBLOCK;
      }
      fd = io::filedesc(::accept4(m_fdSocket.get(), psaClient, &cb, iFlags));
#elif ABC_HOST_API_WIN32
      fd = io::filedesc(reinterpret_cast<io::filedesc_t>(::WSAAccept(
         reinterpret_cast< ::SOCKET>(m_fdSocket.get()), psaClient, &cb, nullptr, 0
      )));
#else
   #error "TODO: HOST_API"
#endif
      if (fd) {
         cbClient = cb;
         break;
      }
#if ABC_HOST_API_POSIX
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            // Check for pending interruptions.
            this_thread::interruption_point();
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            // Wait for m_fdSocket. Accepting a connection is considered a read event.
            this_coroutine::sleep_until_fd_ready(m_fdSocket.get(), false);
#elif ABC_HOST_API_WIN32
      int iErr = ::WSAGetLastError();
      switch (iErr) {
         case WSAEWOULDBLOCK:
            {
               // Wait for m_fdSocket. Accepting a connection is considered a read event.
               // TODO: track hIocp.
               ::HANDLE hIocp = nullptr;
               this_coroutine::sleep_until_fd_ready(m_fdSocket.get(), false, &hIocp);
            }
#else
   #error "TODO: HOST_API"
#endif
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(iErr));
      }
   }
   // Check for pending interruptions.
   this_thread::interruption_point();

   ip_address ipaddrClient;
   port_t portClient;
   // TODO: validate cbClient and set ipaddrClient/portClient using saClient4/6.sin_addr.
   ipaddrClient = ip_address(0);
   portClient = 0;
   return std::make_shared<connection>(std::move(fd), std::move(ipaddrClient), portClient);
}

/*static*/ io::filedesc tcp_server::create_socket(std::uint8_t iIPVersion) {
   ABC_TRACE_FUNC(iIPVersion);

   if (iIPVersion != 4 && iIPVersion != 6) {
      // TODO: provide more information in the exception.
      ABC_THROW(domain_error, ());
   }
   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
   int iFamily = (iIPVersion == 4 ? AF_INET : AF_INET6);
   int iType = SOCK_STREAM;
#if ABC_HOST_API_POSIX
   #if !ABC_HOST_API_DARWIN
      iType |= SOCK_CLOEXEC;
      if (bAsync) {
         // Using coroutines, so make this socket non-blocking.
         iType |= SOCK_NONBLOCK;
      }
   #endif
   io::filedesc fd(::socket(iFamily, iType, 0));
   if (!fd) {
      exception::throw_os_error();
   }
   #if ABC_HOST_API_DARWIN
      /* Note that at this point there’s no hack that will ensure a fork()/exec() from another
      thread won’t leak the file descriptor. That’s the whole point of the extra SOCK_* flags. */
      fd.set_close_on_exec(true);
      if (bAsync) {
         fd.set_nonblocking(true);
      }
   #endif
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   static std::uint8_t const sc_iWSAMajorVersion = 2, sc_iWSAMinorVersion = 2;
   ::WSADATA wsad;
   if (int iRet = ::WSAStartup(MAKEWORD(sc_iWSAMajorVersion, sc_iWSAMinorVersion), &wsad)) {
      exception::throw_os_error(static_cast<errint_t>(iRet));
   }
   if (
      LOBYTE(wsad.wVersion) != sc_iWSAMajorVersion || HIBYTE(wsad.wVersion) != sc_iWSAMinorVersion
   ) {
      // The loaded WinSock implementation does not support the requested version.
      ::WSACleanup();
      // TODO: use a better exception class.
      ABC_THROW(generic_error, ());
   }

   ::DWORD iFlags = 0;
   if (bAsync) {
      iFlags |= WSA_FLAG_OVERLAPPED;
   }
   #ifdef WSA_FLAG_NO_HANDLE_INHERIT
      iFlags |= WSA_FLAG_NO_HANDLE_INHERIT;
   #endif
   io::filedesc fd(reinterpret_cast<io::filedesc_t>(::WSASocket(
      iFamily, iType, 0, nullptr, 0, iFlags
   )));
   if (!fd) {
      exception::throw_os_error();
   }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   return std::move(fd);
}

}} //namespace abc::net

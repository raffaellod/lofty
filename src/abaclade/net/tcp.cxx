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
#include <abaclade/coroutine.hxx>
#include <abaclade/net/tcp.hxx>
#include <abaclade/thread.hxx>

#if ABC_HOST_API_POSIX
   #include <arpa/inet.h> // inet_addr()
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // htons() ntohs()
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // accept4() bind() getsockname() socket()
#elif ABC_HOST_API_WIN32
   #include <winsock2.h>
   #include <mswsock.h> // AcceptEx() GetAcceptExSockaddrs()
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

namespace abc { namespace net { namespace tcp {

connection::connection(
   io::filedesc fd, ip::address && addrLocal, ip::port && portLocal, ip::address && addrRemote,
   ip::port && portRemote
) :
   m_bfrw(io::binary::make_readwriter(_std::move(fd))),
   m_addrLocal(_std::move(addrLocal)),
   m_portLocal(_std::move(portLocal)),
   m_addrRemote(_std::move(addrRemote)),
   m_portRemote(_std::move(portRemote)) {
}

connection::~connection() {
}

}}} //namespace abc::net::tcp

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace tcp {

union sockaddr_any {
   ::sockaddr_in sa4;
   ::sockaddr_in6 sa6;
};

server::server(ip::address const & addr, ip::port const & port, unsigned cBacklog /*= 5*/) :
   m_fdSocket(create_socket(addr.version())),
   m_ipv(addr.version()) {
   ABC_TRACE_FUNC(this, addr, port, cBacklog);

#if ABC_HOST_API_POSIX
   ::socklen_t cbServerSockAddr;
#elif ABC_HOST_API_WIN32
   int cbServerSockAddr;
#else
   #error "TODO: HOST_API"
#endif
   sockaddr_any saaServer;
   switch (m_ipv.base()) {
      case ip::version::v4:
         cbServerSockAddr = sizeof saaServer.sa4;
         memory::clear(&saaServer.sa4);
         saaServer.sa4.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&saaServer.sa4.sin_addr.s_addr),
            addr.raw(),
            sizeof saaServer.sa4.sin_addr.s_addr
         );
         saaServer.sa4.sin_port = htons(port.number());
         break;
      case ip::version::v6:
         cbServerSockAddr = sizeof saaServer.sa6;
         memory::clear(&saaServer.sa6);
         //saaServer.sa6.sin6_flowinfo = 0;
         saaServer.sa6.sin6_family = AF_INET6;
         memory::copy(
            &saaServer.sa6.sin6_addr.s6_addr[0], addr.raw(), sizeof saaServer.sa6.sin6_addr.s6_addr
         );
         saaServer.sa6.sin6_port = htons(port.number());
         break;
      ABC_SWITCH_WITHOUT_DEFAULT
   }
#if ABC_HOST_API_WIN32
   if (
      ::bind(
         reinterpret_cast< ::SOCKET>(m_fdSocket.get()),
         reinterpret_cast< ::SOCKADDR *>(&saaServer), cbServerSockAddr
      ) < 0 ||
      ::listen(reinterpret_cast< ::SOCKET>(m_fdSocket.get()), static_cast<int>(cBacklog)) < 0
   ) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (
      ::bind(m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saaServer), cbServerSockAddr) < 0 ||
      ::listen(m_fdSocket.get(), static_cast<int>(cBacklog)) < 0
   ) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
#if ABC_HOST_API_WIN32
   ::WSACleanup();
#endif
}

_std::shared_ptr<connection> server::accept() {
   ABC_TRACE_FUNC(this);

   io::filedesc fdConnection;
   sockaddr_any * psaaLocal, * psaaRemote;
#if ABC_HOST_API_POSIX
   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
   sockaddr_any saaLocal, saaRemote;
   psaaLocal = &saaLocal;
   psaaRemote = &saaRemote;
   ::socklen_t cbRemoteSockAddr;
   switch (m_ipv.base()) {
      case ip::version::v4:
         cbRemoteSockAddr = sizeof saaRemote.sa4;
         break;
      case ip::version::v6:
         cbRemoteSockAddr = sizeof saaRemote.sa6;
         break;
      ABC_SWITCH_WITHOUT_DEFAULT
   }
   ::socklen_t cbLocalSockAddr = cbRemoteSockAddr;
   for (;;) {
      ::socklen_t cb = cbRemoteSockAddr;
   #if ABC_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      fdConnection = io::filedesc(
         ::accept(m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saaRemote), &cb)
      );
      if (fdConnection) {
         /* Note that at this point there’s no hack that will ensure a fork()/exec() from another
         thread won’t leak the file descriptor. That’s the whole point of accept4(). */
         fdConnection.set_close_on_exec(true);
         if (bAsync) {
            fdConnection.set_nonblocking(true);
         }
      }
   #else
      int iFlags = SOCK_CLOEXEC;
      if (bAsync) {
         // Using coroutines, so make the client socket non-blocking.
         iFlags |= SOCK_NONBLOCK;
      }
      fdConnection = io::filedesc(
         ::accept4(m_fdSocket.get(), reinterpret_cast< ::sockaddr *>(&saaRemote), &cb, iFlags)
      );
   #endif
      if (fdConnection) {
         cbRemoteSockAddr = cb;
         break;
      }
      int iErr = errno;
      switch (iErr) {
         case EINTR:
            this_coroutine::interruption_point();
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            // Wait for m_fdSocket. Accepting a connection is considered a read event.
            this_coroutine::sleep_until_fd_ready(m_fdSocket.get(), false);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(iErr));
      }
   }
   ::getsockname(fdConnection.get(), reinterpret_cast< ::sockaddr *>(&saaLocal), &cbLocalSockAddr);
#elif ABC_HOST_API_WIN32
   // ::AcceptEx() expects a weird and under-documented buffer of which we only know the size.
   static ::DWORD const sc_cbSockAddrBuf = sizeof(sockaddr_any) + 16;
   std::int8_t abBuf[sc_cbSockAddrBuf * 2];

   fdConnection = create_socket(m_ipv);
   ::DWORD cbRead;
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   m_fdSocket.bind_to_this_coroutine_scheduler_iocp();
   if (!::AcceptEx(
      reinterpret_cast< ::SOCKET>(m_fdSocket.get()),
      reinterpret_cast< ::SOCKET>(fdConnection.get()),
      abBuf, 0 /*don’t wait for data, just wait for a connection*/,
      sc_cbSockAddrBuf, sc_cbSockAddrBuf, &cbRead, &ovl
   )) {
      ::DWORD iErr = static_cast< ::DWORD>(::WSAGetLastError());
      if (iErr == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(m_fdSocket.get(), false, &ovl);
         iErr = ovl.status();
         cbRead = ovl.transferred_size();
      }
      if (iErr != ERROR_SUCCESS) {
         exception::throw_os_error(iErr);
      }
   }

   // Parse the weird buffer.
   int cbRemoteSockAddr, cbLocalSockAddr;
   ::GetAcceptExSockaddrs(
      abBuf, 0 /*no other data was read*/, sc_cbSockAddrBuf, sc_cbSockAddrBuf,
      reinterpret_cast< ::SOCKADDR **>(&psaaLocal), &cbLocalSockAddr,
      reinterpret_cast< ::SOCKADDR **>(&psaaRemote), &cbRemoteSockAddr
   );
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   ip::address addrLocal, addrRemote;
   ip::port portLocal, portRemote;
   switch (m_ipv.base()) {
      case ip::version::v4:
         if (cbLocalSockAddr == sizeof(sockaddr_any::sa4)) {
            addrLocal = ip::address(
               *reinterpret_cast<ip::address::v4_type *>(&psaaLocal->sa4.sin_addr.s_addr)
            );
            portLocal = ip::port(ntohs(psaaLocal->sa4.sin_port));
         }
         if (cbRemoteSockAddr == sizeof(sockaddr_any::sa4)) {
            addrRemote = ip::address(
               *reinterpret_cast<ip::address::v4_type *>(&psaaRemote->sa4.sin_addr.s_addr)
            );
            portRemote = ip::port(ntohs(psaaRemote->sa4.sin_port));
         }
         break;
      case ip::version::v6:
         if (cbLocalSockAddr == sizeof(sockaddr_any::sa6)) {
            addrLocal = ip::address(
               *reinterpret_cast<ip::address::v6_type *>(&psaaLocal->sa6.sin6_addr.s6_addr)
            );
            portLocal = ip::port(ntohs(psaaLocal->sa6.sin6_port));
         }
         if (cbRemoteSockAddr == sizeof(sockaddr_any::sa6)) {
            addrRemote = ip::address(
               *reinterpret_cast<ip::address::v6_type *>(&psaaRemote->sa6.sin6_addr.s6_addr)
            );
            portRemote = ip::port(ntohs(psaaRemote->sa6.sin6_port));
         }
         break;
      ABC_SWITCH_WITHOUT_DEFAULT
   }
   return _std::make_shared<connection>(
      _std::move(fdConnection), _std::move(addrLocal), _std::move(portLocal),
      _std::move(addrRemote), _std::move(portRemote)
   );
}

/*static*/ io::filedesc server::create_socket(ip::version ipv) {
   ABC_TRACE_FUNC(ipv);

   if (ipv == ip::version::any) {
      // TODO: provide more information in the exception.
      ABC_THROW(domain_error, ());
   }
   bool bAsync = (this_thread::coroutine_scheduler() != nullptr);
   int iFamily;
   switch (ipv.base()) {
      case ip::version::v4:
         iFamily = AF_INET;
         break;
      case ip::version::v6:
         iFamily = AF_INET6;
         break;
      ABC_SWITCH_WITHOUT_DEFAULT
   }
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
   return _std::move(fd);
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
   ::SOCKET sock = ::WSASocket(iFamily, iType, 0, nullptr, 0, iFlags);
   if (sock == INVALID_SOCKET) {
      exception::throw_os_error();
   }
   return io::filedesc(reinterpret_cast<io::filedesc_t>(sock));
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
}

}}} //namespace abc::net::tcp

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/net/tcp.hxx>
#include <lofty/thread.hxx>

#if LOFTY_HOST_API_POSIX
   #include <arpa/inet.h> // inet_addr()
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // htons() ntohs()
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // accept4() bind() getsockname() socket()
#elif LOFTY_HOST_API_WIN32
   #include <winsock2.h>
   #include <mswsock.h> // AcceptEx() GetAcceptExSockaddrs()
   #if LOFTY_HOST_CXX_MSC
      // Silence warnings from system header files.
      #pragma warning(push)

      // “'id' : conversion from 'type1' to 'type2', signed / unsigned mismatch”
      #pragma warning(disable: 4365)
   #endif
   #include <ws2tcpip.h>
   #include <mstcpip.h>
   #if LOFTY_HOST_CXX_MSC
      #pragma warning(pop)
   #endif
   #if _WIN32_WINNT == 0x0500
      // Additional header required for Windows 2000 IPv6 Tech Preview.
      #include <tpipv6.h>
   #endif
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {

connection::connection(
   io::filedesc fd, ip::address && local_address__, ip::port && local_port__, ip::address && remote_address__,
   ip::port && remote_port__
) :
   socket_(io::binary::make_iostream(_std::move(fd))),
   local_address_(_std::move(local_address__)),
   local_port_(_std::move(local_port__)),
   remote_address_(_std::move(remote_address__)),
   remote_port_(_std::move(remote_port__)) {
}

connection::~connection() {
}

}}} //namespace lofty::net::tcp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {

union sockaddr_any {
   ::sockaddr_in sa4;
   ::sockaddr_in6 sa6;
};

server::server(ip::address const & address, ip::port const & port, unsigned backlog_size /*= 5*/) :
   sock_fd(create_socket(address.version())),
   ip_version(address.version()) {
   LOFTY_TRACE_FUNC(this, address, port, backlog_size);

#if LOFTY_HOST_API_POSIX
   ::socklen_t server_sock_addr_size;
#elif LOFTY_HOST_API_WIN32
   int server_sock_addr_size;
#else
   #error "TODO: HOST_API"
#endif
   sockaddr_any server_sockaddr;
   switch (ip_version.base()) {
      case ip::version::v4:
         server_sock_addr_size = sizeof server_sockaddr.sa4;
         memory::clear(&server_sockaddr.sa4);
         server_sockaddr.sa4.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&server_sockaddr.sa4.sin_addr.s_addr), address.raw(),
            sizeof server_sockaddr.sa4.sin_addr.s_addr
         );
         server_sockaddr.sa4.sin_port = htons(port.number());
         break;
      case ip::version::v6:
         server_sock_addr_size = sizeof server_sockaddr.sa6;
         memory::clear(&server_sockaddr.sa6);
         //server_sockaddr.sa6.sin6_flowinfo = 0;
         server_sockaddr.sa6.sin6_family = AF_INET6;
         memory::copy(
            &server_sockaddr.sa6.sin6_addr.s6_addr[0], address.raw(),
            sizeof server_sockaddr.sa6.sin6_addr.s6_addr
         );
         server_sockaddr.sa6.sin6_port = htons(port.number());
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
#if LOFTY_HOST_API_WIN32
   if (
      ::bind(
         reinterpret_cast< ::SOCKET>(sock_fd.get()),
         reinterpret_cast< ::SOCKADDR *>(&server_sockaddr), server_sock_addr_size
      ) < 0 ||
      ::listen(reinterpret_cast< ::SOCKET>(sock_fd.get()), static_cast<int>(backlog_size)) < 0
   ) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (
      ::bind(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&server_sockaddr), server_sock_addr_size) < 0 ||
      ::listen(sock_fd.get(), static_cast<int>(backlog_size)) < 0
   ) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
#if LOFTY_HOST_API_WIN32
   ::WSACleanup();
#endif
}

_std::shared_ptr<connection> server::accept() {
   LOFTY_TRACE_FUNC(this);

   io::filedesc conn_fd;
   sockaddr_any * local_sa_ptr, * remote_sa_ptr;
#if LOFTY_HOST_API_POSIX
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   sockaddr_any local_sa, remote_sa;
   local_sa_ptr = &local_sa;
   remote_sa_ptr = &remote_sa;
   ::socklen_t remote_sock_addr_size;
   switch (ip_version.base()) {
      case ip::version::v4:
         remote_sock_addr_size = sizeof remote_sa.sa4;
         break;
      case ip::version::v6:
         remote_sock_addr_size = sizeof remote_sa.sa6;
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   ::socklen_t local_sock_addr_size = remote_sock_addr_size;
   for (;;) {
      ::socklen_t addr_size = remote_sock_addr_size;
   #if LOFTY_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      conn_fd = io::filedesc(
         ::accept(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&remote_sa), &addr_size)
      );
      if (conn_fd) {
         /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t
         leak the file descriptor. That’s the whole point of accept4(). */
         conn_fd.set_close_on_exec(true);
         if (async) {
            conn_fd.set_nonblocking(true);
         }
      }
   #else
      int flags = SOCK_CLOEXEC;
      if (async) {
         // Using coroutines, so make the client socket non-blocking.
         flags |= SOCK_NONBLOCK;
      }
      conn_fd = io::filedesc(
         ::accept4(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&remote_sa), &addr_size, flags)
      );
   #endif
      if (conn_fd) {
         remote_sock_addr_size = addr_size;
         break;
      }
      auto err = errno;
      switch (err) {
         case EINTR:
            this_coroutine::interruption_point();
            break;
         case EAGAIN:
   #if EWOULDBLOCK != EAGAIN
         case EWOULDBLOCK:
   #endif
            // Wait for sock_fd. Accepting a connection is considered a read event.
            this_coroutine::sleep_until_fd_ready(sock_fd.get(), false);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
   ::getsockname(conn_fd.get(), reinterpret_cast< ::sockaddr *>(&local_sa), &local_sock_addr_size);
#elif LOFTY_HOST_API_WIN32
   // ::AcceptEx() expects a weird and under-documented buffer of which we only know the size.
   static ::DWORD const sock_addr_buf_size = sizeof(sockaddr_any) + 16;
   std::int8_t sock_addr_buf[sock_addr_buf_size * 2];

   conn_fd = create_socket(ip_version);
   ::DWORD bytes_read;
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   sock_fd.bind_to_this_coroutine_scheduler_iocp();
   if (!::AcceptEx(
      reinterpret_cast< ::SOCKET>(sock_fd.get()), reinterpret_cast< ::SOCKET>(conn_fd.get()),
      sock_addr_buf, 0 /*don’t wait for data, just wait for a connection*/,
      sock_addr_buf_size, sock_addr_buf_size, &bytes_read, &ovl
   )) {
      auto err = static_cast< ::DWORD>(::WSAGetLastError());
      if (err == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(sock_fd.get(), false, &ovl);
         err = ovl.status();
         bytes_read = ovl.transferred_size();
      }
      if (err != ERROR_SUCCESS) {
         exception::throw_os_error(err);
      }
   }

   // Parse the weird buffer.
   int remote_sock_addr_size, local_sock_addr_size;
   ::GetAcceptExSockaddrs(
      sock_addr_buf, 0 /*no other data was read*/, sock_addr_buf_size, sock_addr_buf_size,
      reinterpret_cast< ::SOCKADDR **>(&local_sa_ptr), &local_sock_addr_size,
      reinterpret_cast< ::SOCKADDR **>(&remote_sa_ptr), &remote_sock_addr_size
   );
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   ip::address local_addr, remote_address;
   ip::port local_port, remote_port;
   switch (ip_version.base()) {
      case ip::version::v4:
         if (local_sock_addr_size == sizeof local_sa_ptr->sa4) {
            local_addr = ip::address(
               *reinterpret_cast<ip::address::v4_type *>(&local_sa_ptr->sa4.sin_addr.s_addr)
            );
            local_port = ip::port(ntohs(local_sa_ptr->sa4.sin_port));
         }
         if (remote_sock_addr_size == sizeof local_sa_ptr->sa4) {
            remote_address = ip::address(
               *reinterpret_cast<ip::address::v4_type *>(&remote_sa_ptr->sa4.sin_addr.s_addr)
            );
            remote_port = ip::port(ntohs(remote_sa_ptr->sa4.sin_port));
         }
         break;
      case ip::version::v6:
         if (local_sock_addr_size == sizeof local_sa_ptr->sa6) {
            local_addr = ip::address(
               *reinterpret_cast<ip::address::v6_type *>(&local_sa_ptr->sa6.sin6_addr.s6_addr)
            );
            local_port = ip::port(ntohs(local_sa_ptr->sa6.sin6_port));
         }
         if (remote_sock_addr_size == sizeof local_sa_ptr->sa6) {
            remote_address = ip::address(
               *reinterpret_cast<ip::address::v6_type *>(&remote_sa_ptr->sa6.sin6_addr.s6_addr)
            );
            remote_port = ip::port(ntohs(remote_sa_ptr->sa6.sin6_port));
         }
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   return _std::make_shared<connection>(
      _std::move(conn_fd), _std::move(local_addr), _std::move(local_port), _std::move(remote_address),
      _std::move(remote_port)
   );
}

/*static*/ io::filedesc server::create_socket(ip::version ip_version_) {
   LOFTY_TRACE_FUNC(ip_version_);

   if (ip_version_ == ip::version::any) {
      // TODO: provide more information in the exception.
      LOFTY_THROW(domain_error, ());
   }
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   int family;
   switch (ip_version_.base()) {
      case ip::version::v4:
         family = AF_INET;
         break;
      case ip::version::v6:
         family = AF_INET6;
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   int type = SOCK_STREAM;
#if LOFTY_HOST_API_POSIX
   #if !LOFTY_HOST_API_DARWIN
      type |= SOCK_CLOEXEC;
      if (async) {
         // Using coroutines, so make this socket non-blocking.
         type |= SOCK_NONBLOCK;
      }
   #endif
   io::filedesc fd(::socket(family, type, 0));
   if (!fd) {
      exception::throw_os_error();
   }
   #if LOFTY_HOST_API_DARWIN
      /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t
      leak the file descriptor. That’s the whole point of the extra SOCK_* flags. */
      fd.set_close_on_exec(true);
      if (async) {
         fd.set_nonblocking(true);
      }
   #endif
   return _std::move(fd);
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   static std::uint8_t const wsa_major_version = 2, wsa_minor_version = 2;
   ::WSADATA wsa_data;
   if (int ret = ::WSAStartup(MAKEWORD(wsa_major_version, wsa_minor_version), &wsa_data)) {
      exception::throw_os_error(static_cast<errint_t>(ret));
   }
   if (LOBYTE(wsa_data.wVersion) != wsa_major_version || HIBYTE(wsa_data.wVersion) != wsa_minor_version) {
      // The loaded WinSock implementation does not support the requested version.
      ::WSACleanup();
      // TODO: use a better exception class.
      LOFTY_THROW(generic_error, ());
   }

   ::DWORD flags = 0;
   if (async) {
      flags |= WSA_FLAG_OVERLAPPED;
   }
   #ifdef WSA_FLAG_NO_HANDLE_INHERIT
      flags |= WSA_FLAG_NO_HANDLE_INHERIT;
   #endif
   ::SOCKET sock = ::WSASocket(family, type, 0, nullptr, 0, flags);
   if (sock == INVALID_SOCKET) {
      exception::throw_os_error();
   }
   return io::filedesc(reinterpret_cast<io::filedesc_t>(sock));
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
}

}}} //namespace lofty::net::tcp

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/net/tcp.hxx>
#include <lofty/thread.hxx>

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // ntohs()
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // accept4() getsockname()
#elif LOFTY_HOST_API_WIN32
   #include <winsock2.h>
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
   #include <mswsock.h> // AcceptEx() GetAcceptExSockaddrs()
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

namespace {

union sockaddr_any {
   ::sockaddr_in sa4;
   ::sockaddr_in6 sa6;
};

} //namespace

server::server(ip::address const & address, ip::port const & port, unsigned backlog_size /*= 5*/) :
   ip::server(address, port, ip::transport::tcp) {
#if LOFTY_HOST_API_WIN32
   if (::listen(reinterpret_cast< ::SOCKET>(sock_fd.get()), static_cast<int>(backlog_size)) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (::listen(sock_fd.get(), static_cast<int>(backlog_size)) < 0) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
}

_std::shared_ptr<connection> server::accept() {
   io::filedesc conn_fd;
   sockaddr_any * local_sock_addr_ptr, * remote_sock_addr_ptr;
#if LOFTY_HOST_API_POSIX
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   sockaddr_any local_sock_addr, remote_sock_addr;
   local_sock_addr_ptr = &local_sock_addr;
   remote_sock_addr_ptr = &remote_sock_addr;
   ::socklen_t remote_sock_addr_size;
   switch (ip_version.base()) {
      case ip::version::v4:
         remote_sock_addr_size = sizeof remote_sock_addr.sa4;
         break;
      case ip::version::v6:
         remote_sock_addr_size = sizeof remote_sock_addr.sa6;
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   ::socklen_t local_sock_addr_size = remote_sock_addr_size;
   for (;;) {
      ::socklen_t addr_size = remote_sock_addr_size;
   #if LOFTY_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      conn_fd = io::filedesc(
         ::accept(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&remote_sock_addr), &addr_size)
      );
      if (conn_fd) {
         /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t
         leak the file descriptor. That’s the whole point of accept4(). */
         conn_fd.share_with_subprocesses(false);
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
         ::accept4(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&remote_sock_addr), &addr_size, flags)
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
            this_coroutine::sleep_until_fd_ready(sock_fd.get(), false /*read*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
   ::getsockname(conn_fd.get(), reinterpret_cast< ::sockaddr *>(&local_sock_addr), &local_sock_addr_size);
#elif LOFTY_HOST_API_WIN32
   // ::AcceptEx() expects a weird and under-documented buffer of which we only know the size.
   static ::DWORD const sock_addr_buf_size = sizeof(sockaddr_any) + 16;
   std::int8_t sock_addr_buf[sock_addr_buf_size * 2];

   conn_fd = ip::create_socket(ip_version, ip::transport::tcp);
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
         this_coroutine::sleep_until_fd_ready(sock_fd.get(), false /*read*/, 0 /*no timeout*/, &ovl);
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
      reinterpret_cast< ::SOCKADDR **>(&local_sock_addr_ptr), &local_sock_addr_size,
      reinterpret_cast< ::SOCKADDR **>(&remote_sock_addr_ptr), &remote_sock_addr_size
   );
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   ip::address local_addr, remote_addr;
   ip::port local_port, remote_port;
   switch (local_sock_addr_size) {
      case sizeof local_sock_addr_ptr->sa4:
         local_addr = ip::address(
            *reinterpret_cast<ip::address::v4_type *>(&local_sock_addr_ptr->sa4.sin_addr.s_addr)
         );
         local_port = ip::port(ntohs(local_sock_addr_ptr->sa4.sin_port));
         break;
      case sizeof local_sock_addr_ptr->sa6:
         local_addr = ip::address(
            *reinterpret_cast<ip::address::v6_type *>(&local_sock_addr_ptr->sa6.sin6_addr.s6_addr)
         );
         local_port = ip::port(ntohs(local_sock_addr_ptr->sa6.sin6_port));
         break;
   }
   switch (remote_sock_addr_size) {
      case sizeof local_sock_addr_ptr->sa4:
         remote_addr = ip::address(
            *reinterpret_cast<ip::address::v4_type *>(&remote_sock_addr_ptr->sa4.sin_addr.s_addr)
         );
         remote_port = ip::port(ntohs(remote_sock_addr_ptr->sa4.sin_port));
         break;
      case sizeof local_sock_addr_ptr->sa6:
         remote_addr = ip::address(
            *reinterpret_cast<ip::address::v6_type *>(&remote_sock_addr_ptr->sa6.sin6_addr.s6_addr)
         );
         remote_port = ip::port(ntohs(remote_sock_addr_ptr->sa6.sin6_port));
         break;
   }
   return _std::make_shared<connection>(
      _std::move(conn_fd), _std::move(local_addr), _std::move(local_port), _std::move(remote_addr),
      _std::move(remote_port)
   );
}

}}} //namespace lofty::net::tcp

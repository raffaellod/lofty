/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

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
#include "sockaddr_any.hxx"

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // ntohs()
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

server::server(ip::address const & address, ip::port const & port, unsigned backlog_size /*= 5*/) :
   ip::server(address, port, address.version() == ip::version::v4 ? protocol::tcp_ipv4 : protocol::tcp_ipv6) {
#if LOFTY_HOST_API_WIN32
   if (::listen(reinterpret_cast< ::SOCKET>(sock.get()), static_cast<int>(backlog_size)) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (::listen(sock.get(), static_cast<int>(backlog_size)) < 0) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
}

_std::shared_ptr<connection> server::accept() {
   socket conn_sock;
   ip::sockaddr_any local_sock_addr, remote_sock_addr;
#if LOFTY_HOST_API_POSIX
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   for (;;) {
      remote_sock_addr.set_size_from_ip_version(ip_version.base());
   #if LOFTY_HOST_API_DARWIN
      // accept4() is not available, so emulate it with accept() + fcntl().
      conn_sock = socket(::accept(sock.get(), remote_sock_addr.sockaddr_ptr(), remote_sock_addr.size_ptr()));
      if (conn_sock) {
         /* Note that at this point there’s no hack that will ensure a fork()/exec() from another thread won’t
         leak the file descriptor. That’s the whole point of accept4(). */
         conn_sock.share_with_subprocesses(false);
         if (async) {
            conn_sock.set_nonblocking(true);
         }
      }
   #else
      int flags = SOCK_CLOEXEC;
      if (async) {
         // Using coroutines, so make the client socket non-blocking.
         flags |= SOCK_NONBLOCK;
      }
      conn_sock = socket(
         ::accept4(sock.get(), remote_sock_addr.sockaddr_ptr(), remote_sock_addr.size_ptr(), flags)
      );
   #endif
      if (conn_sock) {
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
            // Wait for sock. Accepting a connection is considered a read event.
            this_coroutine::sleep_until_fd_ready(sock.get(), false /*read*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
   local_sock_addr.set_size_from_ip_version(ip_version.base());
   ::getsockname(conn_sock.get(), local_sock_addr.sockaddr_ptr(), local_sock_addr.size_ptr());
#elif LOFTY_HOST_API_WIN32
   // ::AcceptEx() expects a weird and under-documented buffer of which we only know the size.
   static ::DWORD const sock_addr_buf_size = sizeof(ip::sockaddr_any) + 16;
   std::int8_t sock_addr_buf[sock_addr_buf_size * 2];

   conn_sock = socket(ip_version == ip::version::v4 ? protocol::tcp_ipv4 : protocol::tcp_ipv6);
   ::DWORD bytes_read;
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   sock.bind_to_this_coroutine_scheduler_iocp();
   if (!::AcceptEx(
      reinterpret_cast< ::SOCKET>(sock.get()), reinterpret_cast< ::SOCKET>(conn_sock.get()),
      sock_addr_buf, 0 /*don’t wait for data, just wait for a connection*/,
      sock_addr_buf_size, sock_addr_buf_size, &bytes_read, &ovl
   )) {
      auto err = static_cast< ::DWORD>(::WSAGetLastError());
      if (err == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(sock.get(), false /*read*/, 0 /*no timeout*/, &ovl);
         err = ovl.status();
         bytes_read = ovl.transferred_size();
      }
      if (err != ERROR_SUCCESS) {
         exception::throw_os_error(err);
      }
   }

   // Parse the weird buffer.
   ::SOCKADDR * local_sock_addr_ptr, * remote_sock_addr_ptr;
   ::GetAcceptExSockaddrs(
      sock_addr_buf, 0 /*no other data was read*/, sock_addr_buf_size, sock_addr_buf_size,
      &local_sock_addr_ptr, local_sock_addr.size_ptr(), &remote_sock_addr_ptr, remote_sock_addr.size_ptr()
   );
   memory::copy<std::int8_t>(
      reinterpret_cast<std::int8_t *>(local_sock_addr.sockaddr_ptr()),
      reinterpret_cast<std::int8_t const *>(local_sock_addr_ptr),
      static_cast<std::size_t>(local_sock_addr.size())
   );
   memory::copy<std::int8_t>(
      reinterpret_cast<std::int8_t *>(remote_sock_addr.sockaddr_ptr()),
      reinterpret_cast<std::int8_t const *>(remote_sock_addr_ptr),
      static_cast<std::size_t>(remote_sock_addr.size())
   );
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   // For Win32, this will result in +1 and -1 references to WinSock, so it just works.
   return _std::make_shared<connection>(
      _std::move(conn_sock), local_sock_addr.address(), local_sock_addr.port(), remote_sock_addr.address(),
      remote_sock_addr.port()
   );
}

}}} //namespace lofty::net::tcp

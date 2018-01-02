/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/io/binary/buffer.hxx>
#include <lofty/net/udp.hxx>
#include <lofty/thread.hxx>
#include "sockaddr_any.hxx"

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // ntohs()
   #include <sys/types.h> // ssize_t
   #include <sys/socket.h> // recvfrom() sendto()
#elif LOFTY_HOST_API_WIN32
   #include <winsock2.h>
   #include <mswsock.h> // WSARecvFrom()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

datagram::datagram(
   ip::address const & address__, ip::port port__,
   _std::shared_ptr<io::binary::memory_stream> data__ /*=nullptr*/
) :
   address_(address__),
   port_(port__),
   data_(_std::move(data__)) {
}

datagram::~datagram() {
}

}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

client::client() {
}

client::~client() {
}

void client::send(datagram const & dgram) {
   if (!sock) {
      // No socket yet; create one now.
      ip_version = dgram.address().version();
      sock = socket(ip_version == ip::version::v4 ? protocol::udp_ipv4 : protocol::udp_ipv6);
   }
   send_via(dgram, sock);
}

/*static*/ void client::send_via(datagram const & dgram, socket const & sock) {
   ip::sockaddr_any server_sock_addr(dgram.address(), dgram.port());
   std::int8_t const * buf;
   std::size_t buf_size;
   _std::tie(buf, buf_size) = dgram.data()->peek<std::int8_t>();
#if LOFTY_HOST_API_POSIX
   ::ssize_t bytes_sent;
   for (;;) {
      bytes_sent = ::sendto(
         sock.get(), buf, buf_size, 0, server_sock_addr.sockaddr_ptr(), server_sock_addr.size()
      );
      if (bytes_sent >= 0) {
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
            // Wait for sock.
            this_coroutine::sleep_until_fd_ready(sock.get(), true /*write*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
#elif LOFTY_HOST_API_WIN32
   const_cast<socket &>(sock).bind_to_this_coroutine_scheduler_iocp();
   ::WSABUF wsabuf;
   wsabuf.buf = reinterpret_cast<char *>(const_cast<std::int8_t *>(buf));
   wsabuf.len = static_cast< ::ULONG>(buf_size);
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   ::DWORD bytes_sent;
   if (::WSASendTo(
      reinterpret_cast< ::SOCKET>(sock.get()), &wsabuf, 1, nullptr, 0 /*no flags*/,
      server_sock_addr.sockaddr_ptr(), server_sock_addr.size(), &ovl, nullptr
   )) {
      auto err = static_cast< ::DWORD>(::WSAGetLastError());
      if (err == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(sock.get(), true /*write*/, 0 /*no timeout*/, &ovl);
         err = ovl.status();
         bytes_sent = ovl.transferred_size();
      }
      if (err != ERROR_SUCCESS) {
         exception::throw_os_error(err);
      }
   } else {
      bytes_sent = static_cast< ::DWORD>(wsabuf.len);
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();
}

}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

server::server(ip::address const & address, ip::port const & port) :
   ip::server(address, port, address.version() == ip::version::v4 ? protocol::udp_ipv4 : protocol::udp_ipv6) {
}

server::~server() {
}

_std::shared_ptr<datagram> server::receive() {
   // Create a new buffer large enough for a UDP datagram.
   io::binary::buffer buf(0xffff);
   ip::sockaddr_any sender_sock_addr;
   sender_sock_addr.set_size_from_ip_version(ip_version);
#if LOFTY_HOST_API_POSIX
   ::ssize_t bytes_received;
   for (;;) {
      bytes_received = ::recvfrom(
         sock.get(), buf.get_available(), buf.available_size(), 0,
         sender_sock_addr.sockaddr_ptr(), sender_sock_addr.size_ptr()
      );
      if (bytes_received >= 0) {
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
            // Wait for sock.
            this_coroutine::sleep_until_fd_ready(sock.get(), false /*read*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
#elif LOFTY_HOST_API_WIN32
   ::DWORD bytes_received;
   sock.bind_to_this_coroutine_scheduler_iocp();
   io::overlapped ovl;
   for (;;) {
      ::WSABUF wsabuf;
      wsabuf.buf = reinterpret_cast<char *>(buf.get_available());
      wsabuf.len = static_cast< ::ULONG>(buf.available_size());
      ::DWORD flags = 0;
      ovl.Offset = 0;
      ovl.OffsetHigh = 0;
      if (::WSARecvFrom(
         reinterpret_cast< ::SOCKET>(sock.get()), &wsabuf, 1, nullptr, &flags,
         sender_sock_addr.sockaddr_ptr(), sender_sock_addr.size_ptr(), &ovl, nullptr
      )) {
         auto err = static_cast< ::DWORD>(::WSAGetLastError());
         if (err == ERROR_IO_PENDING) {
            this_coroutine::sleep_until_fd_ready(sock.get(), false /*read*/, 0 /*no timeout*/, &ovl);
            err = ovl.status();
         }
         /* WinXP+ bug: UDP and IOCPs don’t mix well: WinXP+ will report an ICMP failure via
         ERROR_PORT_UNREACHABLE from an attempt to deliver a datagram only on the *next* IOCP call. Since
         it’s UDP, applications should not rely on the OS to report delivery failures. */
         if (err == ERROR_SUCCESS) {
            break;
         } else if (err == ERROR_PORT_UNREACHABLE) {
            ::OutputDebugString(L"ERROR_PORT_UNREACHABLE on UDP\r\n");
         } else if (err == WSAECONNRESET) {
            ::OutputDebugString(L"WSAECONNRESET on non-connected UDP\r\n");
         } else {
            exception::throw_os_error(err);
         }
      } else {
         break;
      }
   }
   bytes_received = ovl.transferred_size();
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   buf.mark_as_used(static_cast<std::size_t>(bytes_received));
   buf.shrink_to_fit();
   return _std::make_shared<datagram>(
      sender_sock_addr.address(), sender_sock_addr.port(),
      _std::make_shared<io::binary::memory_stream>(_std::move(buf))
   );
}

void server::send(datagram const & dgram) {
   client::send_via(dgram, sock);
}

}}} //namespace lofty::net::udp

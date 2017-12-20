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

#if LOFTY_HOST_API_POSIX
   #include <errno.h> // EINTR errno
   #include <netinet/in.h> // ntohs()
   #include <sys/types.h> // sockaddr sockaddr_in ssize_t
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

namespace {

union sockaddr_any {
   ::sockaddr_in sa4;
   ::sockaddr_in6 sa6;
};

} //namespace

client::client() {
}

client::~client() {
}

void client::send(datagram const & dgram) {
   if (!sock_fd) {
      // No socket yet; create one now.
      ip_version = dgram.address().version();
      sock_fd = ip::create_socket(ip_version, ip::transport::udp);
   }
   send_via(dgram, sock_fd);
}

/*static*/ void client::send_via(datagram const & dgram, io::filedesc const & sock_fd) {
#if LOFTY_HOST_API_POSIX
   ::socklen_t server_sock_addr_size;
#elif LOFTY_HOST_API_WIN32
   int server_sock_addr_size;
#else
   #error "TODO: HOST_API"
#endif
   sockaddr_any server_sock_addr;
   switch (dgram.address().version().base()) {
      case ip::version::v4:
         server_sock_addr_size = sizeof server_sock_addr.sa4;
         memory::clear(&server_sock_addr.sa4);
         server_sock_addr.sa4.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&server_sock_addr.sa4.sin_addr.s_addr), dgram.address().raw(),
            sizeof server_sock_addr.sa4.sin_addr.s_addr
         );
         server_sock_addr.sa4.sin_port = htons(dgram.port().number());
         break;
      case ip::version::v6:
         server_sock_addr_size = sizeof server_sock_addr.sa6;
         memory::clear(&server_sock_addr.sa6);
         //server_sock_addr.sa6.sin6_flowinfo = 0;
         server_sock_addr.sa6.sin6_family = AF_INET6;
         memory::copy(
            &server_sock_addr.sa6.sin6_addr.s6_addr[0], dgram.address().raw(),
            sizeof server_sock_addr.sa6.sin6_addr.s6_addr
         );
         server_sock_addr.sa6.sin6_port = htons(dgram.port().number());
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   std::int8_t const * buf;
   std::size_t buf_size;
   _std::tie(buf, buf_size) = dgram.data()->peek<std::int8_t>();
#if LOFTY_HOST_API_POSIX
   ::ssize_t bytes_sent;
   for (;;) {
      bytes_sent = ::sendto(
         sock_fd.get(), buf, buf_size, 0,
         reinterpret_cast< ::sockaddr const *>(&server_sock_addr), server_sock_addr_size
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
            // Wait for sock_fd.
            this_coroutine::sleep_until_fd_ready(sock_fd.get(), true /*write*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
#elif LOFTY_HOST_API_WIN32
   sock_fd.bind_to_this_coroutine_scheduler_iocp();
   ::WSABUF wsabuf;
   wsabuf.buf = buf;
   wsabuf.len = buf_size;
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   ::DWORD bytes_sent;
   if (::WSASendTo(
      sock_fd.get(), &wsabuf, 1, nullptr, 0 /*no flags*/,
      reinterpret_cast< ::SOCKADDR const *>(&server_sock_addr), server_sock_addr_size, &ovl, nullptr
   )) {
      auto err = static_cast< ::DWORD>(::WSAGetLastError());
      if (err == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(sock_fd.get(), true /*write*/, 0 /*no timeout*/, &ovl);
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
   ip::server(address, port, ip::transport::udp) {
}

server::~server() {
}

_std::shared_ptr<datagram> server::receive() {
   // Create a new buffer large enough for a UDP datagram.
   io::binary::buffer buf(0xffff);
   sockaddr_any sender_sock_addr;
#if LOFTY_HOST_API_POSIX
   ::socklen_t sender_sock_addr_size;
   ::ssize_t bytes_received;
   for (;;) {
      bytes_received = ::recvfrom(
         sock_fd.get(), buf.get_available(), buf.available_size(), 0,
         reinterpret_cast< ::sockaddr *>(&sender_sock_addr), &sender_sock_addr_size
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
            // Wait for sock_fd.
            this_coroutine::sleep_until_fd_ready(sock_fd.get(), false /*read*/, 0 /*no timeout*/);
            break;
         default:
            exception::throw_os_error(static_cast<errint_t>(err));
      }
   }
#elif LOFTY_HOST_API_WIN32
   sock_fd.bind_to_this_coroutine_scheduler_iocp();
   ::WSABUF wsabuf;
   wsabuf.buf = buf.get_available();
   wsabuf.len = buf.available_size();
   int flags = 0;
   ::socklen_t sender_sock_addr_size = sizeof sender_sock_addr;
   io::overlapped ovl;
   ovl.Offset = 0;
   ovl.OffsetHigh = 0;
   ::DWORD bytes_received;
   if (::WSARecvFrom(
      sock_fd.get(), &wsabuf, 1, nullptr, &flags,
      reinterpret_cast< ::SOCKADDR *>(&sender_sock_addr), &sender_sock_addr_size, &ovl, nullptr
   )) {
      auto err = static_cast< ::DWORD>(::WSAGetLastError());
      if (err == ERROR_IO_PENDING) {
         this_coroutine::sleep_until_fd_ready(sock_fd.get(), false /*read*/, 0 /*no timeout*/, &ovl);
         err = ovl.status();
         bytes_received = ovl.transferred_size();
      }
      if (err != ERROR_SUCCESS) {
         exception::throw_os_error(err);
      }
   } else {
      bytes_received = static_cast< ::DWORD>(wsabuf.len);
   }
#else
   #error "TODO: HOST_API"
#endif
   this_coroutine::interruption_point();

   buf.mark_as_used(static_cast<std::size_t>(bytes_received));
   buf.shrink_to_fit();
   ip::address sender_addr;
   ip::port sender_port;
   switch (sender_sock_addr_size) {
      case sizeof sender_sock_addr.sa4:
         sender_addr = ip::address(
            *reinterpret_cast<ip::address::v4_type *>(&sender_sock_addr.sa4.sin_addr.s_addr)
         );
         sender_port = ip::port(ntohs(sender_sock_addr.sa4.sin_port));
         break;
      case sizeof sender_sock_addr.sa6:
         sender_addr = ip::address(
            *reinterpret_cast<ip::address::v6_type *>(&sender_sock_addr.sa6.sin6_addr.s6_addr)
         );
         sender_port = ip::port(ntohs(sender_sock_addr.sa6.sin6_port));
         break;
   }
   return _std::make_shared<datagram>(
      sender_addr, sender_port, _std::make_shared<io::binary::memory_stream>(_std::move(buf))
   );
}

void server::send(datagram const & dgram) {
   client::send_via(dgram, sock_fd);
}

}}} //namespace lofty::net::udp

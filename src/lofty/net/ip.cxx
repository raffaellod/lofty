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
#include <lofty/byte_order.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/thread.hxx>

#if LOFTY_HOST_API_POSIX
   #include <netinet/in.h> // htons()
   #include <sys/types.h> // sockaddr sockaddr_in
   #include <sys/socket.h> // bind() socket()
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
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

static_assert(
   sizeof(address::v4_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->bytes, "v4_type is too big"
);
static_assert(
   sizeof(address::v6_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->bytes, "v6_type is too big"
);

static _pvt::raw_address const raw_any_v4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v4
};
static _pvt::raw_address const raw_any_v6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v6
};

address const & address::any_v4 = static_cast<address const &>(raw_any_v4);
address const & address::any_v6 = static_cast<address const &>(raw_any_v6);

io::filedesc create_socket(version version_, transport transport_) {
   if (version_ == version::any) {
      // TODO: provide more information in the exception.
      LOFTY_THROW(domain_error, ());
   }
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   int family, type;
   switch (version_.base()) {
      case version::v4:
         family = AF_INET;
         break;
      case version::v6:
         family = AF_INET6;
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   switch (transport_.base()) {
      case transport::tcp:
         type = SOCK_STREAM;
         break;
      case transport::udp:
         type = SOCK_DGRAM;
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
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
      fd.share_with_subprocesses(false);
      if (async) {
         fd.set_nonblocking(true);
      }
   #endif
   return _std::move(fd);
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   static std::uint8_t const wsa_major_version = 2, wsa_minor_version = 2;
   ::WSADATA wsa_data;
   // TODO: only call once per process! And pair that with calling WSACleanup().
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

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

namespace {

union sockaddr_any {
   ::sockaddr_in sa4;
   ::sockaddr_in6 sa6;
};

} //namespace

server::server(address const & address, port const & port, transport transport_) :
   sock_fd(create_socket(address.version(), transport_)),
   ip_version(address.version()) {
#if LOFTY_HOST_API_POSIX
   ::socklen_t server_sock_addr_size;
#elif LOFTY_HOST_API_WIN32
   int server_sock_addr_size;
#else
   #error "TODO: HOST_API"
#endif
   sockaddr_any server_sock_addr;
   switch (ip_version.base()) {
      case ip::version::v4:
         server_sock_addr_size = sizeof server_sock_addr.sa4;
         memory::clear(&server_sock_addr.sa4);
         server_sock_addr.sa4.sin_family = AF_INET;
         memory::copy(
            reinterpret_cast<std::uint8_t *>(&server_sock_addr.sa4.sin_addr.s_addr), address.raw(),
            sizeof server_sock_addr.sa4.sin_addr.s_addr
         );
         server_sock_addr.sa4.sin_port = htons(port.number());
         break;
      case ip::version::v6:
         server_sock_addr_size = sizeof server_sock_addr.sa6;
         memory::clear(&server_sock_addr.sa6);
         //server_sock_addr.sa6.sin6_flowinfo = 0;
         server_sock_addr.sa6.sin6_family = AF_INET6;
         memory::copy(
            &server_sock_addr.sa6.sin6_addr.s6_addr[0], address.raw(),
            sizeof server_sock_addr.sa6.sin6_addr.s6_addr
         );
         server_sock_addr.sa6.sin6_port = htons(port.number());
         break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
#if LOFTY_HOST_API_WIN32
   if (::bind(
      reinterpret_cast< ::SOCKET>(sock_fd.get()),
      reinterpret_cast< ::SOCKADDR *>(&server_sock_addr), server_sock_addr_size
   ) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (::bind(sock_fd.get(), reinterpret_cast< ::sockaddr *>(&server_sock_addr), server_sock_addr_size) < 0) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
#if LOFTY_HOST_API_POSIX
   int value = 1;
   ::setsockopt(sock_fd.get(), SOL_SOCKET, SO_REUSEADDR, &value, sizeof value);
#endif
#if LOFTY_HOST_API_WIN32
   ::WSACleanup();
#endif
}

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

to_text_ostream<net::ip::address>::to_text_ostream() {
   v6_group_ttos.set_format(LOFTY_SL("x"));
}

to_text_ostream<net::ip::address>::~to_text_ostream() {
}

void to_text_ostream<net::ip::address>::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<net::ip::address>::write(net::ip::address const & src, io::text::ostream * dst) {
   switch (src.version().base()) {
      case net::ip::version::any:
         char_ttos.write('-', dst);
         break;
      case net::ip::version::v4: {
         auto group = src.raw(), groups_end = group + sizeof(net::ip::address::v4_type);
         v4_group_ttos.write(*group, dst);
         while (++group < groups_end) {
            char_ttos.write('.', dst);
            v4_group_ttos.write(*group, dst);
         }
         break;
      }
      case net::ip::version::v6: {
         /* This implementation complies with RFC 4291 “IP Version 6 Addressing Architecture” § 2.2. “Text
         Representation of Addresses”. */

         auto groups_begin = reinterpret_cast<std::uint16_t const *>(src.raw());
         auto groups_end   = groups_begin + sizeof(net::ip::address::v6_type) / sizeof groups_begin[0];

         // Find the longest run of zeroes, so we can print “::” instead.
         auto curr_0s_begin = groups_begin, max_0s_begin = groups_begin;
         auto curr_0s_end   = groups_begin, max_0s_end   = groups_begin;
         for (auto group = groups_begin; group < groups_end; ++group) {
            if (*group == 0) {
               if (group != curr_0s_end) {
                  // Start a new “current 0s” range.
                  curr_0s_begin = group;
               }
               // Include this group in the “current 0s” range.
               curr_0s_end = group + 1;
            } else if (group == curr_0s_end) {
               // End of the “current 0s” range; save it as “max 0s” if it’s the longest.
               if (curr_0s_end - curr_0s_begin > max_0s_end - max_0s_begin) {
                  max_0s_begin = curr_0s_begin;
                  max_0s_end = curr_0s_end;
               }
            }
         }
         // Check if we ended on what should become the “max 0s” range.
         if (curr_0s_end - curr_0s_begin > max_0s_end - max_0s_begin) {
            max_0s_begin = curr_0s_begin;
            max_0s_end = curr_0s_end;
         }

         auto group = groups_begin;
         if (max_0s_end == max_0s_begin) {
            // Write the first group, not preceded by “:”.
            v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
         } else {
            if (group < max_0s_begin) {
               // Write all the groups before the “max 0s” range.
               do {
                  v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
                  char_ttos.write(':', dst);
               } while (group < max_0s_begin);
            } else {
               // Print one ”:”; the second will be printed in the loop below.
               char_ttos.write(':', dst);
            }
            if (max_0s_end == groups_end) {
               // No more groups to write; just add a second “:” and skip the second loop.
               char_ttos.write(':', dst);
               break;
            }
            group = max_0s_end;
         }
         // Write all the groups after the “max 0s” range.
         while (group < groups_end) {
            char_ttos.write(':', dst);
            v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
         }
         break;
      }
   }
}

} //namespace lofty

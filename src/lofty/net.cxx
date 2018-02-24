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
#include <lofty/byte_order.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/regex.hxx>
#include <lofty/thread.hxx>
#include "net/sockaddr_any.hxx"

#if LOFTY_HOST_API_POSIX
   #include <netinet/in.h> // htons()
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

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace net {

_std::atomic<unsigned> wsa_client::refs(0);

wsa_client::wsa_client() {
   if (++refs == 1) {
      static std::uint8_t const major_version = 2, minor_version = 2;
      ::WSADATA data;
      if (int ret = ::WSAStartup(MAKEWORD(major_version, minor_version), &data)) {
         exception::throw_os_error(static_cast<errint_t>(ret));
      }
      // Don’t bother checking for versions; even WinSock 2.2 is ancient.
   }
}

wsa_client::~wsa_client() {
   if (--refs == 0) {
      ::WSACleanup();
   }
}

}} //namespace lofty::net

#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

/*static*/ io::filedesc socket::socket_filedesc(protocol protocol_) {
   bool async = (this_thread::coroutine_scheduler() != nullptr);
   int family, type;
   switch (protocol_.base()) {
      case protocol::tcp_ipv4: family = AF_INET;  type = SOCK_STREAM; break;
      case protocol::tcp_ipv6: family = AF_INET6; type = SOCK_STREAM; break;
      case protocol::udp_ipv4: family = AF_INET;  type = SOCK_DGRAM;  break;
      case protocol::udp_ipv6: family = AF_INET6; type = SOCK_DGRAM;  break;
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
      fd.share_with_subprocesses(false);
      if (async) {
         // Using coroutines, so make this socket non-blocking.
         fd.set_nonblocking(true);
      }
   #endif
   return _std::move(fd);
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
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

}} //namespace lofty::net

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
static _pvt::raw_address const raw_localhost_v4 = {
   { 127, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v4
};
static _pvt::raw_address const raw_localhost_v6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, version::v6
};

address const & address::any_v4 = static_cast<address const &>(raw_any_v4);
address const & address::any_v6 = static_cast<address const &>(raw_any_v6);
address const & address::localhost_v4 = static_cast<address const &>(raw_localhost_v4);
address const & address::localhost_v6 = static_cast<address const &>(raw_localhost_v6);

/*explicit*/ address::address(collections::vector<std::uint8_t> const & src_raw) {
   switch (src_raw.size()) {
      case 0:
         // 0 bytes result in the same version as the default constructor, IPv6.
      case sizeof(v6_type):
         version_ = ip::version::v6;
         break;
      case sizeof(v4_type):
         version_ = ip::version::v4;
         break;
      default:
         LOFTY_THROW(argument_error, ());
   }
   memory::copy(bytes, src_raw.data(), src_raw.size());
}

bool address::operator==(address const & right) const {
   if (version_ != right.version_) {
      return false;
   }
   unsigned bytes_size;
   switch (version_) {
      case ip::version::v4: bytes_size = sizeof(address::v4_type); break;
      case ip::version::v6: bytes_size = sizeof(address::v6_type); break;
      LOFTY_SWITCH_WITHOUT_DEFAULT
   }
   for (unsigned i = 0; i < bytes_size; ++i) {
      if (bytes[i] != right.bytes[i]) {
         return false;
      }
   }
   return true;
}

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

server::server() {
}

server::server(address const & address, port const & port, protocol protocol_) :
   sock(protocol_),
   ip_version(address.version()) {
   sockaddr_any server_sock_addr(address, port);
#if LOFTY_HOST_API_WIN32
   if (::bind(
      reinterpret_cast< ::SOCKET>(sock.get()),
      server_sock_addr.sockaddr_ptr(), server_sock_addr.size()
   ) < 0) {
      exception::throw_os_error(static_cast<errint_t>(::WSAGetLastError()));
   }
#else
   if (::bind(sock.get(), server_sock_addr.sockaddr_ptr(), server_sock_addr.size()) < 0) {
      exception::throw_os_error();
   }
#endif
}

server::~server() {
#if LOFTY_HOST_API_POSIX
   if (sock) {
      int value = 1;
      ::setsockopt(sock.get(), SOL_SOCKET, SO_REUSEADDR, &value, sizeof value);
   }
#endif
}

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

from_text_istream<net::ip::address>::from_text_istream() {
}

void from_text_istream<net::ip::address>::convert_capture(
   text::parsers::dynamic_match_capture const & capture0, net::ip::address * dst
) {
   // TODO: check for ‘.’ or ‘:’ to decide whether it’s IPv4 or IPv6.

   dst->version_ = net::ip::version::v4;
   auto bytes = const_cast<std::uint8_t *>(dst->raw());
   std::uint16_t digits_group;
   for (unsigned i = 0; i < 3; ++i) {
      digits_group_ftis.convert_capture(capture0.repetition_group(0)[i].capture_group(0), &digits_group);
      bytes[i] = digits_group;
   }
   digits_group_ftis.convert_capture(capture0.capture_group(0), &digits_group);
   bytes[3] = digits_group;
}

text::parsers::dynamic_state const * from_text_istream<net::ip::address>::format_to_parser_states(
   text::parsers::regex_capture_format const & format, text::parsers::dynamic * parser
) {
   // TODO: more format validation.
   throw_on_unused_streaming_format_chars(format.expr.cbegin(), format.expr);

   text::parsers::regex_capture_format digits_group_format;
   // TODO: for IPv6, change this to only accept hex.
   //digits_group_format.expr = LOFTY_SL("x");
   auto v4_digits_group = digits_group_ftis.format_to_parser_states(digits_group_format, parser);

   auto digits4_cap_group = parser->create_capture_group(v4_digits_group);
   LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(dot_state, nullptr, nullptr, '.');
   auto digits13_cap_group = parser->create_capture_group(v4_digits_group);
   digits13_cap_group->set_next(&dot_state.base);
   auto digits13_rep_group = parser->create_repetition_group(digits13_cap_group, 3, 3);
   digits13_rep_group->set_next(digits4_cap_group);

   return digits13_rep_group;
   // TODO: IPv6 support as alternative to digits1_cap_group.
}

} //namespace lofty

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

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

#if LOFTY_HOST_API_POSIX
   #include <netinet/in.h> // htons()
   #include <sys/socket.h> // AF_*
   #include <sys/types.h> // sockaddr sockaddr_in
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

//! IPv4 or v6 socket address.
class sockaddr_any {
public:
   //! Type of a generic socket address.
#if LOFTY_HOST_API_POSIX
   typedef ::sockaddr sockaddr_t;
#elif LOFTY_HOST_API_WIN32
   typedef ::SOCKADDR sockaddr_t;
#endif
   //! Type of the size of a socket address.
#if LOFTY_HOST_API_POSIX
   typedef ::socklen_t socksize_t;
#elif LOFTY_HOST_API_WIN32
   typedef int socksize_t;
#endif

private:
   union u_t {
      //! IPv4 socket address.
      ::sockaddr_in sa4;
      //! IPv6 socket address.
      ::sockaddr_in6 sa6;
   };

public:
   sockaddr_any() :
      size_(0) {
   }

   sockaddr_any(ip::address const & address_, ip::port const & port_) {
      switch (address_.version().base()) {
         case version::v4:
            size_ = sizeof u.sa4;
            memory::clear(&u.sa4);
            u.sa4.sin_family = AF_INET;
            memory::copy(
               reinterpret_cast<std::uint8_t *>(&u.sa4.sin_addr.s_addr), address_.raw(),
               sizeof u.sa4.sin_addr.s_addr
            );
            u.sa4.sin_port = htons(port_.number());
            break;
         case version::v6:
            size_ = sizeof u.sa6;
            memory::clear(&u.sa6);
            //u.sa6.sin6_flowinfo = 0;
            u.sa6.sin6_family = AF_INET6;
            memory::copy(&u.sa6.sin6_addr.s6_addr[0], address_.raw(), sizeof u.sa6.sin6_addr.s6_addr);
            u.sa6.sin6_port = htons(port_.number());
            break;
         LOFTY_SWITCH_WITHOUT_DEFAULT
      }
   }

   ip::address address() const {
      switch (size_) {
         case sizeof u.sa4:
            return ip::address(*reinterpret_cast<address::v4_type const *>(&u.sa4.sin_addr.s_addr));
         case sizeof u.sa6:
            return ip::address(*reinterpret_cast<address::v6_type const *>(&u.sa6.sin6_addr.s6_addr));
         default:
            return ip::address();
      }
   }

   ip::port port() const {
      switch (size_) {
         case sizeof u.sa4:
            return ip::port(ntohs(u.sa4.sin_port));
         case sizeof u.sa6:
            return ip::port(ntohs(u.sa6.sin6_port));
         default:
            return ip::port();
      }
   }

   void set_size_from_ip_version(version ip_version) {
      switch (ip_version.base()) {
         case version::v4:
            size_ = sizeof u.sa4;
            break;
         case version::v6:
            size_ = sizeof u.sa6;
            break;
         LOFTY_SWITCH_WITHOUT_DEFAULT
      }
   }

   sockaddr_t * sockaddr_ptr() {
      return reinterpret_cast<sockaddr_t *>(&u);
   }

   socksize_t size() const {
      return size_;
   }

   socksize_t * size_ptr() {
      return &size_;
   }

private:
   socksize_t size_;
   u_t u;
};

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

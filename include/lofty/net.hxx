/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_NET_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_NET_HXX
#endif

#ifndef _LOFTY_NET_HXX_NOPUB
#define _LOFTY_NET_HXX_NOPUB

#include <lofty/enum.hxx>
#include <lofty/io.hxx>
#include <lofty/_std/atomic.hxx>
#include <lofty/_std/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Networking facilities.
namespace net {

   //! Internet Protocol-related classes and facilities.
   namespace ip {}

}
}


namespace lofty { namespace net {
_LOFTY_PUBNS_BEGIN

//! Networking protocols.
LOFTY_ENUM(protocol,
   //! Transmission Control Protocol over Internet Protocol version 4 (TCP/IP).
   (tcp_ipv4, 1),
   //! Transmission Control Protocol over Internet Protocol version 6 (TCP/IPv6).
   (tcp_ipv6, 2),
   //! User Datagram Protocol over Internet Protocol version 4 (UDP/IP).
   (udp_ipv4, 3),
   //! User Datagram Protocol over Internet Protocol version 6 (UDP/IPv6).
   (udp_ipv6, 4)
);

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace net {
_LOFTY_PUBNS_BEGIN

//! Adds scoped WinSock reference counting with implicit initialization and termination.
class LOFTY_SYM wsa_client {
protected:
   //! Constructor. Adds one reference to the WinSock DLL.
   wsa_client();

   //! Destructor. Discards one reference to the WinSock DLL.
   ~wsa_client();

private:
   //! Reference count for the WinSock DLL.
   static _std::_LOFTY_PUBNS atomic<unsigned> refs;
};

_LOFTY_PUBNS_END
}}

#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {
_LOFTY_PUBNS_BEGIN

//! Socket for networking I/O.
class LOFTY_SYM socket :
#if LOFTY_HOST_API_WIN32
   private wsa_client,
#endif //if LOFTY_HOST_API_WIN32
   public io::_LOFTY_PUBNS filedesc {
public:
   //! Constructs an empty socket.
   socket() {
   }

   /*! Constructor for a given protocol.

   @param protocol_
      Protocol for which to create the socket.
   */
   explicit socket(protocol protocol_) :
      io::_pub::filedesc(socket_filedesc(protocol_)) {
   }

   /*! Constructor for a given protocol. This overload is necessary to prevent the compiler, under POSIX, from
   implicitly converting the enum into an int, which is the same thing as io::filedesc_t, and therefore pick
   the wrong overload.

   @param protocol_
      Protocol for which to create the socket.
   */
   explicit socket(protocol::enum_type protocol_) :
      io::_pub::filedesc(socket_filedesc(protocol_)) {
   }

   /*! Constructor that takes ownership of a file descriptor.

   @param fd_
      Source file descriptor.
   */
   explicit socket(io::_LOFTY_PUBNS filedesc_t fd_) :
      io::_pub::filedesc(fd_) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   socket(socket && src) :
      io::_pub::filedesc(_std::_pub::move(src)) {
   }

   ~socket() {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   socket & operator=(socket && src) {
      io::_pub::filedesc::operator=(_std::_pub::move(src));
      return *this;
   }

private:
   /*! Implementation of the main constructor, to allow for easy RAII.

   @param protocol_
      Protocol for which to create the socket.
   */
   static io::_LOFTY_PUBNS filedesc socket_filedesc(protocol protocol_);
};

_LOFTY_PUBNS_END
}} //namespace lofty::net

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_HXX_NOPUB

#ifdef _LOFTY_NET_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace net {

   using _pub::protocol;
   using _pub::socket;
   #if LOFTY_HOST_API_WIN32
   using _pub::wsa_client;
   #endif

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_NET_HXX

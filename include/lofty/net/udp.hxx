/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_NET_UDP_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_NET_UDP_HXX
#endif

#ifndef _LOFTY_NET_UDP_HXX_NOPUB
#define _LOFTY_NET_UDP_HXX_NOPUB

#include <lofty/io/binary/memory.hxx>
#include <lofty/net.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/memory.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

//! User Datagram Protocol-related classes and facilities.
namespace udp {}

}} //namespace lofty::net

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {
_LOFTY_PUBNS_BEGIN

//! Single UDP message.
class LOFTY_SYM datagram : public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param address
      IP address.
   @param port
      IP port.
   @param
      Data contained in the datagram.
   */
   datagram(
      ip::_LOFTY_PUBNS address const & address, ip::_LOFTY_PUBNS port port,
      _std::_LOFTY_PUBNS shared_ptr<io::binary::_LOFTY_PUBNS memory_stream> data = nullptr
   );

   //! Destructor.
   ~datagram();

   /*! Returns the address used/to be used.

   @return
      IP address.
   */
   ip::_LOFTY_PUBNS address const & address() const {
      return address_;
   }

   /*! Returns the port used/to be used.

   @return
      Port.
   */
   ip::_LOFTY_PUBNS port const & port() const {
      return port_;
   }

   /*! Returns an input/output binary buffer representing the datagram, to read its payload or to build one.

   @return
      Input/output stream for the datagram’s data.
   */
   _std::_LOFTY_PUBNS shared_ptr<io::binary::_LOFTY_PUBNS memory_stream> const & data() const {
      return data_;
   }

private:
   //! Address.
   ip::_LOFTY_PUBNS address address_;
   //! Port.
   ip::_LOFTY_PUBNS port port_;
   //! Message data.
   _std::_LOFTY_PUBNS shared_ptr<io::binary::_LOFTY_PUBNS memory_stream> data_;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {
_LOFTY_PUBNS_BEGIN

//! Receives datagrams sent to a given UDP port.
class LOFTY_SYM server : public ip::_LOFTY_PUBNS server {
public:
   /*! Constructor.

   @param address
      Address to bind to.
   @param port
      Port to listen for connections on.
   */
   server(ip::_LOFTY_PUBNS address const & address, ip::_LOFTY_PUBNS port const & port);

   //! Destructor.
   ~server();

   /*! Accepts and returns a datagram from another UDP client. A UDP client must not call this method without
   having first called send().

   @return
      Newly-received datagram.
   */
   _std::_LOFTY_PUBNS shared_ptr<datagram> receive();

   /*! Sends a datagram to the server indicated by its address() and port() properties.

   @param dgram
      Datagram to send.
   */
   void send(datagram const & dgram);

protected:
   //! Default constructor for subclasses.
   server();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {
_LOFTY_PUBNS_BEGIN

//! Sends datagrams to UDP servers.
class LOFTY_SYM client : public server {
public:
   //! Constructor.
   client();

   /*! Assigns an IP version and creates a suitable UDP socket.

   @param version
      IP version to set.
   */
   void set_ip_version(ip::_LOFTY_PUBNS version const & version);
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_UDP_HXX_NOPUB

#ifdef _LOFTY_NET_UDP_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace net { namespace udp {

   using _pub::client;
   using _pub::datagram;
   using _pub::server;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_NET_UDP_HXX

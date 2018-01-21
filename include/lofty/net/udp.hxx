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
#define _LOFTY_NET_UDP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/io/binary/memory.hxx>
#include <lofty/net.hxx>
#include <lofty/net/ip.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

//! User Datagram Protocol-related classes and facilities.
namespace udp {}

}} //namespace lofty::net

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

//! Single UDP message.
class LOFTY_SYM datagram : public noncopyable {
public:
   /*! Constructor.

   @param address
      IP address.
   @param port
      IP port.
   @param
      Data contained in the datagram.
   */
   datagram(ip::address const & address, ip::port port, _std::shared_ptr<io::binary::memory_stream> data = nullptr);

   //! Destructor.
   ~datagram();

   /*! Returns the address used/to be used.

   @return
      IP address.
   */
   ip::address const & address() const {
      return address_;
   }

   /*! Returns the port used/to be used.

   @return
      Port.
   */
   ip::port const & port() const {
      return port_;
   }

   /*! Returns an input/output binary buffer representing the datagram, to read its payload or to build one.

   @return
      Input/output stream for the datagram’s data.
   */
   _std::shared_ptr<io::binary::memory_stream> const & data() const {
      return data_;
   }

private:
   //! Address.
   ip::address address_;
   //! Port.
   ip::port port_;
   //! Message data.
   _std::shared_ptr<io::binary::memory_stream> data_;
};

}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

//! Sends datagrams to UDP servers.
class LOFTY_SYM client : public ip::server {
public:
   //! Constructor.
   client();

   //! Destructor.
   ~client();

   /*! Accepts and returns a datagram from another UDP client. A UDP client must not call this method without
   having first called send().

   @return
      Newly-received datagram.
   */
   _std::shared_ptr<datagram> receive();

   /*! Sends a datagram to the server indicated by its address() and port() properties.

   @param dgram
      Datagram to send.
   */
   void send(datagram const & dgram);

protected:
   /*! Constructor for use by udp::server; creates and binds the socket to the specified address and port.

   @param address
      Address to bind to.
   @param port
      Port to listen for connections on.
   */
   client(ip::address const & address, ip::port const & port);
};

}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace udp {

//! Receives datagrams sent to a given UDP port.
class LOFTY_SYM server : public client {
public:
   /*! Constructor.

   @param address
      Address to bind to.
   @param port
      Port to listen for connections on.
   */
   server(ip::address const & address, ip::port const & port);

   //! Destructor.
   ~server();
};

}}} //namespace lofty::net::udp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_UDP_HXX

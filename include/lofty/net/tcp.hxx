/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_NET_TCP_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_NET_TCP_HXX
#endif

#ifndef _LOFTY_NET_TCP_HXX_NOPUB
#define _LOFTY_NET_TCP_HXX_NOPUB

#include <lofty/io/binary.hxx>
#include <lofty/net.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/memory.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

//! Transmission Control Protocol-related classes and facilities.
namespace tcp {}

}} //namespace lofty::net

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {
_LOFTY_PUBNS_BEGIN

//! Initialized TCP connection.
class LOFTY_SYM connection :
#if LOFTY_HOST_API_WIN32
   private net::_LOFTY_PUBNS wsa_client,
#endif //if LOFTY_HOST_API_WIN32
   public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param fd
      Connected socket.
   @param local_address
      Local address.
   @param local_port
      Local port.
   @param remote_address
      Address of the remote peer.
   @param remote_port
      Port of the remote peer.
   */
   connection(
      io::_LOFTY_PUBNS filedesc fd, ip::_LOFTY_PUBNS address && local_address,
      ip::_LOFTY_PUBNS port && local_port, ip::_LOFTY_PUBNS address && remote_address,
      ip::_LOFTY_PUBNS port && remote_port
   );

   //! Destructor.
   ~connection();

   /*! Returns the local address for the connection.

   @return
      IP address.
   */
   ip::_LOFTY_PUBNS address const & local_address() const {
      return local_address_;
   }

   /*! Returns the local port being used.

   @return
      Port.
   */
   ip::_LOFTY_PUBNS port const & local_port() const {
      return local_port_;
   }

   /*! Returns the address of the remote peer.

   @return
      IP address.
   */
   ip::_LOFTY_PUBNS address const & remote_address() const {
      return remote_address_;
   }

   /*! Returns the port the remote peer is using.

   @return
      Port.
   */
   ip::_LOFTY_PUBNS port const & remote_port() const {
      return remote_port_;
   }

   /*! Returns a binary input/output stream representing the socket, to receive data from the remote peer.

   @return
      Input/output stream for the connection’s socket.
   */
   _std::_LOFTY_PUBNS shared_ptr<io::binary::_LOFTY_PUBNS file_iostream> const & socket() {
      return socket_;
   }

private:
   //! Stream for the connection’s socket.
   _std::_LOFTY_PUBNS shared_ptr<io::binary::_LOFTY_PUBNS file_iostream> socket_;
   //! Local address.
   ip::_LOFTY_PUBNS address local_address_;
   //! Local port.
   ip::_LOFTY_PUBNS port local_port_;
   //! Address of the remote peer.
   ip::_LOFTY_PUBNS address remote_address_;
   //! Port of the remote peer.
   ip::_LOFTY_PUBNS port remote_port_;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::net::tcp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {
_LOFTY_PUBNS_BEGIN

//! Accepts client connection requests for a given TCP port.
class LOFTY_SYM server : public ip::_LOFTY_PUBNS server {
public:
   /*! Constructor.

   @param address
      Address to bind to.
   @param port
      Port to listen for connections on.
   @param backlog_size
      Count of established connections that will be allowed to queue until the server is able to accept them.
   */
   server(
      ip::_LOFTY_PUBNS address const & address, ip::_LOFTY_PUBNS port const & port, unsigned backlog_size = 5
   );

   //! Destructor.
   ~server();

   /*! Accepts and returns a connection from a client.

   @return
      New client connection.
   */
   _std::_LOFTY_PUBNS shared_ptr<connection> accept();
};

_LOFTY_PUBNS_END
}}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_TCP_HXX_NOPUB

#ifdef _LOFTY_NET_TCP_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace net { namespace tcp {

   using _pub::connection;
   using _pub::server;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_NET_TCP_HXX

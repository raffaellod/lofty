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
#define _LOFTY_NET_TCP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/io/binary.hxx>
#include <lofty/net.hxx>
#include <lofty/net/ip.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

//! Transmission Control Protocol-related classes and facilities.
namespace tcp {}

}} //namespace lofty::net

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {

//! Initialized TCP connection.
class LOFTY_SYM connection :
#if LOFTY_HOST_API_WIN32
   private wsa,
#endif //if LOFTY_HOST_API_WIN32
   public noncopyable {
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
      io::filedesc fd, ip::address && local_address, ip::port && local_port, ip::address && remote_address,
      ip::port && remote_port
   );

   //! Destructor.
   ~connection();

   /*! Returns the local address for the connection.

   @return
      IP address.
   */
   ip::address const & local_address() const {
      return local_address_;
   }

   /*! Returns the local port being used.

   @return
      Port.
   */
   ip::port const & local_port() const {
      return local_port_;
   }

   /*! Returns the address of the remote peer.

   @return
      IP address.
   */
   ip::address const & remote_address() const {
      return remote_address_;
   }

   /*! Returns the port the remote peer is using.

   @return
      Port.
   */
   ip::port const & remote_port() const {
      return remote_port_;
   }

   /*! Returns a binary input/output stream representing the socket, to receive data from the remote peer.

   @return
      Input/output stream for the connection’s socket.
   */
   _std::shared_ptr<io::binary::file_iostream> const & socket() {
      return socket_;
   }

private:
   //! Stream for the connection’s socket.
   _std::shared_ptr<io::binary::file_iostream> socket_;
   //! Local address.
   ip::address local_address_;
   //! Local port.
   ip::port local_port_;
   //! Address of the remote peer.
   ip::address remote_address_;
   //! Port of the remote peer.
   ip::port remote_port_;
};

}}} //namespace lofty::net::tcp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace tcp {

//! Accepts client connection requests for a given TCP port.
class LOFTY_SYM server : public ip::server {
public:
   /*! Constructor.

   @param address
      Address to bind to.
   @param port
      Port to listen for connections on.
   @param backlog_size
      Count of established connections that will be allowed to queue until the server is able to accept them.
   */
   server(ip::address const & address, ip::port const & port, unsigned backlog_size = 5);

   //! Destructor.
   ~server();

   /*! Accepts and returns a connection from a client.

   @return
      New client connection.
   */
   _std::shared_ptr<connection> accept();
};

}}} //namespace lofty::net::tcp

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_TCP_HXX

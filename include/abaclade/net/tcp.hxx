/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_NET_TCP_HXX
#define _ABACLADE_NET_TCP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io/binary.hxx>
#include <abaclade/net/ip.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

//! Transmission Control Protocol-related classes and facilities.
namespace tcp {}

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace tcp {

//! Initialized TCP connection.
class ABACLADE_SYM connection : public noncopyable {
public:
   /*! Constructor.

   @param fd
      Connected socket.
   @param addrRemote
      Address of the remote peer.
   @param portRemote
      Port of the remote peer.
   */
   connection(io::filedesc fd, ip::address && addrRemote, ip::port && portRemote);

   //! Destructor.
   ~connection();

   /*! Returns the address of the remote peer.

   @return
      IP address.
   */
   ip::address const & address() const {
      return m_addrRemote;
   }

   /*! Returns a binary reader/writer object representing the socket, to receive data from the
   remote peer.

   @return
      Reader/writer for the connection’s socket.
   */
   _std::shared_ptr<io::binary::file_readwriter> const & socket() {
      return m_bfrw;
   }

private:
   //! Reader/writer for the connection’s socket.
   _std::shared_ptr<io::binary::file_readwriter> m_bfrw;
   //! Address of the remote peer.
   ip::address m_addrRemote;
   //! Port of the remote peer.
   ip::port m_portRemote;
};

}}} //namespace abc::net::tcp

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace tcp {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization
required for the current thread to run coroutines.
*/
class ABACLADE_SYM server : public noncopyable {
public:
   /*! Constructor.

   @param addr
      Address to bind to.
   @param port
      Port to listen for connections on.
   @param cBacklog
      Count of established connections that will be allowed to queue until the server is able to
      accept them.
   */
   server(ip::address const & addr, ip::port const & port, unsigned cBacklog = 5);

   //! Destructor.
   ~server();

   /*! Accepts and returns a connection from a client.

   @return
      New client connection.
   */
   _std::shared_ptr<connection> accept();

private:
   /*! Creates a socket for the server.

   @param ipversion
      IP version.
   @return
      New server socket.
   */
   static io::filedesc create_socket(ip::version ipversion);

private:
   //! Server socket bound to the TCP port.
   io::filedesc m_fdSocket;
   //! IP version.
   ip::version m_ipversion;
};

}}} //namespace abc::net::tcp

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_NET_TCP_HXX

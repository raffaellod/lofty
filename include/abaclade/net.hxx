/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_NET_HXX
#define _ABACLADE_NET_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::connection

namespace abc {
namespace net {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization
required for the current thread to run coroutines.
*/
class ABACLADE_SYM connection : public noncopyable {
public:
   /*! Constructor.

   @param fd
      Connected socket.
   @param sAddress
      Client address.
   */
   connection(io::filedesc fd, smstr<45> && sAddress);

   //! Destructor.
   ~connection();

   istr const & address() const {
      return m_sAddress;
   }

   /*! Returns a binary reader to receive data from the remote peer.

   @return
      Reader for the connection’s socket.
   */
   std::shared_ptr<io::binary::reader> const & reader() {
      return m_br;
   }

   /*! Returns a binary writer to send data to the remote peer.

   @return
      Writer for the connection’s socket.
   */
   std::shared_ptr<io::binary::writer> const & writer() {
      return m_bw;
   }

private:
   //! Reader for the connection’s socket.
   std::shared_ptr<io::binary::reader> m_br;
   //! Writer for the connection’s socket.
   std::shared_ptr<io::binary::writer> m_bw;
   //! Address of the remote peer.
   smstr<45> m_sAddress;
};

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::net::tcp_server

namespace abc {
namespace net {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization
required for the current thread to run coroutines.
*/
class ABACLADE_SYM tcp_server : public noncopyable {
public:
   /*! Constructor.

   @param sAddress
      Address to bind to.
   @param iPort
      Port to listen for connections on.
   @param cBacklog
      Count of established connections that will be allowed to queue until the server is able to
      accept them.
   */
   tcp_server(istr const & sAddress, std::uint16_t iPort, unsigned cBacklog = 5);

   //! Destructor.
   ~tcp_server();

   /*! Accepts and returns a connection from a client.

   @return
      New client connection.
   */
   std::shared_ptr<connection> accept();

private:
   /*! Creates a socket for the server.

   @return
      New server socket.
   */
   static io::filedesc create_socket();

private:
   io::filedesc m_fdSocket;
};

} //namespace net
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_NET_HXX

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

namespace abc { namespace net {

//! Type of a network port.
typedef std::uint16_t port_t;

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace detail {

//! Contains an IPv4 or IPv6 address.
struct raw_ip_address {
   //! Raw bytes of an IP address.
   std::uint8_t m_abAddress[16];
   //! IP version contained in *this; 4 = IPv4, 6 = IPv6.
   std::uint8_t m_iVersion;
};

}}} //namespace abc::net::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

//! IP address.
class ABACLADE_SYM ip_address : public detail::raw_ip_address {
public:
   //! Used to indicate “any IPv4 address”, e.g. when binding to a port.
   static ip_address const & any_ipv4;
   //! Used to indicate “any IPv6 address”, e.g. when binding to a port.
   static ip_address const & any_ipv6;
   //! Type of an IPv4 address.
   typedef std::uint32_t ipv4_type;
   //! Type of an IPv6 address.
   typedef std::uint8_t ipv6_type[16];
   //! Maximum length of the string representation of an IPv4 address.
   static std::size_t const ipv4_str_size = 15 /*“255.255.255.255”*/;
   //! Maximum length of the string representation of an IPv6 address.
   static std::size_t const ipv6_str_size = 45 /*“0000:0000:0000:0000:0000:0000:255.255.255.255”*/;

public:
   /*! Constructor.

   @param iAddress
      Source address, in host endianness.
   @param abAddress
      Array of bytes to be used as the address, in host endianness.
   @param addr
      Source object.
   */
   ip_address() {
      memory::clear(m_abAddress);
      m_iVersion = 0;
   }
   explicit ip_address(std::uint32_t iAddress) {
      memory::copy(
         m_abAddress,
         reinterpret_cast<std::uint8_t const *>(&iAddress),
         sizeof m_abAddress / sizeof iAddress
      );
      m_iVersion = 4;
   }
   explicit ip_address(std::uint8_t const (& abAddress)[4]) {
      static_assert(sizeof abAddress == 4, "sizeof != 4");
      memory::copy<std::uint8_t>(m_abAddress, abAddress, sizeof abAddress);
      m_iVersion = 4;
   }
   explicit ip_address(std::uint8_t const (& abAddress)[16]) {
      static_assert(sizeof abAddress == 16, "sizeof != 16");
      memory::copy<std::uint8_t>(m_abAddress, abAddress, sizeof abAddress);
      m_iVersion = 6;
   }
   ip_address(ip_address const & addr) :
      detail::raw_ip_address(addr) {
   }

   /*! Returns a pointer to the raw address storage.

   @return
      Pointer to an array of bytes containing the address.
   */
   std::uint8_t const * raw() const {
      return m_abAddress;
   }

   /*! Returns the IP version for the address.

   @return
      IP version.
   */
   std::uint8_t version() const {
      return m_iVersion;
   }
};

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

//! Initialized TCP connection.
class ABACLADE_SYM connection : public noncopyable {
public:
   /*! Constructor.

   @param fd
      Connected socket.
   @param ipaddrRemote
      Address of the remote peer.
   */
   connection(io::filedesc fd, ip_address && ipaddrRemote, port_t portRemote);

   //! Destructor.
   ~connection();

   /*! Returns the address of the remote peer.

   @return
      IP address.
   */
   ip_address const & address() const {
      return m_ipaddrRemote;
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
   ip_address m_ipaddrRemote;
   //! Port of the remote peer.
   port_t m_portRemote;
};

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

/*! Attaches a coroutine scheduler to the current thread, and performs and necessary initialization
required for the current thread to run coroutines.
*/
class ABACLADE_SYM tcp_server : public noncopyable {
public:
   /*! Constructor.

   @param ipaddr
      Address to bind to.
   @param port
      Port to listen for connections on.
   @param cBacklog
      Count of established connections that will be allowed to queue until the server is able to
      accept them.
   */
   tcp_server(ip_address const & ipaddr, port_t port, unsigned cBacklog = 5);

   //! Destructor.
   ~tcp_server();

   /*! Accepts and returns a connection from a client.

   @return
      New client connection.
   */
   _std::shared_ptr<connection> accept();

private:
   /*! Creates a socket for the server.

   @param iIPVersion
      IP version; 4 = IPv4, 6 = IPv6.
   @return
      New server socket.
   */
   static io::filedesc create_socket(std::uint8_t iIPVersion);

private:
   //! Server socket bound to the TCP port.
   io::filedesc m_fdSocket;
   //! IP version; 4 = IPv4, 6 = IPv6.
   std::uint8_t m_iIPVersion;
};

}} //namespace abc::net

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_NET_HXX

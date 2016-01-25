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

#ifndef _ABACLADE_NET_IP_HXX
#define _ABACLADE_NET_IP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io/binary.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net {

//! Internet Protocol-related classes and facilities.
namespace ip {}

}} //namespace abc::net

namespace abc { namespace net { namespace ip {

//! IP protocol version.
ABC_ENUM(version,
   //! No specific version.
   (any, 0),
   //! Identifies IPv4.
   (v4, 4),
   //! Identifies IPv6.
   (v6, 6)
);

}}} //namespace abc::net::ip

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip { namespace detail {

//! Contains an IP port number.
struct raw_port {
   //! Port number.
   std::uint16_t m_i;
};

}}}} //namespace abc::net::ip::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip {

//! IP port.
class ABACLADE_SYM port : public detail::raw_port {
public:
   //! Type of an IP port number.
   typedef std::uint16_t type;

public:
   //! Default constructor.
   port() {
      m_i = 0;
   }

   /*! Constructor.

   @param i
      Port number.
   */
   explicit port(type i) {
      m_i = i;
   }

   /*! Returns the port number.

   @return
      Port number.
   */
   type number() const {
      return m_i;
   }
};

}}} //namespace abc::net::ip

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_str_backend<net::ip::port> : public to_str_backend<net::ip::port::type> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes an IP port, applying the formatting options.

   @param port
      Port to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(net::ip::port const & port, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip { namespace detail {

//! Contains an IPv4 or IPv6 address.
struct raw_address {
   //! Raw bytes of an IP address.
   std::uint8_t m_ab[16];
   //! IP version contained in *this.
   version::enum_type m_version;
};

}}}} //namespace abc::net::ip::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip {

//! IP address.
class ABACLADE_SYM address : public detail::raw_address {
public:
   //! Used to indicate “any IPv4 address”, e.g. when binding to a port.
   static address const & any_v4;
   //! Used to indicate “any IPv6 address”, e.g. when binding to a port.
   static address const & any_v6;
   //! Type of an IPv4 address.
   typedef std::uint8_t v4_type[4];
   //! Type of an IPv6 address.
   typedef std::uint8_t v6_type[16];
   //! Maximum length of the string representation of an IPv4 address.
   static std::size_t const v4_str_size = 15 /*“255.255.255.255”*/;
   //! Maximum length of the string representation of an IPv6 address.
   static std::size_t const v6_str_size = 45 /*“0000:0000:0000:0000:0000:0000:255.255.255.255”*/;

public:
   //! Default constructor.
   address() {
      memory::clear(m_ab);
      m_version = ip::version::any;
   }

   /*! Move constructor.

   @param addr
      Source object.
   */
   address(address const & addr) :
      detail::raw_address(addr) {
   }

   /*! Constructor. Initializes the object as an IPv4 address.

   @param ab
      Array of bytes to be used as an IPv4 address, in network order (big endian).
   */
   explicit address(v4_type const & ab) {
      memory::copy(&m_ab[0], &ab[0], sizeof(v4_type));
      m_version = ip::version::v4;
   }

   /*! Constructor. Initializes the object as an IPv6 address.

   @param ab
      Array of bytes to be used as an IPv6 address, in network order (big endian).
   */
   explicit address(v6_type const & ab) {
      memory::copy(&m_ab[0], &ab[0], sizeof(v6_type));
      m_version = ip::version::v6;
   }

   /*! Returns a pointer to the raw address storage.

   @return
      Pointer to an array of bytes containing the address.
   */
   std::uint8_t const * raw() const {
      return m_ab;
   }

   /*! Returns the IP version for the address.

   @return
      IP version.
   */
   ip::version version() const {
      return m_version;
   }
};

}}} //namespace abc::net::ip

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_str_backend<net::ip::address> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(str const & sFormat);

   /*! Writes an IP address, applying the formatting options.

   @param addr
      Address to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(net::ip::address const & addr, io::text::ostream * ptos);

protected:
   to_str_backend<char_t> m_tsbChar;
   to_str_backend<std::uint8_t> m_tsbV4Group;
   to_str_backend<std::uint16_t> m_tsbV6Group;
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_NET_IP_HXX

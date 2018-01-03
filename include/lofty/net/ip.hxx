/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_NET_IP_HXX
#define _LOFTY_NET_IP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/net.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net {

//! Internet Protocol-related classes and facilities.
namespace ip {}

}} //namespace lofty::net

namespace lofty { namespace net { namespace ip {

//! IP protocol version.
LOFTY_ENUM(version,
   //! Identifies IPv4.
   (v4, 4),
   //! Identifies IPv6.
   (v6, 6)
);

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip { namespace _pvt {

//! Contains an IP port number.
struct raw_port {
   //! Port number.
   std::uint16_t number_;
};

}}}} //namespace lofty::net::ip::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

//! IP port.
class LOFTY_SYM port : public _pvt::raw_port {
public:
   //! Type of an IP port number.
   typedef std::uint16_t number_type;

public:
   //! Default constructor.
   port() {
      number_ = 0;
   }

   /*! Constructor.

   @param number__
      Port number.
   */
   explicit port(number_type number__) {
      number_ = number__;
   }

   /*! Returns the port number.

   @return
      Port number.
   */
   number_type number() const {
      return number_;
   }
};

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<net::ip::port> : public to_text_ostream<net::ip::port::number_type> {
public:
   /*! Writes an IP port, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(net::ip::port const & src, io::text::ostream * dst) {
      to_text_ostream<net::ip::port::number_type>::write(src.number(), dst);
   }
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip { namespace _pvt {

//! Contains an IPv4 or IPv6 address.
struct raw_address {
   //! Raw bytes of an IP address.
   std::uint8_t bytes[16];
   //! IP version contained in *this.
   version::enum_type version_;
};

}}}} //namespace lofty::net::ip::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

//! IP address.
class LOFTY_SYM address : public _pvt::raw_address {
public:
   //! Used to indicate “any IPv4 address”, e.g. when binding to a port.
   static address const & any_v4;
   //! Used to indicate “any IPv6 address”, e.g. when binding to a port.
   static address const & any_v6;
   //! IPv4 address of any machine to refer to itself.
   static address const & localhost_v4;
   //! IPv6 address of any machine to refer to itself.
   static address const & localhost_v6;
   //! Type of an IPv4 address.
   typedef std::uint8_t v4_type[4];
   //! Type of an IPv6 address.
   typedef std::uint8_t v6_type[16];
   //! Maximum length of the string representation of an IPv4 address.
   static std::size_t const v4_str_size = 15 /*“255.255.255.255”*/;
   //! Maximum length of the string representation of an IPv6 address.
   static std::size_t const v6_str_size = 45 /*“0000:0000:0000:0000:0000:0000:255.255.255.255”*/;

public:
   /*! Constructs a null address for the specified IP version.

   @param version__
      IP version to use.
   */
   explicit address(ip::version version__ = ip::version::v6) {
      memory::clear(&bytes);
      version_ = version__.base();
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   address(address const & src) :
      _pvt::raw_address(src) {
   }

   /*! Constructor. Initializes the object as an IPv4 address.

   @param src_raw
      Array of bytes to be used as an IPv4 address, in network order (big endian).
   */
   explicit address(v4_type const & src_raw) {
      memory::copy(&bytes[0], &src_raw[0], sizeof(v4_type));
      version_ = ip::version::v4;
   }

   /*! Constructor. Initializes the object as an IPv6 address.

   @param src_raw
      Array of bytes to be used as an IPv6 address, in network order (big endian).
   */
   explicit address(v6_type const & src_raw) {
      memory::copy(&bytes[0], &src_raw[0], sizeof(v6_type));
      version_ = ip::version::v6;
   }

   /*! Equality relational operator.

   @param right
      Right comparand.
   @return
      true if *this and right represent the same IP address, or false otherwise.
   */
   bool operator==(address const & right) const;

   /*! Inequality relational operator.

   @param right
      Right comparand.
   @return
      true if *this and right represent different IP addresses, or false otherwise.
   */
   bool operator!=(address const & right) const {
      return !operator==(right);
   }

   /*! Returns a pointer to the raw address storage.

   @return
      Pointer to an array of bytes containing the address.
   */
   std::uint8_t const * raw() const {
      return bytes;
   }

   /*! Returns the IP version for the address.

   @return
      IP version.
   */
   ip::version version() const {
      return version_;
   }
};

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

//! Abstract server for transport layer protocols over IP.
class LOFTY_SYM server : public noncopyable {
public:
   //! Destructor.
   ~server();

protected:
   /*! Constructor.

   @param address
      Address to bind to. The IP version of this is ignored, in favor of the protocol_ argument.
   @param port
      Port to listen for connections on.
   @param protocol_
      Networking protocol.
   */
   server(address const & address, port const & port, protocol protocol_);

protected:
   //! Server socket bound to the port.
   socket sock;
   //! IP version.
   ip::version ip_version;
};

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<net::ip::address> {
public:
   //! Default constructor.
   to_text_ostream();

   //! Destructor.
   ~to_text_ostream();

   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes an IP address, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(net::ip::address const & src, io::text::ostream * dst);

protected:
   to_text_ostream<char_t> char_ttos;
   to_text_ostream<std::uint8_t> v4_group_ttos;
   to_text_ostream<std::uint16_t> v6_group_ttos;
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_NET_IP_HXX

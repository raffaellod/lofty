/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/from_str.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   net_ip_address_v4,
   "lofty::net::ip::address – IPv4 instantiation, display, and parsing"
) {
   LOFTY_TRACE_FUNC();

   #define ADDR(i, str, ...) \
      static std::uint8_t const LOFTY_CPP_CAT(addr, i, _bytes)[] = __VA_ARGS__; \
      net::ip::address LOFTY_CPP_CAT(addr, i)(LOFTY_CPP_CAT(addr, i, _bytes)); \
      static char_t const LOFTY_CPP_CAT(addr, i, _str)[] = str;
   ADDR(00, LOFTY_SL("0.0.0.0"),         { 0, 0, 0, 0 })
   ADDR(40, LOFTY_SL("1.2.3.4"),         { 1, 2, 3, 4 })
   ADDR(44, LOFTY_SL("255.255.255.255"), { 255, 255, 255, 255 })
   #undef ADDR
   LOFTY_TESTING_ASSERT_EQUAL(addr00.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(addr40.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(addr44.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr00), addr00_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr40), addr40_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr44), addr44_str);
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("2.")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".3")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("4.5")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("6.7.")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".8.9")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("10.11.12")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("13.14.15.")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".16.17.18")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("19.20.21.22.")));
   LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".23.24.25.26")));
   ///LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("100.200.300.400")));
   ///LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("256.0.0.0")));
   ///LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.256.0.0")));
   ///LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.0.256.0")));
   ///LOFTY_TESTING_ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.0.0.256")));
   LOFTY_TESTING_ASSERT_EQUAL(from_str<net::ip::address>(addr00_str), addr00);
   LOFTY_TESTING_ASSERT_EQUAL(from_str<net::ip::address>(addr40_str), addr40);
   LOFTY_TESTING_ASSERT_EQUAL(from_str<net::ip::address>(addr44_str), addr44);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   net_ip_address_v6,
   "lofty::net::ip::address – IPv6 instantiation, display, and parsing"
) {
   LOFTY_TRACE_FUNC();

   #define ADDR(i, str, ...) \
      static std::uint8_t const LOFTY_CPP_CAT(addr, i, _bytes)[] = __VA_ARGS__; \
      net::ip::address LOFTY_CPP_CAT(addr, i)(LOFTY_CPP_CAT(addr, i, _bytes)); \
      static char_t const LOFTY_CPP_CAT(addr, i, _str)[] = str;
   ADDR(00, LOFTY_SL("::"),            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(01, LOFTY_SL("1::"),           { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(02, LOFTY_SL("0:1::"),         { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(03, LOFTY_SL("0:0:1::"),       { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(04, LOFTY_SL("::1:0:0"),       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 })
   ADDR(05, LOFTY_SL("::1:0"),         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 })
   ADDR(06, LOFTY_SL("::1"),           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 })
   ADDR(07, LOFTY_SL("1::2"),          { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 })
   ADDR(08, LOFTY_SL("0:1::2"),        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 })
   ADDR(09, LOFTY_SL("1::2:0"),        { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(10, LOFTY_SL("0:1::2:0"),      { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(11, LOFTY_SL("0:1::2:0:0"),    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 })
   ADDR(12, LOFTY_SL("0:0:1::2:0"),    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(13, LOFTY_SL("::1:0:0:2:0:0"), { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 })
   ADDR(14, LOFTY_SL("0:0:0:1::"),     { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(15, LOFTY_SL("::1:0:0:0"),     { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 })
   ADDR(
      60, LOFTY_SL("102:304:506:708:90a:b0c:d0e:f10"),
      {   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16 }
   )
   ADDR(
      66, LOFTY_SL("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
   )
   #undef ADDR
   LOFTY_TESTING_ASSERT_EQUAL(addr00.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(addr60.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(addr66.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr00), addr00_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr01), addr01_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr02), addr02_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr03), addr03_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr04), addr04_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr05), addr05_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr06), addr06_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr07), addr07_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr08), addr08_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr09), addr09_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr10), addr10_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr11), addr11_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr12), addr12_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr13), addr13_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr14), addr14_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr15), addr15_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr60), addr60_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr66), addr66_str);
}

}} //namespace lofty::test

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
   ASSERT(addr00.version() == net::ip::version(net::ip::version::v4));
   ASSERT(addr40.version() == net::ip::version(net::ip::version::v4));
   ASSERT(addr44.version() == net::ip::version(net::ip::version::v4));
   ASSERT(to_str(addr00) == addr00_str);
   ASSERT(to_str(addr40) == addr40_str);
   ASSERT(to_str(addr44) == addr44_str);
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("2.")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".3")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("4.5")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("6.7.")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".8.9")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("10.11.12")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("13.14.15.")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".16.17.18")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("19.20.21.22.")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(".23.24.25.26")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("100.200.300.400")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("256.0.0.0")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.256.0.0")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.0.256.0")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0.0.0.256")));
   ASSERT(from_str<net::ip::address>(addr00_str) == addr00);
   ASSERT(from_str<net::ip::address>(addr40_str) == addr40);
   ASSERT(from_str<net::ip::address>(addr44_str) == addr44);
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
   ASSERT(addr00.version() == net::ip::version(net::ip::version::v6));
   ASSERT(addr60.version() == net::ip::version(net::ip::version::v6));
   ASSERT(addr66.version() == net::ip::version(net::ip::version::v6));
   ASSERT(to_str(addr00) == addr00_str);
   ASSERT(to_str(addr01) == addr01_str);
   ASSERT(to_str(addr02) == addr02_str);
   ASSERT(to_str(addr03) == addr03_str);
   ASSERT(to_str(addr04) == addr04_str);
   ASSERT(to_str(addr05) == addr05_str);
   ASSERT(to_str(addr06) == addr06_str);
   ASSERT(to_str(addr07) == addr07_str);
   ASSERT(to_str(addr08) == addr08_str);
   ASSERT(to_str(addr09) == addr09_str);
   ASSERT(to_str(addr10) == addr10_str);
   ASSERT(to_str(addr11) == addr11_str);
   ASSERT(to_str(addr12) == addr12_str);
   ASSERT(to_str(addr13) == addr13_str);
   ASSERT(to_str(addr14) == addr14_str);
   ASSERT(to_str(addr15) == addr15_str);
   ASSERT(to_str(addr60) == addr60_str);
   ASSERT(to_str(addr66) == addr66_str);
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("123")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("qwe")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":0")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("0:")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":2:")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":::")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:::")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":::2")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("::3::")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL(":4::")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("::5:")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("::g")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1::2::3")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:2:3::4:s:6")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:2:3::4:56789:a")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:2:3:4:5:6:7:8:9")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1::2::3")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:::3:4:5")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("1:2:3::4:5:6:7:8:9")));
   ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("::ffff:1.2.3")));
   //ASSERT_THROWS(text::syntax_error, from_str<net::ip::address>(LOFTY_SL("::ffff:256.1.2.3")));
   ASSERT(from_str<net::ip::address>(addr00_str) == addr00);
   ASSERT(from_str<net::ip::address>(addr01_str) == addr01);
   ASSERT(from_str<net::ip::address>(addr02_str) == addr02);
   ASSERT(from_str<net::ip::address>(addr03_str) == addr03);
   ASSERT(from_str<net::ip::address>(addr04_str) == addr04);
   ASSERT(from_str<net::ip::address>(addr05_str) == addr05);
   ASSERT(from_str<net::ip::address>(addr06_str) == addr06);
   ASSERT(from_str<net::ip::address>(addr07_str) == addr07);
   ASSERT(from_str<net::ip::address>(addr08_str) == addr08);
   ASSERT(from_str<net::ip::address>(addr09_str) == addr09);
   ASSERT(from_str<net::ip::address>(addr10_str) == addr10);
   ASSERT(from_str<net::ip::address>(addr11_str) == addr11);
   ASSERT(from_str<net::ip::address>(addr12_str) == addr12);
   ASSERT(from_str<net::ip::address>(addr13_str) == addr13);
   ASSERT(from_str<net::ip::address>(addr14_str) == addr14);
   ASSERT(from_str<net::ip::address>(addr15_str) == addr15);
   ASSERT(from_str<net::ip::address>(addr60_str) == addr60);
   ASSERT(from_str<net::ip::address>(addr66_str) == addr66);
}

}} //namespace lofty::test

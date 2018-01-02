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
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   net_ip_address,
   "lofty::net::ip::address – instantiation, display, and parsing"
) {
   LOFTY_TRACE_FUNC();

   #define ADDR(i, str, ...) \
      static std::uint8_t const LOFTY_CPP_CAT(addr, i, _bytes)[] = __VA_ARGS__; \
      net::ip::address LOFTY_CPP_CAT(addr, i)(LOFTY_CPP_CAT(addr, i, _bytes)); \
      static char_t const LOFTY_CPP_CAT(addr, i, _str)[] = str;
   ADDR(400, LOFTY_SL("0.0.0.0"),         { 0, 0, 0, 0 })
   ADDR(440, LOFTY_SL("1.2.3.4"),         { 1, 2, 3, 4 })
   ADDR(444, LOFTY_SL("255.255.255.255"), { 255, 255, 255, 255 })
   #undef ADDR
   LOFTY_TESTING_ASSERT_EQUAL(addr400.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(addr440.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(addr444.version(), net::ip::version(net::ip::version::v4));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr400), addr400_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr440), addr440_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr444), addr444_str);

   #define ADDR(i, str, ...) \
      static std::uint8_t const LOFTY_CPP_CAT(addr, i, _bytes)[] = __VA_ARGS__; \
      net::ip::address LOFTY_CPP_CAT(addr, i)(LOFTY_CPP_CAT(addr, i, _bytes)); \
      static char_t const LOFTY_CPP_CAT(addr, i, _str)[] = str;
   ADDR(600, LOFTY_SL("::"),            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(601, LOFTY_SL("1::"),           { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(602, LOFTY_SL("0:1::"),         { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(603, LOFTY_SL("0:0:1::"),       { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(604, LOFTY_SL("::1:0:0"),       { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 })
   ADDR(605, LOFTY_SL("::1:0"),         { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 })
   ADDR(606, LOFTY_SL("::1"),           { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 })
   ADDR(607, LOFTY_SL("1::2"),          { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 })
   ADDR(608, LOFTY_SL("0:1::2"),        { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 })
   ADDR(609, LOFTY_SL("1::2:0"),        { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(610, LOFTY_SL("0:1::2:0"),      { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(611, LOFTY_SL("0:1::2:0:0"),    { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 })
   ADDR(612, LOFTY_SL("0:0:1::2:0"),    { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 })
   ADDR(613, LOFTY_SL("::1:0:0:2:0:0"), { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 })
   ADDR(614, LOFTY_SL("0:0:0:1::"),     { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 })
   ADDR(615, LOFTY_SL("::1:0:0:0"),     { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 })
   ADDR(
      660, LOFTY_SL("102:304:506:708:90a:b0c:d0e:f10"),
      {   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16 }
   )
   ADDR(
      666, LOFTY_SL("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"),
      { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 }
   )
   #undef ADDR
   LOFTY_TESTING_ASSERT_EQUAL(addr600.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(addr660.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(addr666.version(), net::ip::version(net::ip::version::v6));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr600), addr600_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr601), addr601_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr602), addr602_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr603), addr603_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr604), addr604_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr605), addr605_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr606), addr606_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr607), addr607_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr608), addr608_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr609), addr609_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr610), addr610_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr611), addr611_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr612), addr612_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr613), addr613_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr614), addr614_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr615), addr615_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr660), addr660_str);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(addr666), addr666_str);
}

}} //namespace lofty::test

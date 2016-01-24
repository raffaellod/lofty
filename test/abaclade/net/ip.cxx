/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

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

#include <abaclade.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/net/ip.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   net_ip_address,
   "abc::net::ip::address – instantiation and display"
) {
   ABC_TRACE_FUNC(this);

   #define ADDR(i, ...) \
      static std::uint8_t const ABC_CPP_CAT(sc_abAddr, i)[] = __VA_ARGS__; \
      net::ip::address ABC_CPP_CAT(addr, i)(ABC_CPP_CAT(sc_abAddr, i));
   ADDR(400, { 0, 0, 0, 0 })
   ADDR(440, { 1, 2, 3, 4 })
   ADDR(444, { 255, 255, 255, 255 });
   #undef ADDR
   ABC_TESTING_ASSERT_EQUAL(addr400.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(addr440.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(addr444.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr400), ABC_SL("0.0.0.0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr440), ABC_SL("1.2.3.4"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr444), ABC_SL("255.255.255.255"));

   #define ADDR(i, ...) \
      static std::uint8_t const ABC_CPP_CAT(sc_abAddr, i)[] = __VA_ARGS__; \
      net::ip::address ABC_CPP_CAT(addr, i)(ABC_CPP_CAT(sc_abAddr, i));
   ADDR(600, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
   ADDR(601, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
   ADDR(602, { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
   ADDR(603, { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 });
   ADDR(604, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0 });
   ADDR(605, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0 });
   ADDR(606, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 });
   ADDR(607, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 });
   ADDR(608, { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2 });
   ADDR(609, { 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 });
   ADDR(610, { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 });
   ADDR(611, { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 });
   ADDR(612, { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0 });
   ADDR(613, { 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0 });
   ADDR(614, { 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 });
   ADDR(615, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0 });
   ADDR(660, {   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16 });
   ADDR(666, { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 });
   #undef ADDR
   ABC_TESTING_ASSERT_EQUAL(addr600.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(addr660.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(addr666.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr600), ABC_SL("::"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr601), ABC_SL("1::"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr602), ABC_SL("0:1::"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr603), ABC_SL("0:0:1::"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr604), ABC_SL("::1:0:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr605), ABC_SL("::1:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr606), ABC_SL("::1"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr607), ABC_SL("1::2"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr608), ABC_SL("0:1::2"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr609), ABC_SL("1::2:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr610), ABC_SL("0:1::2:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr611), ABC_SL("0:1::2:0:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr612), ABC_SL("0:0:1::2:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr613), ABC_SL("::1:0:0:2:0:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr614), ABC_SL("0:0:0:1::"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr615), ABC_SL("::1:0:0:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr660), ABC_SL("102:304:506:708:90a:b0c:d0e:f10"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr666), ABC_SL("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
}

}} //namespace abc::test

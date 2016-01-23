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

   static std::uint8_t const abAddr40[] = { 0, 0, 0, 0 };
   static std::uint8_t const abAddr41[] = { 255, 255, 255, 255 };
   static std::uint8_t const abAddr42[] = { 1, 2, 3, 4 };
   net::ip::address addr40(abAddr40), addr41(abAddr41), addr42(abAddr42);
   ABC_TESTING_ASSERT_EQUAL(addr40.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(addr41.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(addr42.version(), net::ip::version(net::ip::version::v4));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr40), ABC_SL("0.0.0.0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr41), ABC_SL("255.255.255.255"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr42), ABC_SL("1.2.3.4"));

   static std::uint8_t const abAddr60[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
   static std::uint8_t const abAddr61[] = {
      255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
   };
   static std::uint8_t const abAddr62[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
   net::ip::address addr60(abAddr60), addr61(abAddr61), addr62(abAddr62);
   ABC_TESTING_ASSERT_EQUAL(addr60.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(addr61.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(addr62.version(), net::ip::version(net::ip::version::v6));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr60), ABC_SL("0:0:0:0:0:0:0:0"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr61), ABC_SL("ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"));
   ABC_TESTING_ASSERT_EQUAL(to_str(addr62), ABC_SL("102:304:506:708:90a:b0c:d0e:f10"));
}

}} //namespace abc::test

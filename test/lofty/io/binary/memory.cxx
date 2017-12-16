/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/io/binary/memory.hxx>
#include <lofty/logging.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_binary_memory_stream,
   "lofty::io::binary::memory_stream – writing and reading"
) {
   LOFTY_TRACE_FUNC();

   static int const i1 = 10, i2 = 20;
   int i;
   io::binary::memory_stream mems;

   LOFTY_TESTING_ASSERT_EQUAL(mems.read(&i), 0u);

   mems.write(i1);
   LOFTY_TESTING_ASSERT_EQUAL(mems.read(&i), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(i, i1);

   mems.write(i2);
   mems.write(i1);
   LOFTY_TESTING_ASSERT_EQUAL(mems.read(&i), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(i, i2);
   LOFTY_TESTING_ASSERT_EQUAL(mems.read(&i), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(i, i1);

   LOFTY_TESTING_ASSERT_EQUAL(mems.read(&i), 0u);
}

}} //namespace lofty::test

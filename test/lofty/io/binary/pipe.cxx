/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/logging.hxx>
#include <lofty/range.hxx>
#include <lofty/testing/test_case.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   io_binary_pipe_symmetrical,
   "lofty::io::binary::pipe – alternating symmetrical writes and reads"
) {
   LOFTY_TRACE_FUNC();

   static std::size_t const buffer_size = 1024;
   _std::unique_ptr<std::uint8_t[]> src(new std::uint8_t[buffer_size]), dst(new std::uint8_t[buffer_size]);
   // Prepare the source array.
   for (std::size_t i = 0; i < buffer_size; ++i) {
      src[i] = static_cast<std::uint8_t>(i);
   }

   {
      io::binary::pipe pipe;
      LOFTY_DEFER_TO_SCOPE_END(pipe.write_end->finalize());
      // Repeatedly write the buffer to one end of the pipe, and read it back from the other end.
      LOFTY_FOR_EACH(auto copy_number, make_range(1, 5)) {
         LOFTY_UNUSED_ARG(copy_number);
         std::size_t written_bytes = pipe.write_end->write(src.get(), sizeof src[0] * buffer_size);
         LOFTY_TESTING_ASSERT_EQUAL(written_bytes, sizeof src[0] * buffer_size);
         std::size_t read_bytes = pipe.read_end->read(dst.get(), sizeof dst[0] * buffer_size);
         LOFTY_TESTING_ASSERT_EQUAL(read_bytes, written_bytes);

         // Validate the destination array.
         std::size_t errors = 0;
         for (std::size_t i = 0; i < buffer_size; ++i) {
            if (dst[i] != src[i]) {
               ++errors;
            }
            // Alter the destination so we can repeat this test.
            ++dst[i];
         }
         LOFTY_TESTING_ASSERT_EQUAL(errors, 0u);
      }
   }
}

}} //namespace lofty::test

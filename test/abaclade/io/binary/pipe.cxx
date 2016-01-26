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

#include <abaclade.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/range.hxx>
#include <abaclade/testing/test_case.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace test {

ABC_TESTING_TEST_CASE_FUNC(
   io_binary_pipe_symmetrical,
   "abc::io::binary::pipe – alternating symmetrical writes and reads"
) {
   ABC_TRACE_FUNC(this);

   static std::size_t const sc_ciBuffer = 1024;
   _std::unique_ptr<std::uint8_t[]> aiSrc(new std::uint8_t[sc_ciBuffer]),
                                    aiDst(new std::uint8_t[sc_ciBuffer]);
   // Prepare the source array.
   for (std::size_t i = 0; i < sc_ciBuffer; ++i) {
      aiSrc[i] = static_cast<std::uint8_t>(i);
   }

   {
      auto pe(io::binary::pipe());
      auto deferred1(defer_to_scope_end([&pe] () {
         pe.ostream->finalize();
      }));
      // Repeatedly write the buffer to one end of the pipe, and read it back from the other end.
      ABC_FOR_EACH(auto iCopy, make_range(1, 5)) {
         ABC_UNUSED_ARG(iCopy);
         std::size_t cbWritten = pe.ostream->write(aiSrc.get(), sizeof aiSrc[0] * sc_ciBuffer);
         ABC_TESTING_ASSERT_EQUAL(cbWritten, sizeof aiSrc[0] * sc_ciBuffer);
         std::size_t cbRead = pe.istream->read(aiDst.get(), sizeof aiDst[0] * sc_ciBuffer);
         ABC_TESTING_ASSERT_EQUAL(cbRead, cbWritten);

         // Validate the destination array.
         std::size_t cErrors = 0;
         for (std::size_t i = 0; i < sc_ciBuffer; ++i) {
            if (aiDst[i] != aiSrc[i]) {
               ++cErrors;
            }
            // Alter the destination so we can repeat this test.
            ++aiDst[i];
         }
         ABC_TESTING_ASSERT_EQUAL(cErrors, 0u);
      }
   }
}

}} //namespace abc::test

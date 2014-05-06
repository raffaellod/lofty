/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/testing/test_case.hxx>
#include <abc/trace.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::str_ostream

namespace abc {

namespace test {

class str_ostream_basic :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::io::str_ostream - basic operations"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      io::str_ostream sos;

#ifdef U8SL
      ABC_TESTING_ASSERT_EQUAL(
         (sos.write(istr8(U8SL("I/O stream test from UTF-8 string"))), sos.release_content()),
         SL("I/O stream test from UTF-8 string")
      );
#endif
#ifdef U16SL
      ABC_TESTING_ASSERT_EQUAL(
         (sos.write(istr16(U16SL("I/O stream test from UTF-16 string"))), sos.release_content()),
         SL("I/O stream test from UTF-16 string")
      );
#endif
#ifdef U32SL
      ABC_TESTING_ASSERT_EQUAL(
         (sos.write(istr32(U32SL("I/O stream test from UTF-32 string"))), sos.release_content()),
         SL("I/O stream test from UTF-32 string")
      );
#endif
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::str_ostream_basic)


////////////////////////////////////////////////////////////////////////////////////////////////////


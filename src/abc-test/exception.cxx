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
// abc::test::exception_polymorphism

namespace abc {

namespace test {

class exception_polymorphism :
   public testing::test_case {
protected:

   /** First-level abc::generic_error subclass.
   */
   class derived1_error :
      public virtual generic_error {
   public:

      /** Constructor.
      */
      derived1_error() :
         generic_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived1_error";
      }
   };


   /** Second-level abc::generic_error subclass.
   */
   class derived2_error :
      public virtual derived1_error {
   public:

      /** Constructor.
      */
      derived2_error() :
         derived1_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived2_error";
      }
   };


   /** Diamond-inheritance abc::generic_error subclass.
   */
   class derived3_error :
      public virtual derived1_error,
      public virtual derived2_error {
   public:

      /** Constructor.
      */
      derived3_error() :
         derived1_error(),
         derived2_error() {
         m_pszWhat = "abc::test::exception_polymorphism::derived3_error";
      }
   };


public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::exception - polymorphism"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      ABC_TESTING_ASSERT_THROWS(exception, throw_exception());
      ABC_TESTING_ASSERT_THROWS(generic_error, throw_generic_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived1_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived2_error());
      ABC_TESTING_ASSERT_THROWS(derived2_error, throw_derived2_error());
      ABC_TESTING_ASSERT_THROWS(derived1_error, throw_derived3_error(2351));
      ABC_TESTING_ASSERT_THROWS(derived2_error, throw_derived3_error(3512));
      ABC_TESTING_ASSERT_THROWS(derived3_error, throw_derived3_error(5123));
   }


   void throw_exception() {
      ABC_TRACE_FN((this));

      ABC_THROW(exception, ());
   }


   void throw_generic_error() {
      ABC_TRACE_FN((this));

      ABC_THROW(generic_error, ());
   }


   void throw_derived1_error() {
      ABC_TRACE_FN((this));

      ABC_THROW(derived1_error, ());
   }


   void throw_derived2_error() {
      ABC_TRACE_FN((this));

      ABC_THROW(derived2_error, ());
   }


   void throw_derived3_error(int i) {
      ABC_TRACE_FN((this, i));

      ABC_THROW(derived3_error, ());
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_polymorphism)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::test::exception_from_os_hard_error

namespace abc {

namespace test {

class exception_from_os_hard_error :
   public testing::test_case {
public:

   /** See testing::test_case::title().
   */
   virtual istr title() {
      return istr(SL("abc::exception - conversion of hard OS errors into C++ exceptions"));
   }


   /** See testing::test_case::run().
   */
   virtual void run() {
      ABC_TRACE_FN((this));

      {
         int * p(nullptr);
         ABC_TESTING_ASSERT_THROWS(null_pointer_error, *p = 1);

         // Under POSIX, this also counts as second test for SIGSEGV, checking that the handler is
         // still in place after its first activation above.
         ABC_TESTING_ASSERT_THROWS(memory_address_error, *++p = 1);
      }

      // Enable alignment checking if the architecture supports it.
#ifdef ABC_ALIGN_CHECK
#ifdef __GNUC__
   #if defined(__i386__)
      __asm__(
         "pushf\n"
         "orl $0x00040000,(%esp)\n"
         "popf"
      );
      #define ABC_ALIGN_CHECK
   #elif defined(__x86_64__)
      __asm__(
         "pushf\n"
         "orl $0x0000000000040000,(%rsp)\n"
         "popf"
      );
      #define ABC_ALIGN_CHECK
   #endif
#endif

      {
         // Create an int (with another one following it) and a pointer to it.
         int i[2];
         void * p(&i[0]);
         // Misalign the pointer, partly entering the second int.
         p = static_cast<int8_t *>(p) + 1;
         ABC_TESTING_ASSERT_THROWS(memory_access_error, *static_cast<int *>(p) = 1);
      }

      // Disable alignment checking back.
#ifdef __GNUC__
   #if defined(__i386__)
      __asm__(
         "pushf\n"
         "andl $0xfffbffff,(%esp)\n"
         "popf"
      );
   #elif defined(__x86_64__)
      __asm__(
         "pushf\n"
         "andl $0xfffffffffffbffff,(%rsp)\n"
         "popf"
      );
   #endif
#endif
#endif //ifdef ABC_ALIGN_CHECK

      {
         // Non-obvious division by zero that can’t be detected at compile time: sEmpty always has
         // a NUL terminator at its end, so use that as a zero divider.
         istr sEmpty;
         int iZero(sEmpty[0]), iOne(1);
         ABC_TESTING_ASSERT_THROWS(division_by_zero_error, iOne /= iZero);
         // The call to istr::format() makes use of the quotient, so it shouldn’t be optimized away.
         istr(SL("{}")).format(iOne);
      }
   }
};

} //namespace test

} //namespace abc

ABC_TESTING_REGISTER_TEST_CASE(abc::test::exception_from_os_hard_error)


////////////////////////////////////////////////////////////////////////////////////////////////////


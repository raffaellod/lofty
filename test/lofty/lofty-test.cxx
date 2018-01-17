/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/io/binary/memory.hxx>
#include <lofty/logging.hxx>
#include <lofty/math.hxx>
#include <lofty/os.hxx>
#include <lofty/range.hxx>
#include <lofty/testing/app.hxx>
#include <lofty/testing/test_case.hxx>
#include <lofty/text/char_ptr_to_str_adapter.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_str.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

LOFTY_APP_CLASS(lofty::testing::app)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

static_assert(!_std::is_copy_constructible<_std::unique_ptr<int>>::value, "unique");
static_assert(_std::is_copy_constructible<_std::shared_ptr<int>>::value, "shared");

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

/* A coroutine_local variable, being specific to a thread and a coroutine, by definition does not need to be
atomic; however this test case wants to find out if the variable is accidentally shared among multiple threads
or coroutines, and making the value not atomic could hide the problem. So atomic it is. */
static coroutine_local_value<_std::atomic<int>> coroutine_local_int /*= 0*/;

LOFTY_TESTING_TEST_CASE_FUNC(
   coroutine_local_basic,
   "lofty::coroutine_local_* – basic functionality"
) {
   LOFTY_TRACE_FUNC();

   coroutine_local_int.get().store(10);
   thread thread1([this] () {
      LOFTY_TRACE_FUNC();

      coroutine_local_int.get().store(11);
   });
   coroutine coro1([this] () {
      LOFTY_TRACE_FUNC();

      coroutine_local_int.get().store(21);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 21);
   });
   coroutine coro2([this] () {
      LOFTY_TRACE_FUNC();

      coroutine_local_int.get().store(22);
      // Yield to another coroutine.
      this_thread::sleep_for_ms(1);
      LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 22);
   });
   this_thread::run_coroutines();
   // Ensure the .store() in the other thread has taken place after this line.
   thread1.join();

   LOFTY_TESTING_ASSERT_EQUAL(coroutine_local_int.get().load(), 10);

   // Avoid running other tests with a coroutine scheduler, as it might change their behavior.
   this_thread::detach_coroutine_scheduler();
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   defer_to_scope_end_basic,
   "LOFTY_DEFER_TO_SCOPE_END() – basic operation"
) {
   LOFTY_TRACE_FUNC();

   unsigned deferred_invocations = 0;
   {
      LOFTY_DEFER_TO_SCOPE_END(++deferred_invocations);
   }
   LOFTY_TESTING_ASSERT_EQUAL(deferred_invocations, 1u);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

namespace {

LOFTY_ENUM(test_enum,
   (value1, 15),
   (value2, 56),
   (value3, 91)
);

} //namespace

LOFTY_TESTING_TEST_CASE_FUNC(
   enum_basic,
   "LOFTY_ENUM() and similar – basic operation"
) {
   LOFTY_TRACE_FUNC();

   test_enum e(test_enum::value2);

   LOFTY_TESTING_ASSERT_TRUE(e == test_enum::value2);
   LOFTY_TESTING_ASSERT_EQUAL(to_str(e), LOFTY_SL("value2"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

namespace {

LOFTY_ENUM_AUTO_VALUES(auto_enum_test,
   value0,
   value1,
   value2
);

} //namespace

LOFTY_TESTING_TEST_CASE_FUNC(
   enum_auto_values,
   "LOFTY_ENUM_AUTO_VALUES() – generated member values"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value0), 0);
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value1), 1);
   LOFTY_TESTING_ASSERT_EQUAL(static_cast<int>(auto_enum_test::value2), 2);
}

}} //namespace lofty::test

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
         std::size_t written_size = pipe.write_end->write(src.get(), buffer_size);
         LOFTY_TESTING_ASSERT_EQUAL(written_size, buffer_size);
         std::size_t read_size = pipe.read_end->read(dst.get(), buffer_size);
         LOFTY_TESTING_ASSERT_EQUAL(read_size, written_size);

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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   os_registry,
   "lofty::os – accessing Windows Registry"
) {
   LOFTY_TRACE_FUNC();

   sstr<8> s;
   collections::vector<str> v;

   LOFTY_TESTING_ASSERT_FALSE(os::registry::get_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("non-existent key"), str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_FALSE(os::registry::get_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("Software\\Classes\\Interface"), str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_FALSE(os::registry::get_value(
      HKEY_LOCAL_MACHINE, LOFTY_SL("Software"), LOFTY_SL("non-existent value"), s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, str::empty);

   LOFTY_TESTING_ASSERT_TRUE(os::registry::get_value(
      HKEY_LOCAL_MACHINE,
      LOFTY_SL("Software\\Classes\\Interface\\{00000000-0000-0000-c000-000000000046}"),
      str::empty, s.str_ptr()
   ));
   LOFTY_TESTING_ASSERT_EQUAL(s, LOFTY_SL("IUnknown"));

   /* Unfortunately, REG_MULTI_SZ values are rare, and this is the only one I can imagine would work on most
   computers. However, most is not all, so it stays disabled. */
#if 0
   LOFTY_TESTING_ASSERT_TRUE(os::registry::get_value(
      HKEY_LOCAL_MACHINE,
      LOFTY_SL("System\\CurrentControlSet\\Services\\TcpIp\\Linkage"),
      LOFTY_SL("Bind"), &v
   ));
   LOFTY_TESTING_ASSERT_TRUE(v);
   // Also, can’t assert on the actual values since they’re UUIDs.
#endif
}

}} //namespace lofty::test

#endif //if LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   range_basic,
   "lofty::range – basic operation"
) {
   LOFTY_TRACE_FUNC();

   range<int> range1;
   LOFTY_TESTING_ASSERT_EQUAL(range1.size(), 0u);
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(-1));
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(0));
   LOFTY_TESTING_ASSERT_FALSE(range1.contains(1));

   range<int> range2(1, 2);
   LOFTY_TESTING_ASSERT_EQUAL(range2.size(), 1u);
   LOFTY_TESTING_ASSERT_EQUAL(*range2.begin(), 1);
   LOFTY_TESTING_ASSERT_FALSE(range2.contains(0));
   LOFTY_TESTING_ASSERT_TRUE(range2.contains(1));
   LOFTY_TESTING_ASSERT_FALSE(range2.contains(2));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

/* A thread_local variable, being specific to a thread, by definition does not need to be atomic; however this
test case wants to find out if the variable is accidentally shared among multiple threads, and making the
value not atomic could hide the problem. So atomic it is. */
static thread_local_value<_std::atomic<int>> thread_local_int /*= 0*/;

LOFTY_TESTING_TEST_CASE_FUNC(
   thread_local_basic,
   "lofty::thread_local_* – basic functionality"
) {
   LOFTY_TRACE_FUNC();

   thread_local_int.get().store(10);
   thread thread1([this] () {
      LOFTY_TRACE_FUNC();

      thread_local_int.get().store(11);
   });
   // Ensure the .store() in the other thread has taken place after this line.
   thread1.join();

   LOFTY_TESTING_ASSERT_EQUAL(thread_local_int.get().load(), 10);
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   text_char_ptr_to_str_adapter,
   "lofty::to_str – lofty::text::char_ptr_to_str_adapter"
) {
   LOFTY_TRACE_FUNC();

   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter(nullptr)), LOFTY_SL("<nullptr>"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("")), LOFTY_SL(""));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("a")), LOFTY_SL("a"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab")), LOFTY_SL("ab"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("abc")), LOFTY_SL("abc"));
   LOFTY_TESTING_ASSERT_EQUAL(to_str(text::char_ptr_to_str_adapter("ab\0c")), LOFTY_SL("ab"));
}

}} //namespace lofty::test

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace test {

LOFTY_TESTING_TEST_CASE_FUNC(
   _pvt_signal_dispatcher_os_errors_to_cxx_exceptions,
   "lofty::_pvt::signal_dispatcher – conversion of synchronous OS errors into C++ exceptions"
) {
   LOFTY_TRACE_FUNC();

   // Validate generation of invalid pointer dereference errors.
   {
      int * p = nullptr;
      LOFTY_TESTING_ASSERT_THROWS(memory::bad_pointer, *p = 1);
      // Check that the handler is still in place after its first activation above.
      LOFTY_TESTING_ASSERT_THROWS(memory::bad_pointer, *p = 2);

      LOFTY_TESTING_ASSERT_THROWS(memory::bad_pointer, *++p = 1);
   }

   // Validate generation of other pointer dereference errors.
   {
#if 0 // LOFTY_HOST_ARCH_???
      // Enable alignment checking if the architecture supports it.

      // Create an int (with another one following it) and a pointer to it.
      int i[2];
      void * p = &i[0];
      // Misalign the pointer, partly entering the second int.
      p = static_cast<std::int8_t *>(p) + 1;
      LOFTY_TESTING_ASSERT_THROWS(memory::bad_pointer_alignment, *static_cast<int *>(p) = 1);
#endif
   }

   // Validate generation of arithmetic errors.
   {
      // Non-obvious division by zero that can’t be detected at compile time.
      str empty;
      int zero = static_cast<int>(empty.size_in_chars()), one = 1;
      LOFTY_TESTING_ASSERT_THROWS(math::division_by_zero, one /= zero);
      // Use the quotient, so it won’t be optimized away.
      to_str(one);
   }
}

}} //namespace lofty::test

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/testing/test_case.hxx>

#if ABC_HOST_API_POSIX
#include <time.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case

namespace abc {
namespace testing {

namespace {

#if ABC_HOST_API_POSIX

std::pair<bool, ::clockid_t> get_timer_clock() {
   ::clockid_t clkid;
   // Try and get a timer specific to this process.
   if (::clock_getcpuclockid(0, &clkid) == 0) {
      return std::make_pair(true, std::move(clkid));
   }
#if defined(CLOCK_PROCESS_CPUTIME_ID)
   return std::make_pair(true, CLOCK_PROCESS_CPUTIME_ID);
#endif //if defined(CLOCK_PROCESS_CPUTIME_ID)
   // No suitable timer to use.
   return std::make_pair(false, ::clockid_t());
}

::timespec get_time_point() {
   auto clkidTimer = get_timer_clock();
   if (!clkidTimer.first) {
      // No suitable timer to use.
      // TODO: do something other than throw.
      throw 0;
   }
   ::timespec tsRet;
   ::clock_gettime(clkidTimer.second, &tsRet);
   return std::move(tsRet);
}

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

::FILETIME get_time_point() {
   ::FILETIME ftRet, ftUnused;
   ::GetProcessTimes(::GetCurrentProcess(), &ftUnused, &ftUnused, &ftUnused, &ftRet);
   return std::move(ftRet);
}

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32

   // We could probably just use std::chrono here.
   #error "TODO: HOST_API"

#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

} //namespace


test_case::test_case() {
}

/*virtual*/ test_case::~test_case() {
}

void test_case::init(runner * prunner) {
   ABC_TRACE_FUNC(this, prunner);

   m_prunner = prunner;
}

void test_case::assert_does_not_throw(
   source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr
) {
   ABC_TRACE_FUNC(this, srcloc, /*fnExpr, */sExpr);

   istr sCaughtWhat;
   try {
      fnExpr();
   } catch (::std::exception const & x) {
      sCaughtWhat = istr(ABC_SL("throws {}")).format(x.what());
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      srcloc, !sCaughtWhat, sExpr, istr::empty, ABC_SL("does not throw"), sCaughtWhat
   );
}

void test_case::assert_false(source_location const & srcloc, bool bActual, istr const & sExpr) {
   ABC_TRACE_FUNC(this, srcloc, bActual, sExpr);

   m_prunner->log_assertion(
      srcloc, !bActual, sExpr, istr::empty, !bActual ? istr::empty : ABC_SL("false"), ABC_SL("true")
   );
}

void test_case::assert_true(source_location const & srcloc, bool bActual, istr const & sExpr) {
   ABC_TRACE_FUNC(this, srcloc, bActual, sExpr);

   m_prunner->log_assertion(
      srcloc, bActual, sExpr, istr::empty, bActual ? istr::empty : ABC_SL("true"), ABC_SL("false")
   );
}

void test_case::assert_throws(
   source_location const & srcloc, std::function<void ()> const & fnExpr, istr const & sExpr,
   std::function<bool (std::exception const &)> const & fnMatchType, char const * pszExpectedWhat
) {
   ABC_TRACE_FUNC(this, srcloc, /*fnExpr, */sExpr, /*fnMatchType, */pszExpectedWhat);

   bool bPass = false;
   istr sCaughtWhat;
   try {
      fnExpr();
      sCaughtWhat = ABC_SL("does not throw");
   } catch (::std::exception const & x) {
      if (fnMatchType(x)) {
         bPass = true;
      } else {
         sCaughtWhat = istr(ABC_SL("throws {}")).format(char_ptr_to_str_adapter(x.what()));
      }
   } catch (...) {
      sCaughtWhat = ABC_SL("unknown type");
   }
   m_prunner->log_assertion(
      srcloc, bPass, sExpr, istr::empty,
      istr(ABC_SL("throws {}")).format(char_ptr_to_str_adapter(pszExpectedWhat)), sCaughtWhat
   );
}

void test_case::timer_start(istr const & sTimerTitle) {
   ABC_TRACE_FUNC(this, sTimerTitle);

   m_sTimer = sTimerTitle;
   auto timepoint(get_time_point());
   typedef decltype(timepoint) timepoint_t;
   if (!m_pStartTime) {
      m_pStartTime = memory::alloc<timepoint_t>();
   }
   *static_cast<timepoint_t *>(m_pStartTime.get()) = timepoint;
}

void test_case::timer_stop() {
#if ABC_HOST_API_POSIX
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   // We could probably just use std::chrono here.
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

   // We do this here to avoid adding ABC_TRACE_FUNC() to the timed execution.
   ABC_TRACE_FUNC(this);


}

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::test_case_factory_impl

namespace abc {
namespace testing {

ABC_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(test_case_factory_list)

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

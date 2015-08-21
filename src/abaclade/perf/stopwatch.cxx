/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
#include <abaclade/perf/stopwatch.hxx>

#if ABC_HOST_API_POSIX
   #include <time.h>
   #include <unistd.h>
#endif
#if ABC_HOST_API_DARWIN
   #include <mach/mach_time.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace perf {

#if ABC_HOST_API_POSIX && defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

typedef ::timespec timepoint_t;

static _std::tuple<bool, ::clockid_t> get_timer_clock() {
   ::clockid_t clkid;
   // Try and get a timer specific to this process.
   if (::clock_getcpuclockid(0, &clkid) == 0) {
      return _std::make_tuple(true, _std::move(clkid));
   }
#if defined(CLOCK_PROCESS_CPUTIME_ID)
   return _std::make_tuple(true, CLOCK_PROCESS_CPUTIME_ID);
#endif //if defined(CLOCK_PROCESS_CPUTIME_ID)
   // No suitable timer to use.
   return _std::make_tuple(false, ::clockid_t());
}

static ::timespec get_time_point() {
   auto clkidTimer = get_timer_clock();
   if (!_std::get<0>(clkidTimer)) {
      // No suitable timer to use.
      // TODO: do something other than throw.
      throw 0;
   }
   ::timespec tsRet;
   ::clock_gettime(_std::get<1>(clkidTimer), &tsRet);
   return _std::move(tsRet);
}

static stopwatch::duration_type get_duration_ns(
   ::timespec const & tsBegin, ::timespec const & tsEnd
) {
   ABC_TRACE_FUNC();

   typedef stopwatch::duration_type duration_type;
   duration_type iInterval = static_cast<duration_type>(tsEnd.tv_sec - tsBegin.tv_sec) * 1000000000;
   iInterval += static_cast<duration_type>(tsEnd.tv_nsec);
   iInterval -= static_cast<duration_type>(tsBegin.tv_nsec);
   return static_cast<duration_type>(iInterval);
}

#elif ABC_HOST_API_DARWIN //if ABC_HOST_API_POSIX

typedef std::uint64_t timepoint_t;

static std::uint64_t get_time_point() {
   return ::mach_absolute_time();
}

static stopwatch::duration_type get_duration_ns(std::uint64_t iBegin, std::uint64_t iEnd) {
   ::mach_timebase_info_data_t mtid;
   ::mach_timebase_info(&mtid);
   return (iEnd - iBegin) * mtid.numer / mtid.denom;
}

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX … elif ABC_HOST_API_DARWIN

typedef ::FILETIME timepoint_t;

static ::FILETIME get_time_point() {
   ::FILETIME ftRet, ftUnused;
   ::GetProcessTimes(::GetCurrentProcess(), &ftUnused, &ftUnused, &ftUnused, &ftRet);
   return _std::move(ftRet);
}

static stopwatch::duration_type get_duration_ns(
   ::FILETIME const & ftBegin, ::FILETIME const & ftEnd
) {
   ABC_TRACE_FUNC();

   // Compose the FILETIME arguments into 64-bit integers.
   ::ULARGE_INTEGER iBegin, iEnd;
   iBegin.LowPart = ftBegin.dwLowDateTime;
   iBegin.HighPart = ftBegin.dwHighDateTime;
   iEnd.LowPart = ftEnd.dwLowDateTime;
   iEnd.HighPart = ftEnd.dwHighDateTime;
   // FILETIME is in units of 100 ns, so scale it to 1 ns.
   return (iEnd.QuadPart - iBegin.QuadPart) * 100;
}

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_DARWIN … elif ABC_HOST_API_WIN32

   // We could probably just use std::chrono here.
   #error "TODO: HOST_API"

#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_DARWIN … elif ABC_HOST_API_WIN32 … else


stopwatch::stopwatch() :
   m_pStartTime(new timepoint_t),
   m_iTotalDuration(0) {
}
stopwatch::stopwatch(stopwatch const & sw) :
   m_pStartTime(new timepoint_t(*static_cast<timepoint_t const *>(sw.m_pStartTime.get()))),
   m_iTotalDuration(sw.m_iTotalDuration) {
}

stopwatch::~stopwatch() {
}

stopwatch & stopwatch::operator=(stopwatch const & sw) {
   *static_cast<timepoint_t *>(m_pStartTime.get()) = *static_cast<timepoint_t const *>(
      sw.m_pStartTime.get()
   );
   m_iTotalDuration = sw.m_iTotalDuration;
   return *this;
}

void stopwatch::start() {
   ABC_TRACE_FUNC(this);

   timepoint_t timepoint(get_time_point());
   *static_cast<timepoint_t *>(m_pStartTime.get()) = _std::move(timepoint);
}

stopwatch::duration_type stopwatch::stop() {
   auto timepoint(get_time_point());

   // We do this here to avoid adding ABC_TRACE_FUNC() to the timed execution.
   ABC_TRACE_FUNC(this);

   duration_type iPartialDuration = get_duration_ns(
      *static_cast<timepoint_t *>(m_pStartTime.get()), _std::move(timepoint)
   );
   m_iTotalDuration += iPartialDuration;
   return iPartialDuration;
}

}} //namespace abc::perf

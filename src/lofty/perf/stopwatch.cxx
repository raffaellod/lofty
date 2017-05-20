/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/perf/stopwatch.hxx>

#if LOFTY_HOST_API_POSIX
   #include <time.h>
   #include <unistd.h>
#endif
#if LOFTY_HOST_API_DARWIN
   #include <mach/mach_time.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace perf {

#if LOFTY_HOST_API_POSIX && defined(_POSIX_TIMERS) && _POSIX_TIMERS > 0

typedef ::timespec timepoint_t;

static _std::tuple<bool, ::clockid_t> get_timer_clock() {
   ::clockid_t clock_id;
   // Try and get a timer specific to this process.
   if (::clock_getcpuclockid(0, &clock_id) == 0) {
      return _std::make_tuple(true, _std::move(clock_id));
   }
#if defined(CLOCK_PROCESS_CPUTIME_ID)
   return _std::make_tuple(true, CLOCK_PROCESS_CPUTIME_ID);
#endif //if defined(CLOCK_PROCESS_CPUTIME_ID)
   // No suitable timer to use.
   return _std::make_tuple(false, ::clockid_t());
}

static timepoint_t get_timepoint() {
   auto timer_clock = get_timer_clock();
   if (!_std::get<0>(timer_clock)) {
      // No suitable timer to use.
      // TODO: do something other than throw.
      throw 0;
   }
   ::timespec ret;
   ::clock_gettime(_std::get<1>(timer_clock), &ret);
   return _std::move(ret);
}

static stopwatch::duration_type get_duration_ns(::timespec const & begin, ::timespec const & end) {
   LOFTY_TRACE_FUNC();

   typedef stopwatch::duration_type duration_type;
   duration_type interval = static_cast<duration_type>(end.tv_sec - begin.tv_sec) * 1000000000;
   interval += static_cast<duration_type>(end.tv_nsec);
   interval -= static_cast<duration_type>(begin.tv_nsec);
   return static_cast<duration_type>(interval);
}

#elif LOFTY_HOST_API_DARWIN //if LOFTY_HOST_API_POSIX

typedef std::uint64_t timepoint_t;

static timepoint_t get_timepoint() {
   return ::mach_absolute_time();
}

static stopwatch::duration_type get_duration_ns(std::uint64_t begin, std::uint64_t end) {
   ::mach_timebase_info_data_t timebase_info;
   ::mach_timebase_info(&timebase_info);
   return (end - begin) * timebase_info.numer / timebase_info.denom;
}

#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_DARWIN

typedef ::FILETIME timepoint_t;

static timepoint_t get_timepoint() {
   ::FILETIME ret, unused;
   ::GetProcessTimes(::GetCurrentProcess(), &unused, &unused, &unused, &ret);
   return _std::move(ret);
}

static stopwatch::duration_type get_duration_ns(::FILETIME const & begin, ::FILETIME const & end) {
   LOFTY_TRACE_FUNC();

   // Compose the FILETIME arguments into 64-bit integers.
   ::ULARGE_INTEGER begin_int, end_int;
   begin_int.LowPart = begin.dwLowDateTime;
   begin_int.HighPart = begin.dwHighDateTime;
   end_int.LowPart = end.dwLowDateTime;
   end_int.HighPart = end.dwHighDateTime;
   // FILETIME is in units of 100 ns, so scale it to 1 ns.
   return (end_int.QuadPart - begin_int.QuadPart) * 100;
}

#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_DARWIN … elif LOFTY_HOST_API_WIN32

   // We could probably just use std::chrono here.
   #error "TODO: HOST_API"

#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_DARWIN … elif LOFTY_HOST_API_WIN32 … else


stopwatch::stopwatch() :
   start_time(new timepoint_t),
   total_duration(0) {
}
stopwatch::stopwatch(stopwatch const & src) :
   start_time(new timepoint_t(*static_cast<timepoint_t const *>(src.start_time.get()))),
   total_duration(src.total_duration) {
}

stopwatch::~stopwatch() {
}

stopwatch & stopwatch::operator=(stopwatch const & src) {
   *static_cast<timepoint_t *>(start_time.get()) = *static_cast<timepoint_t const *>(src.start_time.get());
   total_duration = src.total_duration;
   return *this;
}

void stopwatch::start() {
   LOFTY_TRACE_FUNC(this);

   timepoint_t timepoint(get_timepoint());
   *static_cast<timepoint_t *>(start_time.get()) = _std::move(timepoint);
}

stopwatch::duration_type stopwatch::stop() {
   auto timepoint(get_timepoint());

   // We do this here to avoid adding LOFTY_TRACE_FUNC() to the timed execution.
   LOFTY_TRACE_FUNC(this);

   duration_type last_duration = get_duration_ns(
      *static_cast<timepoint_t *>(start_time.get()), _std::move(timepoint)
   );
   total_duration += last_duration;
   return last_duration;
}

}} //namespace lofty::perf

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_PERF_STOPWATCH_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_PERF_STOPWATCH_HXX
#endif

#ifndef _LOFTY_PERF_STOPWATCH_HXX_NOPUB
#define _LOFTY_PERF_STOPWATCH_HXX_NOPUB

#include <lofty/memory.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace perf {
_LOFTY_PUBNS_BEGIN

//! Measures processing time intervals for the current process at a high platform-dependent precision.
class LOFTY_SYM stopwatch {
public:
   //! Integer type used to measure durations in nanoseconds.
   typedef std::uint64_t duration_type;

public:
   //! Default constructor.
   stopwatch();

   /*! Copy constructor.

   @param src
      Source object.
   */
   stopwatch(stopwatch const & src);

   //! Destructor.
   ~stopwatch();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   stopwatch & operator=(stopwatch const & src);

   /*! Returns the total tracked time.

   @return
      Cumulative time counted by start()/stop() call pairs, in nanoseconds.
   */
   duration_type duration() const {
      return total_duration;
   }

   //! Starts tracking time.
   void start();

   /*! Stops tracking time.

   @return
      Time elapsed since the last call to start(), in nanoseconds.
   */
   duration_type stop();

protected:
   //! Pointer to the start time of the current timed session.
   _std::_LOFTY_PUBNS unique_ptr<void, memory::_LOFTY_PUBNS freeing_deleter> start_time;
   //! Total measured time duration, in nanoseconds. Precision is not guaranteed on all platforms.
   duration_type total_duration;
};

_LOFTY_PUBNS_END
}} //namespace lofty::perf

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class to_text_ostream<perf::_LOFTY_PUBNS stopwatch> :
   public to_text_ostream<perf::_LOFTY_PUBNS stopwatch::duration_type> {
public:
   /*! Writes a stopwatch by its duration in ns, applying the formatting options.

   @param src
      Stopwatch to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(perf::_LOFTY_PUBNS stopwatch const & src, io::text::_LOFTY_PUBNS ostream * dst) {
      to_text_ostream<perf::_pub::stopwatch::duration_type>::write(src.duration(), dst);
   }
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_PERF_STOPWATCH_HXX_NOPUB

#ifdef _LOFTY_PERF_STOPWATCH_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace perf {

   using _pub::stopwatch;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_PERF_STOPWATCH_HXX

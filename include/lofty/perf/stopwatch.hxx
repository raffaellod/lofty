/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_PERF_STOPWATCH_HXX
#define _LOFTY_PERF_STOPWATCH_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace perf {

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
   _std::unique_ptr<void, memory::freeing_deleter> start_time;
   //! Total measured time duration, in nanoseconds. Precision is not guaranteed on all platforms.
   duration_type total_duration;
};

}} //namespace lofty::perf

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class to_text_ostream<perf::stopwatch> : public to_text_ostream<perf::stopwatch::duration_type> {
public:
   /*! Writes a stopwatch by its duration in ns, applying the formatting options.

   @param src
      Stopwatch to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(perf::stopwatch const & src, io::text::ostream * dst) {
      to_text_ostream<perf::stopwatch::duration_type>::write(src.duration(), dst);
   }
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_PERF_STOPWATCH_HXX

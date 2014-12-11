/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#ifndef _ABACLADE_PERF_STOPWATCH_HXX
#define _ABACLADE_PERF_STOPWATCH_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::perf::stopwatch

namespace abc {
namespace perf {

/*! Measures processing time intervals for the current process at a high platform-dependent
precision. */
class ABACLADE_TESTING_SYM stopwatch {
public:
   //! Integer type used to measure durations.
   typedef std::uint64_t duration_type;

public:
   //! Constructor.
   stopwatch();

   //! Destructor.
   ~stopwatch();

   /*! Returns the total tracked time.

   @return
      Cumulative time counted by start()/stop() call pairs.
   */
   duration_type duration() const {
      return m_iTotalDuration;
   }

   //! Starts tracking time.
   void start();

   /*! Stops tracking time.

   @return
      Time elapsed since the last call to start().
   */
   duration_type stop();

protected:
   /*! Start time of the current timed session. Large enough to accommodate the real type, defined
   in stopwatch.cxx. */
   std::max_align_t m_abStartTime[ABC_ALIGNED_SIZE(8)];
   //! Total measured time duration, in nanoseconds. Precision is not guaranteed on all platforms.
   duration_type m_iTotalDuration;
};

} //namespace perf
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend – specialization for abc::perf::stopwatch

namespace abc {

template <>
class to_str_backend<perf::stopwatch> : public to_str_backend<perf::stopwatch::duration_type> {
public:
   /*! Writes a stopwatch by its duration in ns, applying the formatting options.

   @param sw
      Stopwatch to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(perf::stopwatch const & sw, io::text::writer * ptwOut) {
      to_str_backend<perf::stopwatch::duration_type>::write(sw.duration(), ptwOut);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PERF_STOPWATCH_HXX

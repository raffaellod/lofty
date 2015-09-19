/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_PERF_STOPWATCH_HXX
#define _ABACLADE_PERF_STOPWATCH_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace perf {

/*! Measures processing time intervals for the current process at a high platform-dependent
precision. */
class ABACLADE_SYM stopwatch {
public:
   //! Integer type used to measure durations in nanoseconds.
   typedef std::uint64_t duration_type;

public:
   //! Default constructor.
   stopwatch();

   /*! Copy constructor.

   @param sw
      Source object.
   */
   stopwatch(stopwatch const & sw);

   //! Destructor.
   ~stopwatch();

   /*! Copy-assignment operator.

   @param sw
      Source object.
   @return
      *this.
   */
   stopwatch & operator=(stopwatch const & sw);

   /*! Returns the total tracked time.

   @return
      Cumulative time counted by start()/stop() call pairs, in nanoseconds.
   */
   duration_type duration() const {
      return m_iTotalDuration;
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
   _std::unique_ptr<void, memory::freeing_deleter> m_pStartTime;
   //! Total measured time duration, in nanoseconds. Precision is not guaranteed on all platforms.
   duration_type m_iTotalDuration;
};

}} //namespace abc::perf

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
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
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PERF_STOPWATCH_HXX

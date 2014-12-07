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

//! Base class for test cases.
class ABACLADE_TESTING_SYM stopwatch {
public:
   stopwatch();

   ~stopwatch();

   std::uint64_t duration() const {
      return m_iTotalDuration;
   }

   void start();

   std::uint64_t stop();

protected:
   /*! Start time of the current timed session. Large enough to accommodate the real type, defined
   in stopwatch.cxx. */
   std::max_align_t m_abStartTime[ABC_ALIGNED_SIZE(8)];
   //! Total measured time duration, in nanoseconds. Precision is not guaranteed on all platforms.
   std::uint64_t m_iTotalDuration;
};

} //namespace perf
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_PERF_STOPWATCH_HXX

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

#ifndef _ABC_TESTING_MOCK_IOSTREAM_HXX
#define _ABC_TESTING_MOCK_IOSTREAM_HXX

#include <abc/testing/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mock::istream


namespace abc {
namespace testing {
namespace mock {

#if 0
/** Implementation of a read-only stream based on a string.
*/
class ABCTESTINGAPI istream :
   public virtual ::abc::io::istream {
};
#endif

} //namespace mock
} //namespace testing
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mock::ostream


namespace abc {
namespace testing {
namespace mock {

/** Implementation of an write-only stream based on a string.
*/
class ABCTESTINGAPI ostream :
   public virtual ::abc::io::ostream {
public:

   /** Constructor.
   */
   ostream();


   /** Returns a string referencing the current contents of the stream.

   return
      Contents of the stream.
   */
   istr const contents() const {
      return istr(unsafe, m_achBuf, m_cchUsed);
   }


   /** Empties the contents of the stream.
   */
   void reset() {
      m_cchUsed = 0;
   }


   /** See io::ostream::write_raw().
   */
   virtual void write_raw(void const * p, size_t cb, text::encoding enc);


private:

   /** Target buffer. */
   char_t m_achBuf[4096];
   /** Current write offset into the string, in bytes. Seeks can only change this in increments of a
   character, but internal code doesn’t have to. */
   size_t m_cchUsed;
};

} //namespace mock
} //namespace testing
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_TESTING_MOCK_IOSTREAM_HXX


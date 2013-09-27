/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2011, 2012, 2013
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

#ifndef ABC_MOCK_IOSTREAM_HXX
#define ABC_MOCK_IOSTREAM_HXX

#include <abc/iostream.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mock::istream


namespace abc {

namespace mock {

#if 0
/** Implementation of an read-only stream based on a string.
*/
class istream :
	public virtual ::abc::istream {
};
#endif

} //namespace mock

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mock::ostream


namespace abc {

namespace mock {

/** Implementation of an write-only stream based on a string.
*/
class ostream :
	public virtual ::abc::ostream {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	ostream();


	/** Returns true if the current contents of the stream match the specified string.

	TODO: comment signature.
	*/
	bool contents_equal(istr const & sExpected);


	/** Empties the contents of the stream.

	TODO: comment signature.
	*/
	void reset() {
		m_cchUsed = 0;
	}


	/** See ostream::write().
	*/
	virtual void write(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


private:

	/** Target buffer. */
	char_t m_achBuf[4096];
	/** Current write offset into the string, in bytes. Seeks can only change this in increments of a
	character, but internal code doesn’t have to. */
	size_t m_cchUsed;
};

} //namespace mock

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_MOCK_IOSTREAM_HXX


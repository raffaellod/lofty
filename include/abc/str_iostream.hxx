/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
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

#ifndef ABC_STRING_IOSTREAM_HXX
#define ABC_STRING_IOSTREAM_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/iostream.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_istream


namespace abc {

/** Implementation of an read-only stream based on a string.
*/
class str_istream :
	public virtual istream {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	explicit str_istream(istr const & s);
	explicit str_istream(istr && s);
	explicit str_istream(mstr && s);
	explicit str_istream(dmstr && s);


	/** Destructor.
	*/
	virtual ~str_istream();


	/** See istream::read().
	*/
	virtual size_t read(void * p, size_t cbMax, text::encoding enc = text::encoding::identity);


	/** See istream::unread().
	*/
	virtual void unread(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


protected:

	/** See istream::_read_line().
	*/
	virtual void _read_line(
		_raw_str & rs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
	);


protected:

	/** Source string. */
	istr m_sBuf;
	/** Current read offset into the string, in bytes. Seeks can only change this in increments of a
	character, but internal code doesn’t have to. */
	size_t m_ibRead;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_ostream


namespace abc {

/** Implementation of an write-only stream based on a string.
*/
class str_ostream :
	public virtual ostream {

	typedef dmstr string_type;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	str_ostream();


	/** Destructor.
	*/
	virtual ~str_ostream();


	/** Returns and empties the contents of the stream.

	TODO: comment signature.
	*/
	string_type get_contents();


	/** See ostream::write_raw().
	*/
	virtual void write_raw(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


protected:

	/** Target string. */
	string_type m_sBuf;
	/** Current write offset into the string, in bytes. Seeks can only change this in increments of a
	character, but internal code doesn’t have to. */
	size_t m_ibWrite;
};


// Now these can be implemented.

template <typename C, class TTraits>
template <typename ... Ts>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(Ts const & ... ts) const {
	str_ostream os;
	os.print(*static_cast<istr_<C, TTraits> const *>(this), ts ...);
	return os.get_contents();
}


template <typename T>
inline dmstr to_str(T const & t, istr const & sFormat = istr()) {
	str_ostream os;
	to_str_backend<T> tsb(sFormat);
	tsb.write(t, &os);
	return os.get_contents();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_STRING_IOSTREAM_HXX


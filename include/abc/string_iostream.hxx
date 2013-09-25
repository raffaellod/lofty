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
// abc::string_istream


namespace abc {

/** Implementation of an read-only stream based on a string.
*/
class string_istream :
	public virtual istream {
public:

	/** Constructor.

	TODO: comment signature.
	*/
	explicit string_istream(cstring const & s);
	explicit string_istream(cstring && s);
	explicit string_istream(wstring && s);
	explicit string_istream(wdstring && s);


	/** Destructor.
	*/
	virtual ~string_istream();


	/** See istream::read().

	TODO: comment signature.
	*/
	virtual size_t read(void * p, size_t cbMax, text::encoding enc = text::encoding::identity);


	/** See istream::unread().

	TODO: comment signature.
	*/
	virtual void unread(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


protected:

	/** See istream::_read_line().

	TODO: comment signature.
	*/
	virtual void _read_line(
		_raw_string & rs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
	);


protected:

	/** Source string. */
	cstring m_sBuf;
	/** Current read offset into the string, in bytes. Seeks can only change this in increments of a
	character, but internal code doesn’t have to. */
	size_t m_ibRead;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::string_ostream


namespace abc {

/** Implementation of an write-only stream based on a string.
*/
class string_ostream :
	public virtual ostream {

	typedef wdstring string_type;

public:

	/** Constructor.

	TODO: comment signature.
	*/
	string_ostream();


	/** Destructor.
	*/
	virtual ~string_ostream();


	/** Returns and empties the contents of the stream.

	TODO: comment signature.
	*/
	string_type get_contents();


	/** See ostream::write().

	TODO: comment signature.
	*/
	virtual void write(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


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
inline wdstring_<C, TTraits> string_base_<C, TTraits>::format(Ts const & ... ts) const {
	string_ostream os;
	os.print(*static_cast<cstring_<C, TTraits> const *>(this), ts ...);
	return os.get_contents();
}


template <typename T>
inline wdstring to_string(T const & t, cstring const & sFormat = cstring()) {
	string_ostream os;
	to_string_backend<T> tsb(sFormat);
	tsb.write(t, &os);
	return os.get_contents();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_STRING_IOSTREAM_HXX


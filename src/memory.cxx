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

#include <abc/core.hxx>
#include <abc/memory.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals - standard new/delete operators


#ifdef _MSC_VER
	#pragma warning(push)
	// “'operator': exception specification does not match previous declaration”
	#pragma warning(disable: 4986)
#endif

void * ABC_STL_CALLCONV operator new(size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
void * ABC_STL_CALLCONV operator new[](size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
void * ABC_STL_CALLCONV operator new(size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
	return ::malloc(cb);
}
void * ABC_STL_CALLCONV operator new[](size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
	return ::malloc(cb);
}


void ABC_STL_CALLCONV operator delete(void * p) ABC_STL_NOEXCEPT_TRUE() {
	abc::memory::free(p);
}
void ABC_STL_CALLCONV operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE() {
	abc::memory::free(p);
}
void ABC_STL_CALLCONV operator delete(void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
	abc::memory::free(p);
}
void ABC_STL_CALLCONV operator delete[](void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
	abc::memory::free(p);
}


#ifdef _MSC_VER
	#pragma warning(pop)
#endif

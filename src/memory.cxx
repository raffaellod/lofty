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

#define _ABC_MEMORY_HXX_IMPL
#include <abc/core.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals - dynamic memory allocation


#if ABC_HOST_API_WIN32

extern "C" void * malloc(size_t cb) {
	return ::HeapAlloc(::GetProcessHeap(), 0, cb);
}


extern "C" void * realloc(void * p, size_t cb) {
	HANDLE hHeap(::GetProcessHeap());
	if (p) {
		return ::HeapReAlloc(hHeap, 0, p, cb);
	} else {
		return ::HeapAlloc(hHeap, 0, cb);
	}
}


extern "C" void free(void * p) {
	::HeapFree(::GetProcessHeap(), 0, p);
}

#endif //if ABC_HOST_API_WIN32


////////////////////////////////////////////////////////////////////////////////////////////////////


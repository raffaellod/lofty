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


// Forward declarations.
namespace abc {

namespace memory {

void * _raw_alloc(size_t cb);

template <typename T>
void free(T * pt);

} //namespace memory

} //namespace abc


#ifdef _MSC_VER
	#pragma warning(push)
	// “'operator': exception specification does not match previous declaration”
	#pragma warning(disable: 4986)
#endif

// In Win32, MSC expects ::new() and ::delete() to use the cdecl calling convention.
#if defined(_MSC_VER) && ABC_HOST_API_WIN32 && !ABC_HOST_API_WIN64
	#define operator __cdecl operator
#endif

void * operator new(size_t cb) decl_throw((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
void * operator new[](size_t cb) decl_throw((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
void * operator new(size_t cb, std::nothrow_t const &) decl_throw(()) {
	return ::malloc(cb);
}
void * operator new[](size_t cb, std::nothrow_t const &) decl_throw(()) {
	return ::malloc(cb);
}


void operator delete(void * p) decl_throw(()) {
	abc::memory::free(p);
}
void operator delete[](void * p) decl_throw(()) {
	abc::memory::free(p);
}
void operator delete(void * p, std::nothrow_t const &) decl_throw(()) {
	abc::memory::free(p);
}
void operator delete[](void * p, std::nothrow_t const &) decl_throw(()) {
	abc::memory::free(p);
}


#ifdef operator
	#undef operator
#endif

#ifdef _MSC_VER
	#pragma warning(pop)
#endif

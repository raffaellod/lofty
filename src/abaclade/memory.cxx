/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#include <abaclade.hxx>
#include <cstdlib> // std::free() std::malloc() std::realloc()


////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals – standard new/delete operators

#if ABC_HOST_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * ABC_STL_CALLCONV operator new(std::size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc)) {
   return abc::memory::_raw_alloc(cb);
}
void * ABC_STL_CALLCONV operator new[](std::size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc)) {
   return abc::memory::_raw_alloc(cb);
}
void * ABC_STL_CALLCONV operator new(
   std::size_t cb, std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   return std::malloc(cb);
}
void * ABC_STL_CALLCONV operator new[](
   std::size_t cb, std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   return std::malloc(cb);
}

void ABC_STL_CALLCONV operator delete(void * p) ABC_STL_NOEXCEPT_TRUE() {
   abc::memory::_raw_free(p);
}
void ABC_STL_CALLCONV operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE() {
   abc::memory::_raw_free(p);
}
void ABC_STL_CALLCONV operator delete(void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
   std::free(p);
}
void ABC_STL_CALLCONV operator delete[](void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE() {
   std::free(p);
}

#if ABC_HOST_MSC
   #pragma warning(pop)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory globals – management

namespace abc {
namespace memory {

void * _raw_alloc(std::size_t cb) {
   void * p = std::malloc(cb);
   if (!p) {
      ABC_THROW(memory_allocation_error, ());
   }
   return p;
}

void _raw_free(void const * p) {
   std::free(const_cast<void *>(p));
}

void * _raw_realloc(void * p, std::size_t cb) {
   p = std::realloc(p, cb);
   if (!p) {
      ABC_THROW(memory_allocation_error, ());
   }
   return p;
}

} //namespace memory
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

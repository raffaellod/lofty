/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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
#include <abaclade/bitmanip.hxx>

#include <cstdlib> // std::free() std::malloc() std::realloc()
#if ABC_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

bad_alloc::bad_alloc() {
   m_pszWhat = "abc::memory::bad_alloc";
}

bad_alloc::bad_alloc(bad_alloc const & x) :
   generic_error(x),
   m_cbFailed(x.m_cbFailed) {
}

/*virtual*/ bad_alloc::~bad_alloc() {
}

bad_alloc & bad_alloc::operator=(bad_alloc const & x) {
   generic_error::operator=(x);
   m_cbFailed = x.m_cbFailed;
   return *this;
}

void bad_alloc::init(std::size_t cbFailed, errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      ENOMEM
#elif ABC_HOST_API_WIN32
      ERROR_NOT_ENOUGH_MEMORY
#else
      0
#endif
   );
   m_cbFailed = cbFailed;
}

/*virtual*/ void bad_alloc::write_extended_info(
   io::text::writer * ptwOut
) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL(" requested allocation size={} B"), m_cbFailed);
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

char_t const bad_pointer::smc_szUnknownAddress[] = ABC_SL(" unknown memory address");

bad_pointer::bad_pointer() {
   m_pszWhat = "abc::memory::bad_pointer";
}

bad_pointer::bad_pointer(bad_pointer const & x) :
   generic_error(x),
   m_pInvalid(x.m_pInvalid) {
}

/*virtual*/ bad_pointer::~bad_pointer() {
}

bad_pointer & bad_pointer::operator=(bad_pointer const & x) {
   generic_error::operator=(x);
   m_pInvalid = x.m_pInvalid;
   return *this;
}

void bad_pointer::init(void const * pInvalid, errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EFAULT
#elif ABC_HOST_API_WIN32
      ERROR_INVALID_ADDRESS
#else
      0
#endif
   );
   m_pInvalid = pInvalid;
}

/*virtual*/ void bad_pointer::write_extended_info(io::text::writer * ptwOut) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   if (m_pInvalid != smc_szUnknownAddress) {
      ptwOut->print(ABC_SL(" invalid pointer={}"), m_pInvalid);
   } else {
      ptwOut->write(smc_szUnknownAddress);
   }
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

bad_pointer_alignment::bad_pointer_alignment() {
   m_pszWhat = "abc::memory::bad_pointer_alignment";
}

bad_pointer_alignment::bad_pointer_alignment(bad_pointer_alignment const & x) :
   generic_error(x),
   m_pInvalid(x.m_pInvalid) {
}

/*virtual*/ bad_pointer_alignment::~bad_pointer_alignment() {
}

bad_pointer_alignment & bad_pointer_alignment::operator=(bad_pointer_alignment const & x) {
   generic_error::operator=(x);
   m_pInvalid = x.m_pInvalid;
   return *this;
}

void bad_pointer_alignment::init(void const * pInvalid, errint_t err /*= 0*/) {
   generic_error::init(err ? err :
#if ABC_HOST_API_POSIX
      EFAULT
#elif ABC_HOST_API_WIN32
      ERROR_INVALID_ADDRESS
#else
      0
#endif
   );
   m_pInvalid = pInvalid;
}

/*virtual*/ void bad_pointer_alignment::write_extended_info(
   io::text::writer * ptwOut
) const /*override*/ {
   generic_error::write_extended_info(ptwOut);
   ptwOut->print(ABC_SL(" misaligned pointer={}"), m_pInvalid);
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_CXX_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * ABC_STL_CALLCONV operator new(std::size_t cb) {
   return abc::memory::alloc<void>(cb);
}
void * ABC_STL_CALLCONV operator new[](std::size_t cb) {
   return abc::memory::alloc<void>(cb);
}
void * ABC_STL_CALLCONV operator new(
   std::size_t cb, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   return std::malloc(cb);
}
void * ABC_STL_CALLCONV operator new[](
   std::size_t cb, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   return std::malloc(cb);
}

void ABC_STL_CALLCONV operator delete(void * p) ABC_STL_NOEXCEPT_TRUE() {
   abc::memory::free(p);
}
void ABC_STL_CALLCONV operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE() {
   abc::memory::free(p);
}
void ABC_STL_CALLCONV operator delete(
   void * p, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   std::free(p);
}
void ABC_STL_CALLCONV operator delete[](
   void * p, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE() {
   std::free(p);
}

#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

template <>
void * alloc<void>(std::size_t cb) {
   if (void * p = std::malloc(cb)) {
      return p;
   }
   ABC_THROW(memory::bad_alloc, (cb));
}

void free(void const * p) {
   std::free(const_cast<void *>(p));
}

template <>
void realloc<void>(void ** pp, std::size_t cb) {
   if (void * p = std::realloc(*pp, cb)) {
      *pp = p;
   } else {
      ABC_THROW(memory::bad_alloc, (cb));
   }
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

pages_ptr::pages_ptr() :
   m_p(nullptr),
   m_cb(0) {
}
pages_ptr::pages_ptr(std::size_t cb) :
   m_p(nullptr) {
   std::size_t cbPage = page_size();
   m_cb = bitmanip::ceiling_to_pow2_multiple(cb, cbPage);
#if ABC_HOST_API_POSIX
   if (int iErr = ::posix_memalign(&m_p, cbPage, m_cb)) {
      switch (iErr) {
         case ENOMEM:
            ABC_THROW(bad_alloc, (cb, iErr));
         default:
            exception::throw_os_error(iErr);
      }
   }
#elif ABC_HOST_API_WIN32
   m_p = ::VirtualAlloc(nullptr, m_cb, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   if (!m_p) {
      ::DWORD iErr = ::GetLastError();
      if (iErr == ERROR_NOT_ENOUGH_MEMORY) {
         ABC_THROW(bad_alloc, (m_cb, iErr));
      } else {
         exception::throw_os_error(iErr);
      }
   }
#else
   #error "TODO: HOST_API"
#endif
}
pages_ptr::pages_ptr(pages_ptr && pp) :
   m_p(pp.m_p),
   m_cb(pp.m_cb) {
   pp.m_p = nullptr;
   pp.m_cb = 0;
}

pages_ptr::~pages_ptr() {
   if (m_p) {
#if ABC_HOST_API_POSIX
      ::free(m_p);
#elif ABC_HOST_API_WIN32
      ::VirtualFree(m_p, 0, MEM_RELEASE);
#else
   #error "TODO: HOST_API"
#endif
   }
}

pages_ptr & pages_ptr::operator=(pages_ptr && pp) {
   pages_ptr ppOld(_std::move(*this));
   m_p = pp.m_p;
   pp.m_p = nullptr;
   m_cb = pp.m_cb;
   pp.m_cb = 0;
   return *this;
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

/*! Returns the size of a memory page.

@return
   Size of a memory page, in bytes.
*/
std::size_t page_size() {
   static std::size_t s_cb = 0;
   /* The race condition here is harmless, since the page size will be the same for all the threads
   that will concurrently execute the if block. */
   if (s_cb == 0) {
#if ABC_HOST_API_POSIX
      s_cb = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
#elif ABC_HOST_API_WIN32
      ::SYSTEM_INFO si;
      ::GetSystemInfo(&si);
      s_cb = si.dwPageSize;
#else
   #error "TODO: HOST_API"
#endif
   }
   return s_cb;
}

}} //namespace abc::memory

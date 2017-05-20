/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/bitmanip.hxx>

#include <cstdlib> // std::free() std::malloc() std::realloc()
#if LOFTY_HOST_API_POSIX
   #include <unistd.h> // _SC_* sysconf()
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*explicit*/ bad_alloc::bad_alloc(std::size_t allocation_size__, errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      ENOMEM
#elif LOFTY_HOST_API_WIN32
      ERROR_NOT_ENOUGH_MEMORY
#else
      0
#endif
   ),
   allocation_size_(allocation_size__) {
   what_ostream().print(LOFTY_SL("requested allocation size={} B"), allocation_size_);
}

bad_alloc::bad_alloc(bad_alloc const & src) :
   generic_error(src),
   allocation_size_(src.allocation_size_) {
}

/*virtual*/ bad_alloc::~bad_alloc() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_alloc & bad_alloc::operator=(bad_alloc const & src) {
   generic_error::operator=(src);
   allocation_size_ = src.allocation_size_;
   return *this;
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*explicit*/ bad_pointer::bad_pointer(errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EFAULT
#elif LOFTY_HOST_API_WIN32
      ERROR_INVALID_ADDRESS
#else
      0
#endif
   ),
   ptr(reinterpret_cast<void const *>(0xbadf00d)) {
}

/*explicit*/ bad_pointer::bad_pointer(void const * ptr_, errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EFAULT
#elif LOFTY_HOST_API_WIN32
      ERROR_INVALID_ADDRESS
#else
      0
#endif
   ),
   ptr(ptr_) {
   what_ostream().print(LOFTY_SL("invalid pointer={}"), ptr);
}

bad_pointer::bad_pointer(bad_pointer const & src) :
   generic_error(src),
   ptr(src.ptr) {
}

/*virtual*/ bad_pointer::~bad_pointer() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_pointer & bad_pointer::operator=(bad_pointer const & src) {
   generic_error::operator=(src);
   ptr = src.ptr;
   return *this;
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*explicit*/ bad_pointer_alignment::bad_pointer_alignment(void const * ptr_, errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      EFAULT
#elif LOFTY_HOST_API_WIN32
      ERROR_INVALID_ADDRESS
#else
      0
#endif
   ),
   ptr(ptr_) {
   what_ostream().print(LOFTY_SL("misaligned pointer={}"), ptr);
}

bad_pointer_alignment::bad_pointer_alignment(bad_pointer_alignment const & src) :
   generic_error(src),
   ptr(src.ptr) {
}

/*virtual*/ bad_pointer_alignment::~bad_pointer_alignment() LOFTY_STL_NOEXCEPT_TRUE() {
}

bad_pointer_alignment & bad_pointer_alignment::operator=(bad_pointer_alignment const & src) {
   generic_error::operator=(src);
   ptr = src.ptr;
   return *this;
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_CXX_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * LOFTY_STL_CALLCONV operator new(std::size_t byte_size) {
   return lofty::memory::alloc_bytes(byte_size);
}
void * LOFTY_STL_CALLCONV operator new[](std::size_t byte_size) {
   return lofty::memory::alloc_bytes(byte_size);
}
void * LOFTY_STL_CALLCONV operator new(
   std::size_t byte_size, lofty::_std::nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE() {
   return std::malloc(byte_size);
}
void * LOFTY_STL_CALLCONV operator new[](
   std::size_t byte_size, lofty::_std::nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE() {
   return std::malloc(byte_size);
}

void LOFTY_STL_CALLCONV operator delete(void * p) LOFTY_STL_NOEXCEPT_TRUE() {
   lofty::memory::free(p);
}
void LOFTY_STL_CALLCONV operator delete[](void * p) LOFTY_STL_NOEXCEPT_TRUE() {
   lofty::memory::free(p);
}
void LOFTY_STL_CALLCONV operator delete(void * p, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE() {
   std::free(p);
}
void LOFTY_STL_CALLCONV operator delete[](
   void * p, lofty::_std::nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE() {
   std::free(p);
}

#if LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

void * alloc_bytes(std::size_t byte_size) {
   if (void * p = std::malloc(byte_size)) {
      return p;
   }
   LOFTY_THROW(memory::bad_alloc, (byte_size));
}

void free(void const * p) {
   std::free(const_cast<void *>(p));
}

void realloc_bytes(void ** ptr_ptr, std::size_t byte_size) {
   if (void * p = std::realloc(*ptr_ptr, byte_size)) {
      *ptr_ptr = p;
   } else {
      LOFTY_THROW(memory::bad_alloc, (byte_size));
   }
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

pages_ptr::pages_ptr() :
   ptr(nullptr),
   byte_size(0) {
}
pages_ptr::pages_ptr(std::size_t byte_size_) :
   ptr(nullptr) {
   std::size_t page_byte_size = page_size();
   byte_size = bitmanip::ceiling_to_pow2_multiple(byte_size_, page_byte_size);
#if LOFTY_HOST_API_POSIX
   if (int err = ::posix_memalign(&ptr, page_byte_size, byte_size)) {
      switch (err) {
         case ENOMEM:
            LOFTY_THROW(bad_alloc, (byte_size, err));
         default:
            exception::throw_os_error(err);
      }
   }
#elif LOFTY_HOST_API_WIN32
   ptr = ::VirtualAlloc(nullptr, byte_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
   if (!ptr) {
      auto err = ::GetLastError();
      if (err == ERROR_NOT_ENOUGH_MEMORY) {
         LOFTY_THROW(bad_alloc, (byte_size, err));
      } else {
         exception::throw_os_error(err);
      }
   }
#else
   #error "TODO: HOST_API"
#endif
}
pages_ptr::pages_ptr(pages_ptr && src) :
   ptr(src.ptr),
   byte_size(src.byte_size) {
   src.ptr = nullptr;
   src.byte_size = 0;
}

pages_ptr::~pages_ptr() {
   if (ptr) {
#if LOFTY_HOST_API_POSIX
      ::free(ptr);
#elif LOFTY_HOST_API_WIN32
      ::VirtualFree(ptr, 0, MEM_RELEASE);
#else
   #error "TODO: HOST_API"
#endif
   }
}

pages_ptr & pages_ptr::operator=(pages_ptr && src) {
   pages_ptr old(_std::move(*this));
   ptr = src.ptr;
   src.ptr = nullptr;
   byte_size = src.byte_size;
   src.byte_size = 0;
   return *this;
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*! Returns the size of a memory page.

@return
   Size of a memory page, in bytes.
*/
std::size_t page_size() {
   static std::size_t byte_size = 0;
   /* The race condition here is harmless, since the page size will be the same for all the threads that will
   concurrently execute the if block. */
   if (byte_size == 0) {
#if LOFTY_HOST_API_POSIX
      byte_size = static_cast<std::size_t>(::sysconf(_SC_PAGESIZE));
#elif LOFTY_HOST_API_WIN32
      ::SYSTEM_INFO si;
      ::GetSystemInfo(&si);
      byte_size = si.dwPageSize;
#else
   #error "TODO: HOST_API"
#endif
   }
   return byte_size;
}

}} //namespace lofty::memory

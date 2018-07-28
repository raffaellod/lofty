/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_MEMORY_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_MEMORY_HXX
#endif

#ifndef _LOFTY_MEMORY_HXX_NOPUB
#define _LOFTY_MEMORY_HXX_NOPUB

#include <lofty/noncopyable.hxx>
#include <lofty/_std/exception.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/new.hxx>
#if LOFTY_HOST_API_POSIX
   #include <memory.h> // memcpy() memmove() memset()
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   // Clean up pollution caused by previous headers.
   extern "C" {

   #undef RtlZeroMemory
   WINBASEAPI void WINAPI RtlZeroMemory(void UNALIGNED * dst, ::SIZE_T byte_size);

   #undef RtlFillMemory
   WINBASEAPI void WINAPI RtlFillMemory(void UNALIGNED * dst, ::SIZE_T byte_size, ::UCHAR value);

   #undef RtlFillMemoryUlong
   WINBASEAPI void WINAPI RtlFillMemoryUlong(void * dst, ::SIZE_T byte_size, ::ULONG value);

   #undef RtlFillMemoryUlonglong
   WINBASEAPI void WINAPI RtlFillMemoryUlonglong(void * dst, ::SIZE_T byte_size, ::ULONGLONG value);

   #undef RtlCopyMemory
   WINBASEAPI void WINAPI RtlCopyMemory(void UNALIGNED * dst, void UNALIGNED const * src, ::SIZE_T byte_size);

   #undef RtlMoveMemory
   WINBASEAPI void WINAPI RtlMoveMemory(void UNALIGNED * dst, void UNALIGNED const * src, ::SIZE_T byte_size);

   } //extern "C"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_CXX_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * LOFTY_STL_CALLCONV operator new(std::size_t byte_size);
void * LOFTY_STL_CALLCONV operator new[](std::size_t byte_size);
void * LOFTY_STL_CALLCONV operator new(
   std::size_t byte_size, lofty::_std::_LOFTY_PUBNS nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();
void * LOFTY_STL_CALLCONV operator new[](
   std::size_t byte_size, lofty::_std::_LOFTY_PUBNS nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();

void LOFTY_STL_CALLCONV operator delete(void * p) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete[](void * p) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete(
   void * p, lofty::_std::_LOFTY_PUBNS nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete[](
   void * p, lofty::_std::_LOFTY_PUBNS nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();

#if LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Templated replacements to C’s mem* functions, integrated with STL smart pointers.
namespace memory {}

}

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

/*! Requests the dynamic allocation of a memory block of the specified size.

@param byte_size
   Count of bytes to allocate.
@return
   Pointer to the allocated memory block.
*/
LOFTY_SYM void * alloc_bytes(std::size_t byte_size);

/*! Requests the dynamic allocation of a memory block of the specified size.

@param byte_size
   Count of bytes to allocate.
@return
   Pointer to the allocated memory block.
*/
template <typename T>
T * alloc(std::size_t byte_size) {
   return static_cast<T *>(alloc_bytes(byte_size));
}

/*! Releases a block of dynamically allocated memory.

@param p
   Pointer to the memory block to be released.
*/
LOFTY_SYM void free(void const * p);

/*! Resizes a dynamically allocated memory block.

@param ptr_ptr
   Pointer to the caller’s pointer to the memory block to resize.
@param byte_size
   Count of bytes to resize **ptr_ptr to.
*/
LOFTY_SYM void realloc_bytes(void ** ptr_ptr, std::size_t byte_size);

/*! Resizes a dynamically allocated memory block.

@param ptr_ptr
   Pointer to the caller’s pointer to the memory block to resize.
@param byte_size
   Count of bytes to resize **ptr_ptr to.
*/
template <typename T>
void realloc(T ** ptr_ptr, std::size_t byte_size) {
   realloc_bytes(reinterpret_cast<void **>(ptr_ptr), byte_size);
}

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! Wrapper that invokes a deleter if and only if a set condition is true.
template <typename T, typename TDeleter = _std::_LOFTY_PUBNS default_delete<T>>
class conditional_deleter : public TDeleter {
public:
   /*! Constructor.

   @param enabled__
      If true, the deleter will delete objects when invoked; if false, it will do nothing.
   */
   explicit conditional_deleter(bool enabled__) :
      TDeleter(),
      enabled_(enabled__) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename U, typename UDeleter>
   conditional_deleter(conditional_deleter<U, UDeleter> const & src) :
      TDeleter(static_cast<UDeleter const &>(src)),
      enabled_(src.enabled()) {
   }

   /*! Deletes the specified object if the condition set in the constructor is true.

   @param pt
      Pointer to the object to delete.
   */
   void operator()(T * p) const {
      if (enabled_) {
         TDeleter::operator()(p);
      }
   }

   /*! Returns true if the deleter is enabled.

   @return
      true if the deleter is enabled, or false otherwise.
   */
   bool enabled() const {
      return enabled_;
   }

protected:
   //! true if the deleter is enabled, or false otherwise.
   bool enabled_;
};

//! @cond
// Partial specialization for arrays.
template <typename T, typename TDeleter>
class conditional_deleter<T[], TDeleter> : public conditional_deleter<T, TDeleter> {
public:
   //! See conditional_deleter<T>::conditional_deleter().
   explicit conditional_deleter(bool enabled__) :
      conditional_deleter<T, TDeleter>(enabled__) {
   }

   //! See conditional_deleter<T>::conditional_deleter().
   template <typename U, typename UDeleter>
   conditional_deleter(conditional_deleter<U, UDeleter> const & src) :
      conditional_deleter<T, TDeleter>(src) {
   }

   /*! Deletes the specified array if the condition set in the constructor was true. See also
   conditional_deleter<T, TDeleter>::operator()().

   @param pu
      Pointer to the array to delete.
   */
   template <typename U>
   void operator()(U * p) const {
      if (this->enabled_) {
         TDeleter::operator()(p);
      }
   }
};
//! @endcond

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! Deleter that deallocates memory using memory::free().
struct freeing_deleter {
   /*! Deallocates the specified memory block.

   @param p
      Pointer to the object to deallocate.
   */
   void operator()(void const * p) const {
      free(p);
   }
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

/*! Requests the dynamic allocation of a memory block of the specified size.

@param byte_size
   Amount of memory to allocate, in bytes (void case).
@return
   Pointer to the allocated memory block. The memory will be released with lofty::memory::free() when the
   pointer is destructed.
*/
inline _std::_LOFTY_PUBNS unique_ptr<void, freeing_deleter> alloc_bytes_unique(std::size_t byte_size) {
   return _std::_LOFTY_PUBNS unique_ptr<void, freeing_deleter>(alloc_bytes(byte_size));
}

/*! Requests the dynamic allocation of a memory block large enough to contain c objects of type T, plus
additional extra_byte_size bytes.

@param count
   Count of items to allocate memory for.
@param extra_byte_size
   Count of bytes of additional storage to allocate at the end of the requested items.
@return
   Pointer to the allocated memory block. The memory will be released with lofty::memory::free() when the
   pointer is destructed.
*/
template <typename T>
inline _std::_LOFTY_PUBNS unique_ptr<T, freeing_deleter> alloc_unique(
   std::size_t count = 1, std::size_t extra_byte_size = 0
) {
   typedef typename _std::_LOFTY_PUBNS unique_ptr<T, freeing_deleter>::element_type TElt;
   return _std::_LOFTY_PUBNS unique_ptr<T, freeing_deleter>(
      alloc<TElt>(sizeof(TElt) * count + extra_byte_size)
   );
}

/*! Changes the size of a block of dynamically allocated memory, updating the pointer referencing it in case a
new memory block is needed.

@param ptr_ptr
   Pointer to a smart pointer to the memory block to resize.
@param count
   Count of items to allocate memory for.
@param extra_byte_size
   Count of bytes of additional storage to allocate at the end of the requested items.
*/
template <typename T>
inline void realloc_unique(
   _std::_LOFTY_PUBNS unique_ptr<T, freeing_deleter> * ptr_ptr, std::size_t count,
   std::size_t extra_byte_size = 0
) {
   auto p = ptr_ptr->get();
   realloc(&p, sizeof(*p) * count + extra_byte_size);
   ptr_ptr->release();
   ptr_ptr->reset(p);
}

/*! Changes the size of a block of dynamically allocated memory, updating the pointer referencing it in case a
new memory block is needed.

@param ptr_ptr
   Pointer to a smart pointer to the memory block to resize.
@param byte_size
   Amount of memory to allocate, in bytes.
@param extra_byte_size
   Not used.
*/
template <>
inline void realloc_unique(
   _std::_LOFTY_PUBNS unique_ptr<void, freeing_deleter> * ptr_ptr, std::size_t byte_size,
   std::size_t extra_byte_size /*= 0*/
) {
   auto p = ptr_ptr->get();
   realloc_bytes(&p, byte_size + extra_byte_size);
   ptr_ptr->release();
   ptr_ptr->reset(p);
}

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

/*! Sets to the value 0 every item in the specified memory block.

@param dst
   Pointer to the target memory block.
@param count
   Count of items to clear.
*/
template <typename T>
inline void clear(T * dst, std::size_t count = 1) {
#if LOFTY_HOST_API_POSIX
   ::memset(dst, 0, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlZeroMemory(dst, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
}

/*! Copies memory from an array to another.

@param dst
   Pointer to the destination memory.
@param src
   Pointer to the source data.
@param count
   Count of items to copy.
*/
template <typename T>
inline void copy(T * dst, T const * src, std::size_t count = 1) {
#if LOFTY_HOST_API_POSIX
   ::memcpy(dst, src, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlMoveMemory(dst, src, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
}

/*! Copies memory from an array to another, where the two arrays may be overlapping.

@param dst
   Pointer to the destination memory.
@param src
   Pointer to the source data.
@param count
   Count of items to move.
*/
template <typename T>
inline void move(T * dst, T const * src, std::size_t count = 1) {
#if LOFTY_HOST_API_POSIX
   ::memmove(dst, src, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlMoveMemory(dst, src, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
}

/*! Copies a value over each item of an array.

@param dst
   Pointer to the destination memory.
@param value
   Source value to replicate over *dst.
@param count
   Count of copies of value to make.
*/
template <typename T>
inline void set(T * dst, T const & value, std::size_t count = 1) {
#if LOFTY_HOST_API_POSIX
   if (sizeof(T) == sizeof(std::int8_t)) {
      ::memset(dst, value, count);
      return;
   }
#elif LOFTY_HOST_API_WIN32
   if (sizeof(T) == sizeof(::UCHAR)) {
      ::RtlFillMemory(dst, count, value);
      return;
   }
#endif
   for (auto dst_end = dst + count; dst < dst_end; ++dst) {
      copy(dst, &value);
   }
}

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! Pointer to a chunk of memory allocated by the page.
class LOFTY_SYM pages_ptr : public lofty::_LOFTY_PUBNS noncopyable {
public:
   //! Default constructor.
   pages_ptr();

   /*! Move constructor.

   @param src
      Source object.
   */
   pages_ptr(pages_ptr && src);

   /*! Constructor.

   @param byte_size
      Amount of memory to allocate, in bytes.
   */
   pages_ptr(std::size_t byte_size);

   //! Destructor.
   ~pages_ptr();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   pages_ptr & operator=(pages_ptr && src);

   /*! Returns the raw pointer.

   @return
      Pointer to the start of the memory block.
   */
   void * get() const {
      return ptr;
   }

   /*! Returns the allocated memory size. The size may be greater than originally requested to the
   constructor.

   @return
      Size of the memory block, in bytes.
   */
   std::size_t size() const {
      return byte_size;
   }

private:
   //! Pointer to the memory block.
   void * ptr;
   //! Size of the memory block, in bytes.
   std::size_t byte_size;
};

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

/*! Returns the size of a memory page.

@return
   Size of a memory page, in bytes.
*/
LOFTY_SYM std::size_t page_size();

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <lofty/exception.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! A memory allocation request could not be satisfied.
class LOFTY_SYM bad_alloc : public lofty::_LOFTY_PUBNS generic_error {
public:
   /*! Constructor.

   @param allocation_size
      Amount of memory that could not be allocated.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_alloc(std::size_t allocation_size, lofty::_LOFTY_PUBNS errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_alloc(bad_alloc const & src);

   //! Destructor.
   virtual ~bad_alloc() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_alloc & operator=(bad_alloc const & src);

   /*! Returns the amount of memory that could not be allocated.

   @return
      Amount of requested memory, in bytes.
   */
   std::size_t allocation_size() const {
      return allocation_size_;
   }

private:
   //! Amount of memory that could not be allocated.
   std::size_t allocation_size_;
};

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! An attempt was made to access an invalid memory location.
class LOFTY_SYM bad_pointer : public lofty::_LOFTY_PUBNS generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(lofty::_LOFTY_PUBNS errint_t err = 0);

   /*! Constructor.

   @param ptr
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(void const * ptr, lofty::_LOFTY_PUBNS errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_pointer(bad_pointer const & src);

   //! Destructor.
   virtual ~bad_pointer() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_pointer & operator=(bad_pointer const & src);

   /*! Returns the faulty pointer. If the returned value is 0xbadf00d, the pointer might have not been
   provided in the constructor.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return ptr;
   }

private:
   //! Address that could not be dereferenced.
   void const * ptr;
};

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {
_LOFTY_PUBNS_BEGIN

//! An invalid memory access (e.g. misaligned pointer) was detected.
class LOFTY_SYM bad_pointer_alignment : public lofty::_LOFTY_PUBNS generic_error {
public:
   /*! Constructor.

   @param ptr
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer_alignment(void const * ptr, lofty::_LOFTY_PUBNS errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_pointer_alignment(bad_pointer_alignment const & src);

   //! Destructor.
   virtual ~bad_pointer_alignment() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_pointer_alignment & operator=(bad_pointer_alignment const & src);

   /*! Returns the faulty pointer.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return ptr;
   }

private:
   //! Address that could not be dereferenced.
   void const * ptr;
};

_LOFTY_PUBNS_END
}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_MEMORY_HXX_NOPUB

#ifdef _LOFTY_MEMORY_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace memory {
      using _pub::alloc;
      using _pub::alloc_bytes;
      using _pub::alloc_bytes_unique;
      using _pub::alloc_unique;
      using _pub::bad_alloc;
      using _pub::bad_pointer;
      using _pub::bad_pointer_alignment;
      using _pub::clear;
      using _pub::conditional_deleter;
      using _pub::copy;
      using _pub::free;
      using _pub::freeing_deleter;
      using _pub::move;
      using _pub::page_size;
      using _pub::pages_ptr;
      using _pub::realloc;
      using _pub::realloc_bytes;
      using _pub::realloc_unique;
      using _pub::set;
   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_MEMORY_HXX

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif

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
   std::size_t byte_size, lofty::_std::nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();
void * LOFTY_STL_CALLCONV operator new[](
   std::size_t byte_size, lofty::_std::nothrow_t const &
) LOFTY_STL_NOEXCEPT_TRUE();

void LOFTY_STL_CALLCONV operator delete(void * p) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete[](void * p) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete(void * p, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();
void LOFTY_STL_CALLCONV operator delete[](void * p, lofty::_std::nothrow_t const &) LOFTY_STL_NOEXCEPT_TRUE();

#if LOFTY_HOST_CXX_MSC
   #pragma warning(pop)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Templated replacements to C’s mem* functions, integrated with STL smart pointers.
namespace memory {}

} //namespace lofty

namespace lofty { namespace memory {

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

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

//! Wrapper that invokes a deleter if and only if a set condition is true.
template <typename T, typename TDeleter = _std::default_delete<T>>
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
// Specialization for arrays.
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

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

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

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*! Requests the dynamic allocation of a memory block of the specified size.

@param byte_size
   Amount of memory to allocate, in bytes (void case).
@return
   Pointer to the allocated memory block. The memory will be released with lofty::memory::free() when the
   pointer is destructed.
*/
inline _std::unique_ptr<void, freeing_deleter> alloc_bytes_unique(std::size_t byte_size) {
   return _std::unique_ptr<void, freeing_deleter>(alloc_bytes(byte_size));
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
inline _std::unique_ptr<T, freeing_deleter> alloc_unique(
   std::size_t count = 1, std::size_t extra_byte_size = 0
) {
   typedef typename _std::unique_ptr<T, freeing_deleter>::element_type TElt;
   return _std::unique_ptr<T, freeing_deleter>(alloc<TElt>(sizeof(TElt) * count + extra_byte_size));
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
   _std::unique_ptr<T, freeing_deleter> * ptr_ptr, std::size_t count, std::size_t extra_byte_size = 0
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
   _std::unique_ptr<void, freeing_deleter> * ptr_ptr, std::size_t byte_size,
   std::size_t extra_byte_size /*= 0*/
) {
   auto p = ptr_ptr->get();
   realloc_bytes(&p, byte_size + extra_byte_size);
   ptr_ptr->release();
   ptr_ptr->reset(p);
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*! Sets to the value 0 every item in the specified memory block.

@param dst
   Pointer to the target memory block.
@param count
   Count of items to clear.
@return
   Same as dst.
*/
template <typename T>
inline T * clear(T * dst, std::size_t count = 1) {
#if LOFTY_HOST_API_POSIX
   ::memset(dst, 0, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlZeroMemory(dst, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
   return dst;
}

/*! Copies memory from one pointer to another.

@param dst
   Pointer to the destination memory.
@param src
   Pointer to the source data.
@return
   Same as dst.
*/
template <typename T>
inline T * copy(T * dst, T const * src) {
   /* Optimization: if the copy can be made by mem-reg-mem transfers, avoid calling a function, so that the
   compiler can inline the copy. */
   switch (sizeof(T)) {
      case sizeof(std::int8_t):
         *reinterpret_cast<std::int8_t *>(dst) = *reinterpret_cast<std::int8_t const *>(src);
         break;
      case sizeof(std::int16_t):
         *reinterpret_cast<std::int16_t *>(dst) = *reinterpret_cast<std::int16_t const *>(src);
         break;
      case sizeof(std::int32_t):
         *reinterpret_cast<std::int32_t *>(dst) = *reinterpret_cast<std::int32_t const *>(src);
         break;
      case sizeof(std::int64_t):
         *reinterpret_cast<std::int64_t *>(dst) = *reinterpret_cast<std::int64_t const *>(src);
         break;
      default:
         copy<T>(dst, src, 1);
         break;
   }
   return dst;
}

/*! Copies memory from an array to another.

@param dst
   Pointer to the destination memory.
@param src
   Pointer to the source data.
@param count
   Count of items to copy.
@return
   Same as dst.
*/
template <typename T>
inline T * copy(T * dst, T const * src, std::size_t count) {
#if LOFTY_HOST_API_POSIX
   ::memcpy(dst, src, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlMoveMemory(dst, src, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
   return dst;
}

/*! Copies memory from an array to another, where the two arrays may be overlapping.

@param dst
   Pointer to the destination memory.
@param src
   Pointer to the source data.
@param count
   Count of items to move.
@return
   Same as dst.
*/
template <typename T>
inline T * move(T * dst, T const * src, std::size_t count) {
#if LOFTY_HOST_API_POSIX
   ::memmove(dst, src, sizeof(T) * count);
#elif LOFTY_HOST_API_WIN32
   ::RtlMoveMemory(dst, src, sizeof(T) * count);
#else
   #error "TODO: HOST_API"
#endif
   return dst;
}

/*! Copies a value over each item of an array.

@param dst
   Pointer to the destination memory.
@param value
   Source value to replicate over *dst.
@param count
   Count of copies of value to make.
@return
   Same as dst.
*/
template <typename T>
inline T * set(T * dst, T const & value, std::size_t count) {
   switch (sizeof(T)) {
#if LOFTY_HOST_API_POSIX
      case sizeof(std::int8_t):
         ::memset(dst, value, count);
         break;
#elif LOFTY_HOST_API_WIN32
      case sizeof(::UCHAR):
         ::RtlFillMemory(dst, count, value);
         break;
#endif
      default:
         for (auto dst_end = dst + count; dst < dst_end; ++dst) {
            copy(dst, &value);
         }
         break;
   }
   return dst;
}

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

//! Pointer to a chunk of memory allocated by the page.
class LOFTY_SYM pages_ptr : public noncopyable {
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

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

/*! Returns the size of a memory page.

@return
   Size of a memory page, in bytes.
*/
LOFTY_SYM std::size_t page_size();

}} //namespace lofty::memory

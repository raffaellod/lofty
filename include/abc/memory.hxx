/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef ABC_MEMORY_HXX
#define ABC_MEMORY_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/exception.hxx>


#if ABC_HOST_API_POSIX

   #include <stdlib.h> // free() malloc() realloc()
   #include <memory.h> // memcpy() memmove() memset()

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

   // Clean up pollution caused by previous headers.
   extern "C" {

   // Rtl*Memory*

   #undef RtlZeroMemory
   WINBASEAPI void WINAPI RtlZeroMemory(void UNALIGNED * pDst, size_t cb);

   #undef RtlFillMemory
   WINBASEAPI void WINAPI RtlFillMemory(void UNALIGNED * pDst, size_t cb, UCHAR iValue);

   #undef RtlFillMemoryUlong
   WINBASEAPI void WINAPI RtlFillMemoryUlong(void * pDst, size_t cb, ULONG iValue);

   #undef RtlFillMemoryUlonglong
   WINBASEAPI void WINAPI RtlFillMemoryUlonglong(void * pDst, size_t cb, ULONGLONG iValue);

   #undef RtlCopyMemory
   WINBASEAPI void WINAPI RtlCopyMemory(
      void UNALIGNED * pDst, void UNALIGNED const * pSrc, size_t cb
   );

   #undef RtlMoveMemory
   WINBASEAPI void WINAPI RtlMoveMemory(
      void UNALIGNED * pDst, void UNALIGNED const * pSrc, size_t cb
   );

   } //extern "C"

#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32

/** TODO: comment or remove.
*/
#if ABC_HOST_GCC
   #define _abc_alloca(cb) \
      __builtin_alloca((cb))
#elif ABC_HOST_MSC
   extern "C" void * ABC_STL_CALLCONV _alloca(size_t cb);
   #define _abc_alloca(cb) \
      _alloca(cb)
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals - standard new/delete operators


#if ABC_HOST_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * ABC_STL_CALLCONV operator new(size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc));
void * ABC_STL_CALLCONV operator new[](size_t cb) ABC_STL_NOEXCEPT_FALSE((std::bad_alloc));
void * ABC_STL_CALLCONV operator new(size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();
void * ABC_STL_CALLCONV operator new[](size_t cb, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();


void ABC_STL_CALLCONV operator delete(void * p) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete(void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete[](void * p, std::nothrow_t const &) ABC_STL_NOEXCEPT_TRUE();

#if ABC_HOST_MSC
   #pragma warning(pop)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::allocator


namespace abc {

namespace memory {

/** Allocator that uses a static memory block.

TODO: complete this! Some methods are missing.
*/
template <typename T>
class static_allocator {

#ifdef ABC_CXX_TEMPLATE_FRIENDS
   template <typename T2>
   friend class static_allocator;
#endif

public:

   typedef T const * const_pointer;
   typedef T * pointer;
   typedef typename std::allocator<T>::size_type size_type;

   template <typename T2>
   struct rebind {
      typedef static_allocator<T2> other;
   };


public:

   /** Constructor.

   TODO: comment signature.
   */
   static_allocator() :
      m_p(nullptr),
      m_cb(0) {
   }
   // This overload takes ownership of the provided static buffer.
   template <typename T2>
   static_allocator(T2 * t2) :
      m_p(t2),
      m_cb(sizeof(T2)) {
   }
   static_allocator(static_allocator const & sa) :
      m_p(sa.m_p),
      m_cb(sa.m_cb) {
   }
   template <class T2>
   static_allocator(static_allocator<T2> const & sat2) :
      m_p(sat2.m_p),
      m_cb(sat2.m_cb) {
   }


   /** Allocates enough storage for the specified number of T objects.

   TODO: comment signature.
   */
   pointer allocate(size_type c, void const * pHint = 0) {
      ABC_UNUSED_ARG(pHint);
      // c must fit in our static buffer, and we must still have a buffer.
      if (c > max_size() || !m_p) {
         ABC_THROW(memory_allocation_error, ());
      }

      pointer pt(static_cast<pointer>(m_p));
      m_p = nullptr;
      return pt;
   }


   /** Deallocates the storage associated to the specified T instance.

   TODO: comment signature.
   */
   void deallocate(pointer p, size_type c) {
      ABC_UNUSED_ARG(p);
      ABC_UNUSED_ARG(c);
   }


   /** Returns the maximum number of items that allocate() can create storage for.

   TODO: comment signature.
   */
   size_type max_size() const {
      return m_cb / sizeof(T);
   }


#ifdef ABC_CXX_TEMPLATE_FRIENDS
private:
#else
public:
#endif

   /** Pointer to the static storage. */
   void * m_p;
   /** Maximum size available in the static storage. */
   size_t m_cb;
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::deleter


namespace abc {

namespace memory {

// Forward declaration.
template <typename T>
void free(T * pt);


/** Deleter that deallocates memory using memory::free().
*/
template <typename T>
struct deleter {

   /** Deallocates the specified memory block.

   TODO: comment signature.
   */
   void operator()(T * pt) const {
      free(pt);
   }
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::noop_deleter


namespace abc {

namespace memory {

/** No-op deleter. It assumes the memory doesn’t need to be released.
*/
template <typename T>
struct noop_deleter {

   /** Deallocates the specified memory block.

   TODO: comment signature.
   */
   void operator()(T * pt) const {
      ABC_UNUSED_ARG(pt);
   }
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory globals - management


namespace abc {

namespace memory {

/** Allows to use the keyword auto to declare std::unique_ptr objects that use memory::deleter.

TODO: comment signature.
*/
template <typename T>
inline std::unique_ptr<T, deleter<T>> make_unique_ptr(T * pt = nullptr) {
   return std::unique_ptr<T, deleter<T>>(pt);
}


/** Requests the dynamic allocation of a memory block of the specified number of bytes.

TODO: comment signature.
*/
inline void * _raw_alloc(size_t cb) {
   void * p(::malloc(cb));
   if (!p) {
      ABC_THROW(memory_allocation_error, ());
   }
   return p;
}


/** Resizes a dynamically allocated memory block.

TODO: comment signature.
*/
inline void * _raw_realloc(void * p, size_t cb) {
   p = ::realloc(p, cb);
   if (!p) {
      ABC_THROW(memory_allocation_error, ());
   }
   return p;
}


/** Requests the dynamic allocation of a memory block large enough to contain c objects of type T,
plus an additional cbExtra bytes. With specialization that ignores types (void) and allocates the
specified number of bytes.

TODO: comment signature.
*/
template <typename T>
inline std::unique_ptr<T, deleter<T>> alloc(size_t c = 1, size_t cbExtra = 0) {
   return make_unique_ptr<T>(static_cast<T *>(_raw_alloc(sizeof(T) * c + cbExtra)));
}
template <>
inline std::unique_ptr<void, deleter<void>> alloc(size_t cb /*= 1*/, size_t cbExtra /*= 0*/) {
   return make_unique_ptr<void>(_raw_alloc(cb + cbExtra));
}


/** Releases a block of dynamically allocated memory.

TODO: comment signature.
*/
template <typename T>
inline void free(T * pt) {
   ::free(pt);
}


/** Changes the size of a block of dynamically allocated memory. Both overloads have a
specialization that ignores pointer types (void), and allocates the specified number of bytes.

TODO: comment signature.
*/
template <typename T>
inline T * realloc(T * pt, size_t c, size_t cbExtra = 0) {
   return static_cast<T *>(_raw_realloc(pt, sizeof(T) * c + cbExtra));
}
template <>
inline void * realloc(void * p, size_t cb, size_t cbExtra /*= 0*/) {
   return _raw_realloc(p, cb + cbExtra);
}
template <typename T>
inline void realloc(std::unique_ptr<T, deleter<T>> * ppt, size_t c, size_t cbExtra = 0) {
   T * pt(static_cast<T *>(_raw_realloc(ppt->get(), sizeof(T) * c + cbExtra)));
   ppt->release();
   ppt->reset(pt);
}
template <>
inline void realloc(std::unique_ptr<void, deleter<void>> * pp, size_t cb, size_t cbExtra /*= 0*/) {
   void * p(_raw_realloc(pp->get(), cb + cbExtra));
   pp->release();
   pp->reset(p);
}

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory globals - manipulation


namespace abc {

namespace memory {

/** Sets to the value 0 every item in an array.

TODO: comment signature.
*/
template <typename T>
inline T * clear(T * ptDst, size_t c = 1) {
   return static_cast<T *>(clear<void>(ptDst, sizeof(T) * c));
}
template <>
inline void * clear(void * pDst, size_t cb /*= 1*/) {
#if ABC_HOST_API_POSIX
   ::memset(pDst, 0, cb);
#elif ABC_HOST_API_WIN32
   ::RtlZeroMemory(pDst, cb);
#else
   #error TODO-PORT: HOST_API
#endif
   return pDst;
}


/** Copies memory, by number of items. With specialization that ignores pointer types (void), and
copies the specified number of bytes.

TODO: comment signature.
*/
template <typename T>
inline T * copy(T * ptDst, T const * ptSrc) {
   // Optimization: if the copy can be made by mem-reg-mem transfers, avoid calling a function, so
   // that the compiler can inline the copy.
   switch (sizeof(T)) {
      case sizeof(int8_t):
         *reinterpret_cast<int8_t *>(ptDst) = *reinterpret_cast<int8_t const *>(ptSrc);
         break;
      case sizeof(int16_t):
         *reinterpret_cast<int16_t *>(ptDst) = *reinterpret_cast<int16_t const *>(ptSrc);
         break;
      case sizeof(int32_t):
         *reinterpret_cast<int32_t *>(ptDst) = *reinterpret_cast<int32_t const *>(ptSrc);
         break;
      case sizeof(int64_t):
         *reinterpret_cast<int64_t *>(ptDst) = *reinterpret_cast<int64_t const *>(ptSrc);
         break;
      default:
         copy<void>(ptDst, ptSrc, sizeof(T));
         break;
   }
   return static_cast<T *>(ptDst);
}
// Nobody should want to use this, but it’s here for consistency.
template <>
inline void * copy(void * pDst, void const * pSrc) {
   *reinterpret_cast<int8_t *>(pDst) = *reinterpret_cast<int8_t const *>(pSrc);
   return pDst;
}
template <typename T>
inline T * copy(T * ptDst, T const * ptSrc, size_t c) {
   return static_cast<T *>(copy<void>(ptDst, ptSrc, sizeof(T) * c));
}
template <>
inline void * copy(void * pDst, void const * pSrc, size_t cb) {
#if ABC_HOST_API_POSIX
   ::memcpy(pDst, pSrc, cb);
#elif ABC_HOST_API_WIN32
   ::RtlMoveMemory(pDst, pSrc, cb);
#else
   #error TODO-PORT: HOST_API
#endif
   return pDst;
}


/** Copies possibly overlapping memory, by number of items. With specialization that ignores pointer
types (void), and copies the specified number of bytes.

TODO: comment signature.
*/
template <typename T>
inline T * move(T * ptDst, T const * ptSrc, size_t c) {
   return static_cast<T *>(move<void>(ptDst, ptSrc, sizeof(T) * c));
}
template <>
inline void * move(void * pDst, void const * pSrc, size_t cb) {
#if ABC_HOST_API_POSIX
   ::memmove(pDst, pSrc, cb);
#elif ABC_HOST_API_WIN32
   ::RtlMoveMemory(pDst, pSrc, cb);
#else
   #error TODO-PORT: HOST_API
#endif
   return pDst;
}


/** Copies a value over each item of a static array.

TODO: comment signature.
*/
template <typename T>
inline T * set(T * ptDst, T const & tValue, size_t c) {
   switch (sizeof(T)) {
#if ABC_HOST_API_POSIX
      case sizeof(int8_t):
         ::memset(ptDst, tValue, c);
         break;
#elif ABC_HOST_API_WIN32
      case sizeof(UCHAR):
         ::RtlFillMemory(ptDst, c, tValue);
         break;
#endif
      default:
         for (T const * ptDstMax(ptDst + c); ptDst < ptDstMax; ++ptDst) {
            copy(ptDst, &tValue);
         }
         break;
   }
   return ptDst;
}

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifdef ABC_MEMORY_HXX


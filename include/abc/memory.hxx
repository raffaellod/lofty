/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABC_MEMORY_HXX
#define _ABC_MEMORY_HXX

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
// abc::memory::freeing_deleter


namespace abc {

namespace memory {

// Forward declaration.
template <typename T>
void free(T const * pt);


/** Deleter that deallocates memory using memory::free().
*/
template <typename T>
struct freeing_deleter {

   /** Deallocates the specified memory block.

   pt
      Pointer to the object to delete.
   */
   void operator()(T * pt) const {
      free(pt);
   }
};

// Specialization for arrays.
template <typename T>
struct freeing_deleter<T[]> :
   public freeing_deleter<T> {

   /** Deallocates the specified array. See also freeing_deleter<T>::operator()().

   pt
      Pointer to the array to deallocate.
   */
   template <typename T2>
   void operator()(T2 * pt) const {
      freeing_deleter<T>::operator()(pt);
   }
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::conditional_deleter


namespace abc {

namespace memory {

/** Wrapper that invokes a deleter if and only if a set condition is true.
*/
template <typename T, typename TDeleter = std::default_delete<T>>
class conditional_deleter :
   public TDeleter {
public:

   /** Constructor.

   bEnabled
      If true, the deleter will delete objects when invoked; if false, it will do nothing.
   cd
      Source deleter.
   */
   conditional_deleter(bool bEnabled) :
      TDeleter(),
      m_bEnabled(bEnabled) {
   }
   template <typename T2, typename TDeleter2>
   conditional_deleter(conditional_deleter<T2, TDeleter2> const & cd) :
      TDeleter(static_cast<TDeleter2 const &>(cd)),
      m_bEnabled(cd.enabled()) {
   }


   /** Deletes the specified object if the condition set in the constructor was true.

   pt
      Pointer to the object to delete.
   */
   void operator()(T * pt) const {
      if (m_bEnabled) {
         TDeleter::operator()(pt);
      }
   }


   /** Returns true if the deleter is enabled.

   return
      true if the deleter is enable, or false otherwise.
   */
   bool enabled() const {
      return m_bEnabled;
   }


protected:

   bool m_bEnabled;
};

// Specialization for arrays.
template <typename T, typename TDeleter>
class conditional_deleter<T[], TDeleter> :
   public conditional_deleter<T, TDeleter> {
public:

   /** See conditional_deleter<T>::conditional_deleter().
   */
   conditional_deleter(bool bEnabled) :
      conditional_deleter<T, TDeleter>(bEnabled) {
   }
   template <typename T2, typename TDeleter2>
   conditional_deleter(conditional_deleter<T2, TDeleter2> const & cd) :
      conditional_deleter<T, TDeleter>(cd) {
   }


   /** Deletes the specified array if the condition set in the constructor was true. See also
   conditional_deleter<T, TDeleter>::operator()().

   pt
      Pointer to the array to delete.
   */
   template <typename T2>
   void operator()(T2 * pt) const {
      if (this->m_bEnabled) {
         TDeleter::operator()(pt);
      }
   }
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory globals - management


namespace abc {

namespace memory {

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
inline std::unique_ptr<T, freeing_deleter<T>> alloc(size_t c = 1, size_t cbExtra = 0) {
   return std::unique_ptr<T, freeing_deleter<T>>(
      static_cast<T *>(_raw_alloc(sizeof(T) * c + cbExtra))
   );
}
template <>
inline std::unique_ptr<void, freeing_deleter<void>> alloc(
   size_t cb /*= 1*/, size_t cbExtra /*= 0*/
) {
   return std::unique_ptr<void, freeing_deleter<void>>(_raw_alloc(cb + cbExtra));
}


/** Releases a block of dynamically allocated memory.

pt
   Pointer to the memory block to be released.
*/
template <typename T>
inline void free(T const * pt) {
   ::free(const_cast<T *>(pt));
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
inline void realloc(std::unique_ptr<T, freeing_deleter<T>> * ppt, size_t c, size_t cbExtra = 0) {
   T * pt(static_cast<T *>(_raw_realloc(ppt->get(), sizeof(T) * c + cbExtra)));
   ppt->release();
   ppt->reset(pt);
}
template <>
inline void realloc(
   std::unique_ptr<void, freeing_deleter<void>> * pp, size_t cb, size_t cbExtra /*= 0*/
) {
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


#endif //ifdef _ABC_MEMORY_HXX


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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif

#if ABC_HOST_API_POSIX
   #include <memory.h> // memcpy() memmove() memset()
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   // Clean up pollution caused by previous headers.
   extern "C" {

   #undef RtlZeroMemory
   WINBASEAPI void WINAPI RtlZeroMemory(void UNALIGNED * pDst, ::SIZE_T cb);

   #undef RtlFillMemory
   WINBASEAPI void WINAPI RtlFillMemory(void UNALIGNED * pDst, ::SIZE_T cb, ::UCHAR iValue);

   #undef RtlFillMemoryUlong
   WINBASEAPI void WINAPI RtlFillMemoryUlong(void * pDst, ::SIZE_T cb, ::ULONG iValue);

   #undef RtlFillMemoryUlonglong
   WINBASEAPI void WINAPI RtlFillMemoryUlonglong(void * pDst, ::SIZE_T cb, ::ULONGLONG iValue);

   #undef RtlCopyMemory
   WINBASEAPI void WINAPI RtlCopyMemory(
      void UNALIGNED * pDst, void UNALIGNED const * pSrc, ::SIZE_T cb
   );

   #undef RtlMoveMemory
   WINBASEAPI void WINAPI RtlMoveMemory(
      void UNALIGNED * pDst, void UNALIGNED const * pSrc, ::SIZE_T cb
   );

   } //extern "C"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32

//! TODO: comment or remove.
#if ABC_HOST_CXX_GCC
   #define _abc_alloca(cb) \
      __builtin_alloca((cb))
#elif ABC_HOST_CXX_MSC
   extern "C" void * ABC_STL_CALLCONV _alloca(std::size_t cb);
   #define _abc_alloca(cb) \
      _alloca(cb)
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

#if ABC_HOST_CXX_MSC
   #pragma warning(push)
   // “'operator': exception specification does not match previous declaration”
   #pragma warning(disable: 4986)
#endif

void * ABC_STL_CALLCONV operator new(
   std::size_t cb
) ABC_STL_NOEXCEPT_FALSE((abc::_std::bad_alloc));
void * ABC_STL_CALLCONV operator new[](
   std::size_t cb
) ABC_STL_NOEXCEPT_FALSE((abc::_std::bad_alloc));
void * ABC_STL_CALLCONV operator new(
   std::size_t cb, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE();
void * ABC_STL_CALLCONV operator new[](
   std::size_t cb, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE();

void ABC_STL_CALLCONV operator delete(void * p) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete[](void * p) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete(
   void * p, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE();
void ABC_STL_CALLCONV operator delete[](
   void * p, abc::_std::nothrow_t const &
) ABC_STL_NOEXCEPT_TRUE();

#if ABC_HOST_CXX_MSC
   #pragma warning(pop)
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

/*! Requests the dynamic allocation of a memory block of the specified number of bytes.

@param cb
   Count of bytes to allocate.
@return
   Pointer to the allocated memory block.
*/
ABACLADE_SYM void * _raw_alloc(std::size_t cb);

/*! Releases a block of dynamically allocated memory.

@param p
   Pointer to the memory block to be released.
*/
ABACLADE_SYM void _raw_free(void const * p);

/*! Resizes a dynamically allocated memory block.

@param p
   Pointer to the memory block to resize.
@param cb
   Count of bytes to resize *p to.
@return
   Pointer to the resized memory block. May or may not be the same as p.
*/
ABACLADE_SYM void * _raw_realloc(void * p, std::size_t cb);

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! Wrapper that invokes a deleter if and only if a set condition is true.
template <typename T, typename TDeleter = _std::default_delete<T>>
class conditional_deleter : public TDeleter {
public:
   /*! Constructor.

   @param bEnabled
      If true, the deleter will delete objects when invoked; if false, it will do nothing.
   @param cd
      Source deleter.
   */
   conditional_deleter(bool bEnabled) :
      TDeleter(),
      m_bEnabled(bEnabled) {
   }
   template <typename U, typename UDeleter>
   conditional_deleter(conditional_deleter<U, UDeleter> const & cd) :
      TDeleter(static_cast<UDeleter const &>(cd)),
      m_bEnabled(cd.enabled()) {
   }

   /*! Deletes the specified object if the condition set in the constructor is true.

   @param pt
      Pointer to the object to delete.
   */
   void operator()(T * pt) const {
      if (m_bEnabled) {
         TDeleter::operator()(pt);
      }
   }

   /*! Returns true if the deleter is enabled.

   @return
      true if the deleter is enabled, or false otherwise.
   */
   bool enabled() const {
      return m_bEnabled;
   }

protected:
   //! true if the deleter is enabled, or false otherwise.
   bool m_bEnabled;
};

// Specialization for arrays.
template <typename T, typename TDeleter>
class conditional_deleter<T[], TDeleter> : public conditional_deleter<T, TDeleter> {
public:
   //! See conditional_deleter<T>::conditional_deleter().
   conditional_deleter(bool bEnabled) :
      conditional_deleter<T, TDeleter>(bEnabled) {
   }

   //! See conditional_deleter<T>::conditional_deleter().
   template <typename U, typename UDeleter>
   conditional_deleter(conditional_deleter<U, UDeleter> const & cd) :
      conditional_deleter<T, TDeleter>(cd) {
   }

   /*! Deletes the specified array if the condition set in the constructor was true. See also
   conditional_deleter<T, TDeleter>::operator()().

   @param pu
      Pointer to the array to delete.
   */
   template <typename U>
   void operator()(U * pu) const {
      if (this->m_bEnabled) {
         TDeleter::operator()(pu);
      }
   }
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! Deleter that deallocates memory using memory::free().
struct freeing_deleter {
   /*! Deallocates the specified memory block.

   @param p
      Pointer to the object to deallocate.
   */
   void operator()(void const * p) const {
      _raw_free(p);
   }
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

/*! Requests the dynamic allocation of a memory block large enough to contain c objects of type T,
plus additional cbExtra bytes.

@param c
   Count of items to allocate memory for (non-void case).
@param cb
   Amount of memory to allocate, in bytes (void case).
@param cbExtra
   Count of bytes of additional storage to allocate at the end of the requested items.
@return
   Pointer to the allocated memory block. The memory will be released with abc::memory::free() when
   the pointer is destructed.
*/
template <typename T>
inline _std::unique_ptr<T, freeing_deleter> alloc(std::size_t c = 1, std::size_t cbExtra = 0) {
   typedef typename _std::unique_ptr<T, freeing_deleter>::element_type TElt;
   return _std::unique_ptr<T, freeing_deleter>(
      static_cast<TElt *>(_raw_alloc(sizeof(TElt) * c + cbExtra))
   );
}
template <>
inline _std::unique_ptr<void, freeing_deleter> alloc<void>(
   std::size_t cb, std::size_t cbExtra /*= 0*/
) {
   return _std::unique_ptr<void, freeing_deleter>(_raw_alloc(cb + cbExtra));
}

/*! Changes the size of a block of dynamically allocated memory, updating the pointer referencing
it in case a new memory block is needed.

@param ppt
   Pointer to a smart pointer to the memory block to resize.
@param c
   Count of items to allocate memory for (non-void case).
@param cb
   Amount of memory to allocate, in bytes (void case).
@param cbExtra
   Count of bytes of additional storage to allocate at the end of the requested items.
*/
template <typename T>
inline void realloc(
   _std::unique_ptr<T, freeing_deleter> * ppt, std::size_t c, std::size_t cbExtra = 0
) {
   typedef typename _std::unique_ptr<T, freeing_deleter>::element_type TElt;
   TElt * pt = static_cast<TElt *>(_raw_realloc(ppt->get(), sizeof(TElt) * c + cbExtra));
   ppt->release();
   ppt->reset(pt);
}
template <>
inline void realloc(
   _std::unique_ptr<void, freeing_deleter> * ppt, std::size_t cb, std::size_t cbExtra /*= 0*/
) {
   void * pt = _raw_realloc(ppt->get(), cb + cbExtra);
   ppt->release();
   ppt->reset(pt);
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

/*! Sets to the value 0 every item in the specified memory block.

@param ptDst
   Pointer to the target memory block.
@param c
   Count of items to clear.
@return
   Same as ptDst.
*/
template <typename T>
inline T * clear(T * ptDst, std::size_t c = 1) {
#if ABC_HOST_API_POSIX
   ::memset(ptDst, 0, sizeof(T) * c);
#elif ABC_HOST_API_WIN32
   ::RtlZeroMemory(ptDst, sizeof(T) * c);
#else
   #error "TODO: HOST_API"
#endif
   return ptDst;
}

/*! Copies memory from one pointer to another.

@param ptDst
   Pointer to the destination memory.
@param ptSrc
   Pointer to the source data.
@return
   Same as ptDst.
*/
template <typename T>
inline T * copy(T * ptDst, T const * ptSrc) {
   /* Optimization: if the copy can be made by mem-reg-mem transfers, avoid calling a function, so
   that the compiler can inline the copy. */
   switch (sizeof(T)) {
      case sizeof(std::int8_t):
         *reinterpret_cast<std::int8_t *>(ptDst) = *reinterpret_cast<std::int8_t const *>(ptSrc);
         break;
      case sizeof(std::int16_t):
         *reinterpret_cast<std::int16_t *>(ptDst) = *reinterpret_cast<std::int16_t const *>(ptSrc);
         break;
      case sizeof(std::int32_t):
         *reinterpret_cast<std::int32_t *>(ptDst) = *reinterpret_cast<std::int32_t const *>(ptSrc);
         break;
      case sizeof(std::int64_t):
         *reinterpret_cast<std::int64_t *>(ptDst) = *reinterpret_cast<std::int64_t const *>(ptSrc);
         break;
      default:
         copy<T>(ptDst, ptSrc, 1);
         break;
   }
   return ptDst;
}

/*! Copies memory from an array to another.

@param ptDst
   Pointer to the destination memory.
@param ptSrc
   Pointer to the source data.
@param c
   Count of items to copy.
@return
   Same as ptDst.
*/
template <typename T>
inline T * copy(T * ptDst, T const * ptSrc, std::size_t c) {
#if ABC_HOST_API_POSIX
   ::memcpy(ptDst, ptSrc, sizeof(T) * c);
#elif ABC_HOST_API_WIN32
   ::RtlMoveMemory(ptDst, ptSrc, sizeof(T) * c);
#else
   #error "TODO: HOST_API"
#endif
   return ptDst;
}

/*! Copies memory from an array to another, where the two arrays may be overlapping.

@param ptDst
   Pointer to the destination memory.
@param ptSrc
   Pointer to the source data.
@param c
   Count of items to move.
@return
   Same as ptDst.
*/
template <typename T>
inline T * move(T * ptDst, T const * ptSrc, std::size_t c) {
#if ABC_HOST_API_POSIX
   ::memmove(ptDst, ptSrc, sizeof(T) * c);
#elif ABC_HOST_API_WIN32
   ::RtlMoveMemory(ptDst, ptSrc, sizeof(T) * c);
#else
   #error "TODO: HOST_API"
#endif
   return ptDst;
}

/*! Copies a value over each item of an array.

@param ptDst
   Pointer to the destination memory.
@param tValue
   Source value to replicate over *ptDst.
@param c
   Count of copies of tValue to make.
@return
   Same as ptDst.
*/
template <typename T>
inline T * set(T * ptDst, T const & tValue, std::size_t c) {
   switch (sizeof(T)) {
#if ABC_HOST_API_POSIX
      case sizeof(std::int8_t):
         ::memset(ptDst, tValue, c);
         break;
#elif ABC_HOST_API_WIN32
      case sizeof(::UCHAR):
         ::RtlFillMemory(ptDst, c, tValue);
         break;
#endif
      default:
         for (T const * ptDstMax = ptDst + c; ptDst < ptDstMax; ++ptDst) {
            copy(ptDst, &tValue);
         }
         break;
   }
   return ptDst;
}

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! Pointer to a chunk of memory allocated by the page.
class ABACLADE_SYM pages_ptr : public noncopyable {
public:
   //! Default constructor.
   pages_ptr();

   /*! Move constructor.

   @param pp
      Source object.
   */
   pages_ptr(pages_ptr && pp);

   /*! Constructor.

   @param cb
      Amount of memory to allocate, in bytes.
   */
   pages_ptr(std::size_t cb);

   //! Destructor.
   ~pages_ptr();

   /*! Move-assignment operator.

   @param pp
      Source object.
   @return
      *this.
   */
   pages_ptr & operator=(pages_ptr && pp);

   /*! Returns the raw pointer.

   @return
      Pointer to the start of the memory block.
   */
   void * get() const {
      return m_p;
   }

   /*! Returns the allocated memory size. The size may be greater than originally requested to the
   constructor.

   @return
      Size of the memory block, in bytes.
   */
   std::size_t size() const {
      return m_cb;
   }

private:
   //! Pointer to the memory block.
   void * m_p;
   //! Size of the memory block, in bytes.
   std::size_t m_cb;
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

/*! Returns the size of a memory page.

@return
   Size of a memory page, in bytes.
*/
ABACLADE_SYM std::size_t page_size();

}} //namespace abc::memory

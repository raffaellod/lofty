// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#ifndef ABC_MEMORY_HXX
#define ABC_MEMORY_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/exception.hxx>
#include <memory>

#if ABC_HOST_API_POSIX

	#include <stdlib.h> // free() malloc() realloc()
	#include <memory.h> // memcpy() memmove() memset()

#elif ABC_HOST_API_WIN32

	// Clean up pollution caused by previous headers.
	extern "C" {

	// malloc(), realloc(), free()

	#undef malloc
	void * malloc(size_t cb);

	#undef realloc
	void * realloc(void * p, size_t cb);

	#undef free
	void free(void * p);


	// memset(), memcpy(), memmove()

	#undef memset
	void * memset(void * pDst, int iValue, size_t cb);

	#undef memcpy
	void * memcpy(void * pDst, void const * pSrc, size_t cb);

	#undef memmove
	void * memmove(void * pDst, void const * pSrc, size_t cb);


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

#endif

// _abc_alloca()
#if defined(_GCC_VER)
	#define _abc_alloca(cb) \
		__builtin_alloca((cb))
#elif defined(_MSC_VER)
	extern "C" void * __cdecl _alloca(size_t cb);
	#define _abc_alloca(cb) \
		_alloca(cb)
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

namespace memory {

/// Allocator that uses a static memory block.
template <typename T>
class static_allocator;

/// Deleter that deallocates memory using memory::free().
template <typename T>
struct deleter;

/// No-op deleter. It assumes the memory doesn’t need to be released.
template <typename T>
struct noop_deleter;

/// Allows to use the keyword auto to declare std::unique_ptr objects that use memory::deleter.
template <typename T>
std::unique_ptr<T, deleter<T>> make_unique_ptr(T * pt = NULL);

/// Requests the dynamic allocation of a memory block of the specified number of bytes.
void * _raw_alloc(size_t cb);

/// Resizes a dynamically allocated memory block.
void * _raw_realloc(void * p, size_t cb);


/// Requests the dynamic allocation of a memory block large enough to contain c objects of type T,
// plus an additional cbExtra bytes. With specialization that ignores types (void) and allocates the
// specified number of bytes.
template <typename T = void>
std::unique_ptr<T, deleter<T>> alloc(size_t c = 1, size_t cbExtra = 0);

/// Releases a block of dynamically allocated memory.
template <typename T = void>
void free(T * p);

/// Changes the size of a block of dynamically allocated memory. Both overloads have a
// specialization that ignores pointer types (void), and allocates the specified number of bytes.
template <typename T = void>
T * realloc(T * pt, size_t c, size_t cbExtra = 0);
template <typename T = void>
void realloc(std::unique_ptr<T, deleter<T>> * ppt, size_t c, size_t cbExtra = 0);


/// Sets to the value 0 every item in an array.
template <typename T = void>
T * clear(T * ptDst, size_t c = 1);

/// Copies memory, by number of items. With specialization that ignores pointer types (void), and
// copies the specified number of bytes.
template <typename T = void>
T * copy(T * ptDst, T const * ptSrc);
template <typename T = void>
T * copy(T * ptDst, T const * ptSrc, size_t c);

/// Copies possibly overlapping memory, by number of items. With specialization that ignores pointer
// types (void), and copies the specified number of bytes.
template <typename T = void>
T * move(T * ptDst, T const * ptSrc, size_t c);

/// Copies a value over each item of a static array.
template <typename T>
T * set(T * ptDst, T const & tValue, size_t c);

} //namespace memory

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// :: globals - standard new/delete operators


// This will cause code for the following functions to be generated for the compiled unit. Other
// units will just inline these (very) thin wrappers.
#ifdef _ABC_MEMORY_HXX_IMPL
	#define inline
#endif

inline void * operator new(size_t cb) decl_throw((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
inline void * operator new[](size_t cb) decl_throw((std::bad_alloc)) {
	return abc::memory::_raw_alloc(cb);
}
inline void * operator new(size_t cb, std::nothrow_t const &) decl_throw(()) {
	return ::malloc(cb);
}
inline void * operator new[](size_t cb, std::nothrow_t const &) decl_throw(()) {
	return ::malloc(cb);
}


inline void operator delete(void * p) decl_throw(()) {
	abc::memory::free(p);
}
inline void operator delete[](void * p) decl_throw(()) {
	abc::memory::free(p);
}
inline void operator delete(void * p, std::nothrow_t const &) decl_throw(()) {
	abc::memory::free(p);
}
inline void operator delete[](void * p, std::nothrow_t const &) decl_throw(()) {
	abc::memory::free(p);
}


#ifdef _ABC_MEMORY_HXX_IMPL
	#undef inline
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::allocator


namespace abc {

namespace memory {

// TODO: complete this! Some methods are missing.
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

	/// Constructor.
	//
	static_allocator() :
		m_p(NULL),
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


	/// Allocates enough storage for the specified number of T objects.
	//
	pointer allocate(size_type c, void const * pHint = 0) {
		UNUSED_ARG(pHint);
		// c must fit in our static buffer, and we must still have a buffer.
		if (c > max_size() || !m_p)
			abc_throw(memory_allocation_error());

		pointer pt(static_cast<pointer>(m_p));
		m_p = NULL;
		return pt;
	}


	/// Deallocates the storage associated to the specified T instance.
	//
	void deallocate(pointer p, size_type c) {
		UNUSED_ARG(p);
		UNUSED_ARG(c);
	}


	/// Returns the maximum number of items that allocate() can create storage for.
	//
	size_type max_size() const {
		return m_cb / sizeof(T);
	}


#ifdef ABC_CXX_TEMPLATE_FRIENDS
private:
#else
public:
#endif

	/// Pointer to the static storage.
	void * m_p;
	/// Maximum size available in the static storage.
	size_t m_cb;
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory::deleter
// abc::memory::noop_deleter


namespace abc {

namespace memory {

template <typename T>
struct deleter {

	/// Deallocates the specified memory block.
	//
	void operator()(T * pt) const {
		free(pt);
	}
};


template <typename T>
struct noop_deleter {

	/// Deallocates the specified memory block.
	//
	void operator()(T * pt) const {
		UNUSED_ARG(pt);
	}
};

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory globals - management


namespace abc {

namespace memory {

template <typename T>
inline std::unique_ptr<T, deleter<T>> make_unique_ptr(T * pt /*= NULL*/) {
	return std::unique_ptr<T, deleter<T>>(pt);
}


inline void * _raw_alloc(size_t cb) {
	void * p(::malloc(cb));
	if (!p)
		abc_throw(memory_allocation_error());
	return p;
}


inline void * _raw_realloc(void * p, size_t cb) {
	p = ::realloc(p, cb);
	if (!p)
		abc_throw(memory_allocation_error());
	return p;
}


template <typename T /*= void*/>
inline std::unique_ptr<T, deleter<T>> alloc(size_t c /*= 1*/, size_t cbExtra /*= 0*/) {
	return make_unique_ptr<T>(static_cast<T *>(_raw_alloc(sizeof(T) * c + cbExtra)));
}
template <>
inline std::unique_ptr<void, deleter<void>> alloc(size_t cb /*= 1*/, size_t cbExtra /*= 0*/) {
	return make_unique_ptr<void>(_raw_alloc(cb + cbExtra));
}


template <typename T /*= void*/>
inline void free(T * pt) {
	::free(pt);
}


template <typename T /*= void*/>
inline T * realloc(T * pt, size_t c, size_t cbExtra /*= 0*/) {
	return static_cast<T *>(_raw_realloc(pt, sizeof(T) * c + cbExtra));
}
template <>
inline void * realloc(void * p, size_t cb, size_t cbExtra /*= 0*/) {
	return _raw_realloc(p, cb + cbExtra);
}


template <typename T /*= void*/>
inline void realloc(std::unique_ptr<T, deleter<T>> * ppt, size_t c, size_t cbExtra /*= 0*/) {
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

template <typename T /*= void*/>
inline T * clear(T * ptDst, size_t c /*= 1*/) {
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


template <typename T /*= void*/>
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
template <typename T /*= void*/>
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


template <typename T /*= void*/>
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
			for (T const * ptDstMax(ptDst + c); ptDst < ptDstMax; ++ptDst)
				copy(ptDst, &tValue);
			break;
	}
	return ptDst;
}

} //namespace memory

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifdef ABC_MEMORY_HXX


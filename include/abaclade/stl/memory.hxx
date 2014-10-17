/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_STL_MEMORY_HXX
#define _ABACLADE_STL_MEMORY_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/stl/new.hxx>
#include <abaclade/stl/type_traits.hxx>
#include <abaclade/stl/tuple.hxx>
#include <abaclade/stl/exception.hxx>
#include <abaclade/stl/typeinfo.hxx>
#include <abaclade/atomic.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::default_delete

namespace std {

//! Deallocator functor that invokes delete on its argument (C++11 § 20.7.1.1 “Default deleters”).
template <typename T>
class default_delete {
public:
   /*! Constructor.

   TODO: comment signature.
   */
   /*constexpr*/ default_delete() {
   }
   template <class T2>
   default_delete(default_delete<T2> const & dd2) {
      ABC_UNUSED_ARG(dd2);
   }

   /*! Function call operator.

   TODO: comment signature.
   */
   void operator()(T * pt) const {
      delete pt;
   }
};

// Specialization to use delete[] for array types (C++11 § 20.7.1.1.3 “default_delete<T[]>”).
template <typename T>
class default_delete<T[]> {
public:
   //! Constructor.
   /*constexpr*/ default_delete() {
   }

   /*! Function call operator.

   TODO: comment signature.
   */
   void operator()(T * pt) const {
      delete[] pt;
   }
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::allocator

namespace std {

//! Default allocator (C++11 § 20.6.9 “The default allocator”).
template <typename T>
class allocator;

// Specialization for void. Needs to be declared before the generic version.
template <>
class allocator<void> {
public:
   typedef void * pointer;
   typedef void const * const_pointer;
   typedef void value_type;

   template <typename T2>
   struct rebind {
      typedef allocator<T2> other;
   };
};

template <typename T>
class allocator {
public:
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef T value_type;

   template <typename T2>
   struct rebind {
      typedef allocator<T2> other;
   };

public:
   /*! Constructor.

   a
      Source allocator.
   a2
      Source allocator.
   */
   allocator() {
   }
   allocator(allocator const & a) {
      ABC_UNUSED_ARG(a);
   }
   template <typename T2>
   allocator(allocator<T2> const & a2) {
      ABC_UNUSED_ARG(a2);
   }

   //! Destructor.
   ~allocator() {
   }

   //! TODO: comment.
   pointer address(reference t) const {
      return &reinterpret_cast<int8_t &>(t);
   }
   const_pointer address(const_reference t) const {
      return &reinterpret_cast<int8_t const &>(t);
   }

   /*! Allocates memory for the requested number of T objects.

   TODO: comment signature.
   */
   pointer allocate(size_type c, allocator<void>::const_pointer pHint = 0) {
      return reinterpret_cast<T *>(::new max_align_t[ABC_ALIGNED_SIZE(c * sizeof(T))]);
   }

   /*! Releases memory obtained through allocate().

   TODO: comment signature.
   */
   void deallocate(pointer p, size_type c) {
      ::delete static_cast<void *>(p);
   }

   /*! Returns the meximum number of T objects that allocate() can succeed for.

   TODO: comment signature.
   */
   size_type max_size() const {
      return ~size_t(0) / sizeof(T);
   }

   /*! Construct an object at the specified address, with the provided constructor arguments.

   TODO: comment signature.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename T2, typename... TArgs>
   void construct(T2 * pt2, TArgs &&... targs) {
      ::new(pt2) T2(forward(targs) ...);
   }
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
   // Overload for 0-argument T2::T2().
   template <typename T2>
   void construct(T2 * pt2) {
      ::new(pt2) T2();
   }
   // Overload for 1-argument T2::T2().
   template <typename T2, typename TArg0>
   void construct(T2 * pt2, TArg0 && targ0) {
      ::new(pt2) T2(forward(targ0));
   }
   // Overload for 2-argument T2::T2().
   template <typename T2, typename TArg0, typename TArg1>
   void construct(T2 * pt2, TArg0 && targ0, TArg1 && targ1) {
      ::new(pt2) T2(forward(targ0), forward(targ1));
   }
   // Overload for 3-argument T2::T2().
   template <typename T2, typename TArg0, typename TArg1, typename TArg2>
   void construct(T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2) {
      ::new(pt2) T2(forward(targ0), forward(targ1), forward(targ2));
   }
   // Overload for 4-argument T2::T2().
   template <typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
   void construct(T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3) {
      ::new(pt2) T2(forward(targ0), forward(targ1), forward(targ2), forward(targ3));
   }
   // Overload for 5-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4
   ) {
      ::new(pt2) T2(forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4));
   }
   // Overload for 6-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4,
      TArg5 && targ5
   ) {
      ::new(pt2) T2(
         forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4),
         forward(targ5)
      );
   }
   // Overload for 7-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4,
      TArg5 && targ5, TArg6 && targ6
   ) {
      ::new(pt2) T2(
         forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4),
         forward(targ5), forward(targ6)
      );
   }
   // Overload for 8-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4,
      TArg5 && targ5, TArg6 && targ6, TArg7 && targ7
   ) {
      ::new(pt2) T2(
         forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4),
         forward(targ5), forward(targ6), forward(targ7)
      );
   }
   // Overload for 9-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4,
      TArg5 && targ5, TArg6 && targ6, TArg7 && targ7, TArg8 && targ8
   ) {
      ::new(pt2) T2(
         forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4),
         forward(targ5), forward(targ6), forward(targ7), forward(targ8)
      );
   }
   // Overload for 10-argument T2::T2().
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8, typename TArg9
   >
   void construct(
      T2 * pt2, TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4,
      TArg5 && targ5, TArg6 && targ6, TArg7 && targ7, TArg8 && targ8, TArg9 && targ9
   ) {
      ::new(pt2) T2(
         forward(targ0), forward(targ1), forward(targ2), forward(targ3), forward(targ4),
         forward(targ5), forward(targ6), forward(targ7), forward(targ8), forward(targ9)
      );
   }
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

   /*! Destructs the argument.

   pt2
      Pointer to the object to destruct.
   */
   template <typename T2>
   void destroy(T2 * pt2) {
      pt2->~T2();
   }
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::unique_ptr

namespace std {

//! Smart resource-owning pointer (C++11 § 20.7.1.2 “unique_ptr for single objects”).
template <typename T, typename TDel = default_delete<T>>
class unique_ptr : public ::abc::noncopyable {
public:
   //! Type of the element pointed to.
   typedef T element_type;
   //! Type of the deleter.
   typedef TDel deleter_type;

public:
   /*! Constructor.

   TODO: comment signature.
   */
   /*constexpr*/ unique_ptr() :
      m_pt_and_tdel(nullptr, TDel()) {
   }
   explicit unique_ptr(T * pt) :
      m_pt_and_tdel(pt, TDel()) {
   }
   unique_ptr(
      T * pt, typename conditional<is_reference<TDel>::value, TDel, TDel const &>::type tdel
   ) :
      m_pt_and_tdel(pt, tdel) {
   }
   unique_ptr(T * pt, typename remove_reference<TDel>::type && tdel) :
      m_pt_and_tdel(pt, move(tdel)) {
   }
   unique_ptr(unique_ptr && upt) :
      m_pt_and_tdel(upt.release(), move(upt.get_deleter())) {
   }
   /*constexpr*/ unique_ptr(nullptr_t) :
      m_pt_and_tdel(nullptr, TDel()) {
   }
   template <class T2, typename TDel2>
   unique_ptr(unique_ptr<T2, TDel2> && upt2) :
      m_pt_and_tdel(upt2.release(), move(upt2.get_deleter())) {
   }

   //! Destructor.
   ~unique_ptr() {
      T * pt = get();
      if (pt) {
         get_deleter()(pt);
      }
   }

   /*! Assignment operator.

   upt
      Source object.
   upt2
      Source object.
   return
      *this.
   */
   unique_ptr & operator=(unique_ptr && upt) {
      if (&upt != this) {
         std::get<0>(m_pt_and_tdel) = upt.release();
         std::get<1>(m_pt_and_tdel) = move(upt.get_deleter());
      }
      return *this;
   }
   template <class T2, typename TDel2>
   unique_ptr & operator=(unique_ptr<T2, TDel2> && upt2) {
      std::get<0>(m_pt_and_tdel) = upt2.release();
      std::get<1>(m_pt_and_tdel) = move(upt2.get_deleter());
      return *this;
   }
   unique_ptr & operator=(nullptr_t) {
      reset();
      return *this;
   }

   /*! Dereference operator.

   return
      Reference to the owned object.
   */
   typename add_lvalue_reference<T>::type operator*() const {
      return *get();
   }

   /*! Dereferencing member access operator.

   return
      Pointer to the owned object.
   */
   T * operator->() const {
      return get();
   }

   /*! Boolean evaluation operator.

   return
      true if get() != nullptr, or false otherwise.
   */
   explicit_operator_bool() const {
      return get() != nullptr;
   }

   /*! Returns the wrapped pointer.

   return
      Pointer to the owned object.
   */
   T * get() const {
      return std::get<0>(m_pt_and_tdel);
   }

   /*! Returns the stored deleter.

   TODO: comment signature.
   */
   TDel & get_deleter() {
      return std::get<1>(m_pt_and_tdel);
   }
   TDel const & get_deleter() const {
      return std::get<1>(m_pt_and_tdel);
   }

   /*! Returns the wrapped pointer, and deassociates from it.

   return
      Pointer to the formerly-owned object.
   */
   T * release() {
      T * pt = get();
      std::get<0>(m_pt_and_tdel) = nullptr;
      return pt;
   }

   /*! Deletes the object currently pointed to, if any, and optionally switches to pointing to a
   different object.

   pt
      Pointer to a new object to take ownership of.
   */
   void reset(T * pt = nullptr) {
      T * ptOld = get();
      std::get<0>(m_pt_and_tdel) = move(pt);
      if (ptOld) {
         get_deleter()(ptOld);
      }
   }

protected:
   /*! Wrapper for pointer and deleter. It’s a tuple, so that an empty TDel can end up taking up no
   space at all due to EBO. */
   tuple<T *, TDel> m_pt_and_tdel;
};

// Specialization for dynamically-allocated arrays (C++11 § 20.7.1.3 “unique_ptr for array objects
// with a runtime length”).
template <typename T, typename TDel>
class unique_ptr<T[], TDel> : public ::abc::noncopyable {
public:
   //! Type of the element pointed to.
   typedef T element_type;
   //! Type of the deleter.
   typedef TDel deleter_type;

public:
   /*! Constructor.

   TODO: comment signature.
   */
   /*constexpr*/ unique_ptr() :
      m_pt_and_tdel(nullptr, TDel()) {
   }
   explicit unique_ptr(T * pt) :
      m_pt_and_tdel(pt, TDel()) {
   }
   unique_ptr(
      T * pt, typename conditional<is_reference<TDel>::value, TDel, TDel const &>::type tdel
   ) :
      m_pt_and_tdel(pt, tdel) {
   }
   unique_ptr(T * pt, typename remove_reference<TDel>::type && tdel) :
      m_pt_and_tdel(pt, move(tdel)) {
   }
   unique_ptr(unique_ptr && upt) :
      m_pt_and_tdel(upt.release(), move(upt.get_deleter())) {
   }
   /*constexpr*/ unique_ptr(nullptr_t) :
      m_pt_and_tdel(nullptr, TDel()) {
   }

   //! Destructor.
   ~unique_ptr() {
      reset();
   }

   /*! Assignment operator.

   upt
      Source object.
   return
      *this.
   */
   unique_ptr & operator=(unique_ptr && upt) {
      if (&upt != this) {
         std::get<0>(m_pt_and_tdel) = upt.release();
         std::get<1>(m_pt_and_tdel) = move(upt.get_deleter());
      }
      return *this;
   }
   unique_ptr & operator=(nullptr_t) {
      reset();
      return *this;
   }

   /*! Item access operator.

   TODO: comment signature.
   */
   T & operator[](size_t i) const {
      return get()[i];
   }

   /*! Boolean evaluation operator.

   return
      true if get() != nullptr, or false otherwise.
   */
   explicit_operator_bool() const {
      return get() != nullptr;
   }

   /*! Returns the wrapped pointer.

   return
      Pointer to the owned array.
   */
   T * get() const {
      return std::get<0>(m_pt_and_tdel);
   }

   /*! Returns the stored deleter.

   TODO: comment signature.
   */
   TDel & get_deleter() {
      return std::get<1>(m_pt_and_tdel);
   }
   TDel const & get_deleter() const {
      return std::get<1>(m_pt_and_tdel);
   }

   /*! Returns the wrapped pointer, and deassociates from it.

   return
      Pointer to the formerly-owned array.
   */
   T * release() {
      T * pt = get();
      std::get<0>(m_pt_and_tdel) = nullptr;
      return pt;
   }

   /*! Deletes the object currently pointed to, if any, and optionally switches to pointing to a
   different object.

   pt
      Pointer to a new array to take ownership of.
   */
   void reset(T * pt = nullptr) {
      T * ptOld = get();
      std::get<0>(m_pt_and_tdel) = move(pt);
      if (ptOld) {
         get_deleter()(ptOld);
      }
   };

protected:
   /*! Wrapper for pointer and deleter. It’s a tuple, so that an empty TDel can end up taking up no
   space at all due to EBO. */
   tuple<T *, TDel> m_pt_and_tdel;
};

} //namespace std


// Relational operators for unique_ptr.
#define ABC_RELOP_IMPL(op) \
   template <typename T1, typename TDel1, typename T2, typename TDel2> \
   inline bool operator op( \
      std::unique_ptr<T1, TDel1> const & upt1, std::unique_ptr<T2, TDel2> const & upt2 \
   ) { \
      return upt1.get() op upt2.get(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::bad_weak_ptr

namespace std {

/*! Type of exception thrown by shared_ptr in case of attempt to lock an expired weak_ptr (C++11 §
20.7.2.1 “Class bad_weak_ptr”). */
class bad_weak_ptr : public std::exception {
public:
   //! See exception::exception().
   bad_weak_ptr();

   //! Destructor.
   virtual ~bad_weak_ptr();

   //! See exception::what().
   virtual char const * what() const override;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::_shared_refcount

namespace std {

/*! Base for control object classes used by shared_ptr and weak_ptr.

All strong references to this object collectively hold a weak reference to it; this prevents race
conditions upon release of the last strong reference in absence of other weak references. */
class _shared_refcount : public ::abc::noncopyable {
public:
   /*! Constructor.

   TODO: comment signature.
   */
   _shared_refcount(::abc::atomic::int_t cStrongRefs, ::abc::atomic::int_t cWeakRefs);

   //! Destructor.
   virtual ~_shared_refcount();

   //! Records the creation of a new strong reference to this.
   void add_strong_ref();

   //! Records the creation of a new weak reference to this.
   void add_weak_ref() {
      ::abc::atomic::increment(&m_cWeakRefs);
   }

   /*! Returns the deleter in use by this owner, if any. Used by std::get_deleter().

   TODO: comment signature.
   */
   virtual void * get_deleter(type_info const & ti) const;

   //! Records the release of a strong reference to this.
   void release_strong() {
      if (::abc::atomic::decrement(&m_cStrongRefs) == 0) {
         // All the strong references are gone: release the owned object and the weak link hold by
         // the strong references.
         delete_owned();
         release_weak();
      }
   }

   //! Records the release of a weak reference to this.
   void release_weak() {
      if (::abc::atomic::decrement(&m_cWeakRefs) == 0) {
         // All references are gone, including the one held by all the strong references together:
         // this object can go away as well.
         delete_this();
      }
   }

   /*! Returns the number of strong references to this.

   return
      Reference count.
   */
   long use_count() const {
      return m_cStrongRefs;
   }

protected:
   /*! Destructs the owned object when all the strong references are released, releasing its memory
   if possible (i.e. the owned object doesn’t live on the same memory block as *this). */
   virtual void delete_owned() = 0;

   //! Deletes *this.
   virtual void delete_this();

protected:
   //! Number of shared_ptr references to this.
   ::abc::atomic::int_t volatile m_cStrongRefs;
   //! Number of weak_ptr references to this.
   ::abc::atomic::int_t volatile m_cWeakRefs;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::_basic_shared_refcount

namespace std {

//! Simple control object class with no custom deleter or allocator support.
template <typename T>
class _basic_shared_refcount : public _shared_refcount {
public:
   /*! Constructor.

   TODO: comment signature.
   */
   _basic_shared_refcount(T * pt) :
      _shared_refcount(1, 0),
      m_pt(pt) {
   }

   //! Destructor.
   virtual ~_basic_shared_refcount() {
      ABC_ASSERT(!m_pt);
   }

protected:
   //! See _shared_refcount::delete_owned().
   virtual void delete_owned() override {
      delete m_pt;
      m_pt = nullptr;
   }

protected:
   //! Pointer to the owned object.
   T * m_pt;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::_shared_refcount_with_deleter

namespace std {

//! Control object class with custom deleter support.
template <typename T, typename TDel>
class _shared_refcount_with_deleter : public _basic_shared_refcount<T> {
public:
   /*! Constructor.

   TODO: comment signature.
   */
   _shared_refcount_with_deleter(T * pt, TDel const & tdel) :
      _basic_shared_refcount<T>(pt),
      m_tdel(tdel) {
   }

   //! Destructor.
   virtual ~_shared_refcount_with_deleter() {
   }

   //! See _basic_shared_refcount::get_deleter().
   virtual void * get_deleter(type_info const & ti) const override {
      return ti == typeid(TDel) ? &m_tdel : nullptr;
   }

protected:
   //! See _basic_shared_refcount::delete_owned().
   virtual void delete_owned() override {
      m_tdel(_basic_shared_refcount<T>::m_pt);
      _basic_shared_refcount<T>::m_pt = nullptr;
   }

protected:
   //! Deleter for m_pt.
   TDel m_tdel;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::_prefix_shared_refcount

namespace std {

/*! Special control object class used when instanciating a shared_ptr via make_shared(). It expects
the owned object to be allocated on the same memory block. */
template <typename T>
class _prefix_shared_refcount : public _shared_refcount {
public:
   //! Constructor.
   _prefix_shared_refcount() :
      _shared_refcount(1, 0),
      m_bOwnedConstructed(false) {
   }

   //! Destructor.
   virtual ~_prefix_shared_refcount() {
      ABC_ASSERT(!m_bOwnedConstructed);
   }

   /*! Declares the T that follows *this as constructed, thereby enabling its destruction in
   delete_owned(). */
   void set_owned_constructed() {
      m_bOwnedConstructed = true;
   }

protected:
   //! See _shared_refcount::delete_owned().
   virtual void delete_owned() override {
      if (m_bOwnedConstructed) {
         // Calculate the address of the T that follows *this.
         T * pt = reinterpret_cast<max_align_t *>(this) + ABC_ALIGNED_SIZE(sizeof(*this));
         pt->~T();
         m_bOwnedConstructed = false;
      }
   }

protected:
   //! true if the T that follows *this has been constructed.
   bool m_bOwnedConstructed;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::shared_ptr

namespace std {

// Forward declaration.
template <typename T>
class weak_ptr;

//! Smart resource-sharing pointer (C++11 § 20.7.2.2 “Class template shared_ptr”).
template <typename T>
class shared_ptr : public ::abc::support_explicit_operator_bool<shared_ptr<T>> {
public:
   //! Type of the element pointed to.
   typedef T element_type;

public:
   /*! Constructor. Some of the nullptr_t overloads have been collapsed into the generic pointer
   overloads.

   TODO: comment signature.
   */
   /*constexpr*/ shared_ptr() :
      m_psr(nullptr),
      m_pt(nullptr) {
   }
   template <typename T2>
   explicit shared_ptr(T2 * pt2) try :
      m_psr(new _basic_shared_refcount<T2>(pt2)),
      m_pt(pt2) {
   } catch (...) {
      delete pt2;
      throw;
   }
   template <typename T2, typename TDel>
   shared_ptr(T2 * pt2, TDel tdel) try :
      m_psr(new _shared_refcount_with_deleter<T2, TDel>(pt2, tdel)),
      m_pt(pt2) {
   } catch (...) {
      tdel(pt2);
      throw;
   }
   template <typename T2, typename TDel, class TAllocator>
   shared_ptr(T2 * pt2, TDel tdel, TAllocator talloc);
   template <typename T2>
   shared_ptr(shared_ptr<T2> const & spt2, T * pt);
   shared_ptr(shared_ptr const & spt) :
      m_psr(spt.m_psr),
      m_pt(spt.m_pt) {
      if (m_psr) {
         m_psr->add_strong_ref();
      }
   }
   template <typename T2>
   shared_ptr(shared_ptr<T2> const & spt2) :
      m_psr(spt2.m_psr),
      m_pt(spt2.m_pt) {
      if (m_psr) {
         m_psr->add_strong_ref();
      }
   }
   shared_ptr(shared_ptr && spt) :
      m_psr(spt.m_psr),
      m_pt(spt.m_pt) {
      spt.m_psr = nullptr;
      spt.m_pt = nullptr;
   }
   template <typename T2>
   shared_ptr(shared_ptr<T2> && spt2) :
      m_psr(spt2.m_psr),
      m_pt(spt2.m_pt) {
      spt2.m_psr = nullptr;
      spt2.m_pt = nullptr;
   }
   template <typename T2>
   explicit shared_ptr(weak_ptr<T2> const & wpt2);
   template <typename T2, typename TDel>
   shared_ptr(unique_ptr<T2, TDel> && upt2) :
      m_psr(new _shared_refcount_with_deleter<T2, TDel>(
         upt2.get(), upt2.get_deleter()
      )),
      m_pt(upt2.release()) {
   }
   /*constexpr*/ shared_ptr(nullptr_t) {
      m_psr(new _basic_shared_refcount<nullptr_t>(nullptr)),
      m_pt(nullptr) {
   }

   //! Destructor.
   ~shared_ptr() {
      if (m_psr) {
         m_psr->release_strong();
      }
   }

   /*! Assignment operator.

   spt
      Source object.
   spt2
      Source object.
   return
      *this.
   */
   shared_ptr & operator=(shared_ptr const & spt) {
      if (&spt != this) {
         T * psrNew = spt.m_psr;
         if (psrNew) {
            psrNew->add_strong_ref();
         }
         if (m_psr) {
            m_psr->release_strong();
         }
         m_psr = psrNew;
         m_pt = spt.m_pt;
      }
      return *this;
   }
   template <class T2>
   shared_ptr & operator=(shared_ptr<T2> const & spt2) {
      return operator=(shared_ptr(spt2));
   }
   shared_ptr & operator=(shared_ptr && spt) {
      if (&spt != this) {
         if (m_psr) {
            m_psr->release_strong();
         }
         m_psr = spt.m_psr;
         m_pt = spt.m_pt;
         spt.m_psr = nullptr;
         spt.m_pt = nullptr;
      }
      return *this;
   }
   template <class T2>
   shared_ptr & operator=(shared_ptr<T2> && spt2) {
      return operator=(shared_ptr(move(spt2)));
   }
   template <class T2, typename TDel>
   shared_ptr & operator=(unique_ptr<T2, TDel> && upt2) {
      return operator=(shared_ptr(move(upt2)));
   }

   /*! Dereference operator.

   TODO: comment signature.
   */
   typename add_lvalue_reference<T>::type operator*() const {
      return *m_pt;
   }

   /*! Dereferencing member access operator.

   TODO: comment signature.
   */
   T * operator->() const {
      return m_pt;
   }

   /*! Boolean evaluation operator.

   TODO: comment signature.
   */
   explicit_operator_bool() const {
      return m_pt != nullptr;
   }

   //! Releases the object pointed to, optionally assigning a new pointer.
   void reset() {
      if (m_psr) {
         m_psr->release_strong();
         m_psr = nullptr;
         m_pt = nullptr;
      }
   }
   template <typename T2>
   void reset(T2 * pt) {
      operator=(shared_ptr(pt));
   }
   template <typename T2, typename TDel>
   void reset(T2 * pt2, TDel tdel) {
      operator=(shared_ptr(pt2, tdel));
   }
   template <typename T2, typename TDel, class TAllocator>
   void reset(T2 * pt2, TDel tdel, TAllocator talloc) {
      operator=(shared_ptr(pt2, tdel, talloc));
   }

   /*! Returns the wrapped pointer.

   return
      Pointer to the owned object.
   */
   T * get() const {
      return m_pt;
   }

   //! TODO: comment.
   template <class T2>
   bool owner_before(shared_ptr<T2> const & spt2) const;
   template <class T2>
   bool owner_before(weak_ptr<T2> const & wpt2) const;

   /*! Returns true if no other pointers are referring to the object pointed to.

   return
      true if *this is the only pointer to the owned object, or false otherwise.
   */
   bool unique() const {
      return use_count() == 1;
   }

   /*! Returns the number of references to the object pointed to.

   return
      Reference count.
   */
   long use_count() const {
      return m_psr ? m_psr->use_count() : 0;
   }

#ifdef ABC_CXX_TEMPLATE_FRIENDS
protected:
#else
public:
#endif
   /*! Constructor. Non-standard.

   TODO: comment signature.
   */
   shared_ptr(_shared_refcount * psr, T * pt) :
      m_psr(psr),
      m_pt(pt) {
   }

   /*! Returns a pointer to the shared object owner. Non-standard.

   TODO: comment signature.
   */
   _shared_refcount * get_shared_refcount() const {
      return m_psr;
   }

protected:
   //! Shared reference count. We hold a strong reference to it.
   _shared_refcount * m_psr;
   //! Owned object.
   T * m_pt;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std::weak_ptr

namespace std {

/*! Non-owning pointer that providess access to shared_ptr (C++11 § 20.7.2.3 “Class template
weak_ptr”). */
template <typename T>
class weak_ptr {
public:
   //! Type of the element pointed to.
   typedef T element_type;

public:
   /*! Constructor.

   TODO: comment signature.
   */
   weak_ptr() :
      m_psr(nullptr),
      m_pt(nullptr) {
   }
   weak_ptr(weak_ptr const & wpt) :
      m_psr(wpt.m_psr),
      m_pt(wpt.m_pt) {
      if (m_psr) {
         m_psr->add_weak_ref();
      }
   }
   template <typename T2>
   weak_ptr(weak_ptr<T2> const & wpt2) :
      m_psr(wpt2.m_psr),
      m_pt(wpt2.m_pt) {
      if (m_psr) {
         m_psr->add_weak_ref();
      }
   }
   template <typename T2>
   weak_ptr(shared_ptr<T2> const & spt2) :
      m_psr(spt2.get_shared_refcount()),
      m_pt(spt2.get()) {
      if (m_psr) {
         m_psr->add_weak_ref();
      }
   }
   // Non-standard.
   weak_ptr(weak_ptr && wpt) :
      m_psr(wpt.m_psr),
      m_pt(wpt.m_pt) {
      wpt.m_psr = nullptr;
      wpt.m_pt = nullptr;
   }

   //! Destructor.
   ~weak_ptr() {
      if (m_psr) {
         m_psr->release_weak();
      }
   }

   /*! Assignment operator.

   wpt
      Source object.
   wpt2
      Source object.
   return
      *this.
   */
   weak_ptr & operator=(weak_ptr const & wpt) {
      if (&wpt != this) {
         T * psrNew = wpt.m_psr;
         if (psrNew) {
            psrNew->add_weak_ref();
         }
         if (m_psr) {
            m_psr->release_weak();
         }
         m_psr = psrNew;
         m_pt = wpt.m_pt;
      }
      return *this;
   }
   template <typename T2>
   weak_ptr & operator=(weak_ptr<T2> const & wpt2) {
      return operator=(weak_ptr(wpt2));
   }
   template <typename T2>
   weak_ptr & operator=(shared_ptr<T2> const & spt2) {
      return operator=(weak_ptr(spt2));
   }
   // Non-standard.
   weak_ptr & operator=(weak_ptr && wpt) {
      if (&wpt != this) {
         if (m_psr) {
            m_psr->release_weak();
         }
         m_psr = wpt.m_psr;
         m_pt = wpt.m_pt;
         wpt.m_psr = nullptr;
         wpt.m_pt = nullptr;
      }
      return *this;
   }

   /*! Returns true if the object pointed to no longer exists.

   return
      true if the object pointed to has been deleted, or false otherwise.
   */
   bool expired() const {
      return use_count() == 0;
   }

   /*! Returns a non-weak pointer to the object pointed to.

   return
      Shared pointer to the watched object.
   */
   shared_ptr<T> lock() const {
      return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
   }

   //! TODO: comment.
   template <class T2>
   bool owner_before(shared_ptr<T2> const & spt2) const;
   template <class T2>
   bool owner_before(weak_ptr<T2> const & wpt2) const;

   //! Releases the object pointed to.
   void reset() {
      if (m_psr) {
         m_psr->release_weak();
         m_psr = nullptr;
         m_pt = nullptr;
      }
   }

   /*! Returns the number of strong references to the object pointed to.

   return
      Count of strong references to the watched object.
   */
   long use_count() const {
      return m_psr ? m_psr->use_count() : 0;
   }

#ifdef ABC_CXX_TEMPLATE_FRIENDS
protected:
#else
public:
#endif
   /*! Constructor. Non-standard, used by make_shared().

   TODO: comment signature.
   */
   weak_ptr(_shared_refcount * psr, T * pt) :
      m_psr(psr),
      m_pt(pt) {
      m_psr->add_weak_ref();
   }

   /*! Returns a pointer to the shared object owner. Non-standard.

   TODO: comment signature.
   */
   _shared_refcount * get_shared_refcount() const {
      return m_psr;
   }

   /*! Returns a pointer to the shared object owner. Non-standard.

   TODO: comment signature.
   */
   T * get() const {
      return m_pt;
   }

protected:
   //! Shared reference count. We hold a weak reference to it.
   _shared_refcount * m_psr;
   /*! Weakly-owned object. Not to be used directly; we only keep it to pass it when constructing a
   shared_ptr. */
   T * m_pt;
};


// Now these can be implemented.

template <typename T>
template <typename T2>
inline shared_ptr<T>::shared_ptr(weak_ptr<T2> const & wpt2) :
   m_psr(wpt2->get_shared_refcount()),
   m_pt(wpt2->get()) {
   m_psr->add_strong_ref();
}

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace std {

/*! Base class for sharable objects (C++11 § 20.7.2.4 “Class template enable_shared_from_this”).

The way this works is that when the last shared_ptr to this is released, the embedded refcount will
call delete on this, which will self-destruct the refcount.

TODO: initialize m_pThis. Probably need a _shared_refcount somewhere to function. */
template <typename T>
class enable_shared_from_this {
public:
   /*! Returns a shared pointer to this.

   return
      Pointer to *this.
   */
   shared_ptr<T> shared_from_this() {
      return shared_ptr<T>(m_pThis);
   }
   shared_ptr<T const> shared_from_this() const {
      return shared_ptr<T const>(m_pThis);
   }

protected:
   /*! Constructor.

   esft
      Ignored.
   */
   enable_shared_from_this() {
   }
   enable_shared_from_this(enable_shared_from_this const & esft) {
      ABC_UNUSED_ARG(esft);
   }

   //! Destructor.
   ~enable_shared_from_this() {
   }

   /*! Assignment operator.

   esft
      Ignored.
   */
   enable_shared_from_this & operator=(enable_shared_from_this const & esft) {
      ABC_UNUSED_ARG(esft);
      return *this;
   }

private:
   //! Weak reference to *this.
   weak_ptr<T> m_pThis;
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// std globals – smart pointers-related

namespace std {

/*! Similar to make_shared(), except it uses a custom allocator.

TODO: comment signature. */
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename T, class TAllocator, typename... TArgs>
inline shared_ptr<T> allocate_shared(TAllocator const & talloc, TArgs &&... targs) {
   // Allocate a block of memory large enough to contain a refcount object and a T instance, making
   // sure the T has proper alignment.
   max_align_t * p = new max_align_t[
      ABC_ALIGNED_SIZE(sizeof(_prefix_shared_refcount<T>)) + ABC_ALIGNED_SIZE(sizeof(T))
   ];
   T * pt = p + ABC_ALIGNED_SIZE(sizeof(_prefix_shared_refcount<T>));
   // Construct and return a raw shared_ptr, also constructing the refcount object on the fly.
   // Note that we’ll only call set_owned_constructed() on the refcount after T::T() succeeds; in
   // case this throws, shared_ptr::~shared_ptr() will call m_psr->release_strong(), but this won’t
   // attempt to destruct the unconstructed T object because it hasn’t been told that the object was
   // constructed. This also avoids the need for exception handling.
   shared_ptr<T> spt(::new(p) _prefix_shared_refcount<T>(), pt);
   // Read comments in _make_unconstructed_shared() to see why this is really exception-proof.
   ::new(spt.get()) T(forward(targs) ...);
   static_cast<_prefix_shared_refcount<T> *>(spt.get_shared_refcount())->set_owned_constructed();
   return move(spt);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

// TODO: non-template version!

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

/*! Returns the deleter in use by the specified shared_ptr, if any (C++11 § 20.7.2.2.10
“get_deleter”).

TODO: comment signature.
*/
template <typename TDel, typename T>
inline TDel * get_deleter(shared_ptr<T> const & spt) {
   _shared_refcount * psr = spt.get_shared_refcount();
   if (psr) {
      return static_cast<TDel *>(psr->get_deleter(typeid(TDel)));
   } else {
      return nullptr;
   }
}

/*! Dynamically allocates an object, using the same memory block to store any dynamic memory used by
the returned shared pointer to it.

TODO: comment signature.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename T, typename... TArgs>
inline shared_ptr<T> make_shared(TArgs &&... targs) {
   return allocate_shared<T, allocator, TArgs ...>(targs ...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename T>
inline shared_ptr<T> make_shared() {
}
// Overload for 1-argument T::T().
template <typename T, typename TArg0>
inline shared_ptr<T> make_shared(TArg0 && targ0) {
}
// Overload for 2-argument T::T().
template <typename T, typename TArg0, typename TArg1>
inline shared_ptr<T> make_shared(TArg0 && targ0, TArg1 && targ1) {
}
// Overload for 3-argument T::T().
template <typename T, typename TArg0, typename TArg1, typename TArg2>
inline shared_ptr<T> make_shared(TArg0 && targ0, TArg1 && targ1, TArg2 && targ2) {
}
// Overload for 4-argument T::T().
template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
inline shared_ptr<T> make_shared(TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3) {
}
// Overload for 5-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4
) {
}
// Overload for 6-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
   typename TArg5
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4, TArg5 && targ5
) {
}
// Overload for 7-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
   typename TArg5, typename TArg6
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4, TArg5 && targ5,
   TArg6 && targ6
) {
}
// Overload for 8-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
   typename TArg5, typename TArg6, typename TArg7
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4, TArg5 && targ5,
   TArg6 && targ6, TArg7 && targ7
) {
}
// Overload for 9-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
   typename TArg5, typename TArg6, typename TArg7, typename TArg8
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4, TArg5 && targ5,
   TArg6 && targ6, TArg7 && targ7, TArg8 && targ8
) {
}
// Overload for 10-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
   typename TArg5, typename TArg6, typename TArg7, typename TArg8, typename TArg9
>
inline shared_ptr<T> make_shared(
   TArg0 && targ0, TArg1 && targ1, TArg2 && targ2, TArg3 && targ3, TArg4 && targ4, TArg5 && targ5,
   TArg6 && targ6, TArg7 && targ7, TArg8 && targ8, TArg9 && targ9
) {
}

/*! Helper for make_shared(); handles everything except calling T::T().

TODO: comment signature.
*/
template <typename T>
inline shared_ptr<T> _make_unconstructed_shared() {
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STL_MEMORY_HXX


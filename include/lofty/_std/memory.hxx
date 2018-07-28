/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_MEMORY_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_MEMORY_HXX
#endif

#ifndef _LOFTY_STD_MEMORY_HXX_NOPUB
#define _LOFTY_STD_MEMORY_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* MSC16 BUG: has a half-assed std::shared_ptr that requires the type’s destructor to be defined at the point
of declaration of the pointer. */
#if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600

#include <lofty/explicit_operator_bool.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/type_traits.hxx>
#include <lofty/_std/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Deallocator functor that invokes delete on its argument (C++11 § 20.7.1.1 “Default deleters”).
template <typename T>
class default_delete {
public:
   //! Default constructor.
   /*constexpr*/ default_delete() {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename U>
   default_delete(default_delete<U> const & src) {
      LOFTY_UNUSED_ARG(src);
   }

   /*! Function call operator.

   @param t
      Pointer to the object to delete.
   */
   void operator()(T * t) const {
      delete t;
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

   @param t
      Pointer to the array to delete.
   */
   void operator()(T * t) const {
      delete[] t;
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

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

   template <typename U>
   struct rebind {
      typedef allocator<U> other;
   };
};

template <typename T>
class allocator {
public:
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;
   typedef T * pointer;
   typedef T const * const_pointer;
   typedef T & reference;
   typedef T const & const_reference;
   typedef T value_type;

   template <typename U>
   struct rebind {
      typedef allocator<U> other;
   };

public:
   //! Default constructor.
   allocator() {
   }

   //! Copy constructor.
   allocator(allocator const &) {
   }

   //! Copy constructor.
   template <typename U>
   allocator(allocator<U> const &) {
   }

   //! Destructor.
   ~allocator() {
   }

   /*! Returns a pointer to the argument.

   @param t
      Variable to return the address of.
   @return
      Pointer to t.
   */
   pointer address(reference t) const {
      return &t;
   }

   /*! Returns a const pointer to the argument.

   @param t
      Variable to return the address of.
   @return
      Const pointer to t.
   */
   const_pointer address(const_reference t) const {
      return &t;
   }

   /*! Allocates memory for the requested number of T objects.

   @param count
      Count of objects to allocate.
   @param hint
      Pointer to use as allcation hint.
   @return
      Newly allocated storage for count T objects.
   */
   pointer allocate(size_type count, allocator<void>::const_pointer hint = nullptr) {
      LOFTY_UNUSED_ARG(hint);
      return reinterpret_cast<T *>(::new max_align_t[LOFTY_ALIGNED_SIZE(sizeof(T) * count)]);
   }

   /*! Releases memory obtained through allocate().

   @param p
      Storage for count T objects.
   @param count
      Count of objects to deallocate.
   */
   void deallocate(pointer p, size_type count) {
      LOFTY_UNUSED_ARG(count);
      ::delete static_cast<void *>(p);
   }

   /*! Returns the meximum number of T objects that allocate() can succeed for.

   @return
      allocate() count limit.
   */
   size_type max_size() const {
      return ~size_type(0) / sizeof(T);
   }

   /*! Construct an object at the specified address, with the provided constructor arguments.

   @param p
      Pointer to uninitialized storage of size at least equal to sizeof(U).
   @param args
      Arguments to construct a U instance with.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename U, typename... TArgs>
   void construct(U * p, TArgs &&... args) {
      ::new(p) U(forward(args) ...);
   }
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename U>
   void construct(U * p) {
      ::new(p) U();
   }
   template <typename U, typename TArg0>
   void construct(U * p, TArg0 && arg0) {
      ::new(p) U(forward(arg0));
   }
   template <typename U, typename TArg0, typename TArg1>
   void construct(U * p, TArg0 && arg0, TArg1 && arg1) {
      ::new(p) U(forward(arg0), forward(arg1));
   }
   template <typename U, typename TArg0, typename TArg1, typename TArg2>
   void construct(U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2) {
      ::new(p) U(forward(arg0), forward(arg1), forward(arg2));
   }
   template <typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
   void construct(U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3) {
      ::new(p) U(forward(arg0), forward(arg1), forward(arg2), forward(arg3));
   }
   template <typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4>
   void construct(U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4) {
      ::new(p) U(forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4));
   }
   template <
      typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5
   >
   void construct(
      U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5
   ) {
      ::new(p) U(forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4), forward(arg5));
   }
   template <
      typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6
   >
   void construct(
      U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5,
      TArg6 && arg6
   ) {
      ::new(p) U(
         forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4), forward(arg5),
         forward(arg6)
      );
   }
   template <
      typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7
   >
   void construct(
      U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5,
      TArg6 && arg6, TArg7 && arg7
   ) {
      ::new(p) U(
         forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4), forward(arg5),
         forward(arg6), forward(arg7)
      );
   }
   template <
      typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8
   >
   void construct(
      U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5,
      TArg6 && arg6, TArg7 && arg7, TArg8 && arg8
   ) {
      ::new(p) U(
         forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4), forward(arg5),
         forward(arg6), forward(arg7), forward(arg8)
      );
   }
   template <
      typename U, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8, typename TArg9
   >
   void construct(
      U * p, TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5,
      TArg6 && arg6, TArg7 && arg7, TArg8 && arg8, TArg9 && arg9
   ) {
      ::new(p) U(
         forward(arg0), forward(arg1), forward(arg2), forward(arg3), forward(arg4), forward(arg5),
         forward(arg6), forward(arg7), forward(arg8), forward(arg9)
      );
   }
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Destructs the argument.

   @param u
      Pointer to the object to destruct.
   */
   template <typename U>
   void destroy(U * u) {
      u->~U();
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Smart resource-owning pointer (C++11 § 20.7.1.2 “unique_ptr for single objects”).
template <typename T, typename TDel = default_delete<T>>
class unique_ptr : public noncopyable, public support_explicit_operator_bool<unique_ptr<T, TDel>> {
public:
   //! Type of the element pointed to.
   typedef T element_type;
   //! Type of the deleter.
   typedef TDel deleter_type;

public:
   /*! Constructor.

   @param t
      Pointer to acquire ownership of.
   @param del
      Deleter to use to delete t.
   @param src
      Pointer to transfer ownership from. U and UDel must be convertible to T and TDel, respectively.
   */
   /*constexpr*/ unique_ptr() :
      t_and_del(nullptr, TDel()) {
   }
   explicit unique_ptr(T * t) :
      t_and_del(t, TDel()) {
   }
   unique_ptr(T * t, typename conditional<is_reference<TDel>::value, TDel, TDel const &>::type del) :
      t_and_del(t, del) {
   }
   unique_ptr(T * t, typename remove_reference<TDel>::type && del) :
      t_and_del(t, move(del)) {
   }
   unique_ptr(unique_ptr && src) :
      t_and_del(src.release(), move(src.get_deleter())) {
   }
   /*constexpr*/ unique_ptr(nullptr_t) :
      t_and_del(nullptr, TDel()) {
   }
   template <typename U, typename UDel>
   unique_ptr(unique_ptr<U, UDel> && src) :
      t_and_del(src.release(), move(src.get_deleter())) {
   }

   //! Destructor.
   ~unique_ptr() {
      T * t = get();
      if (t) {
         get_deleter()(t);
      }
   }

   /*! Assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   unique_ptr & operator=(unique_ptr && src) {
      if (&src != this) {
         _std::get<0>(t_and_del) = src.release();
         _std::get<1>(t_and_del) = move(src.get_deleter());
      }
      return *this;
   }
   template <typename U, typename UDel>
   unique_ptr & operator=(unique_ptr<U, UDel> && src) {
      _std::get<0>(t_and_del) = src.release();
      _std::get<1>(t_and_del) = move(src.get_deleter());
      return *this;
   }
   unique_ptr & operator=(nullptr_t) {
      reset();
      return *this;
   }

   /*! Dereference operator.

   @return
      Reference to the owned object.
   */
   typename add_lvalue_reference<T>::type operator*() const {
      return *get();
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the owned object.
   */
   T * operator->() const {
      return get();
   }

   /*! Boolean evaluation operator.

   @return
      true if the pointer is not nullptr, or false if it is.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return get() != nullptr;
   }

   /*! Returns the wrapped pointer.

   @return
      Pointer to the owned object.
   */
   T * get() const {
      return _std::get<0>(t_and_del);
   }

   /*! Returns the stored deleter.

   @return
      Reference to the deleter.
   */
   TDel & get_deleter() {
      return _std::get<1>(t_and_del);
   }
   TDel const & get_deleter() const {
      return _std::get<1>(t_and_del);
   }

   /*! Returns the wrapped pointer, and deassociates from it.

   @return
      Pointer to the formerly-owned object.
   */
   T * release() {
      T * pt = get();
      _std::get<0>(t_and_del) = nullptr;
      return pt;
   }

   /*! Deletes the object currently pointed to, if any, and optionally switches to pointing to a different
   object.

   @param t
      Pointer to a new object to take ownership of.
   */
   void reset(T * t = nullptr) {
      T * old_t = get();
      _std::get<0>(t_and_del) = move(t);
      if (old_t) {
         get_deleter()(old_t);
      }
   }

private:
   /*! Wrapper for pointer and deleter. It’s a tuple, so that an empty TDel can end up taking up no space at
   all due to EBO. */
   tuple<T *, TDel> t_and_del;
};

/* Specialization for dynamically-allocated arrays (C++11 § 20.7.1.3 “unique_ptr for array objects with a
runtime length”). */
template <typename T, typename TDel>
class unique_ptr<T[], TDel> :
   public noncopyable,
   public support_explicit_operator_bool<unique_ptr<T[], TDel>> {
public:
   //! Type of the element pointed to.
   typedef T element_type;
   //! Type of the deleter.
   typedef TDel deleter_type;

public:
   /*! Constructor.

   @param t
      Pointer to acquire ownership of.
   @param del
      Deleter to use to delete t.
   @param src
      Pointer to transfer ownership from.
   */
   /*constexpr*/ unique_ptr() :
      t_and_del(nullptr, TDel()) {
   }
   explicit unique_ptr(T * t) :
      t_and_del(t, TDel()) {
   }
   unique_ptr(T * t, typename conditional<is_reference<TDel>::value, TDel, TDel const &>::type del) :
      t_and_del(t, del) {
   }
   unique_ptr(T * t, typename remove_reference<TDel>::type && del) :
      t_and_del(t, move(del)) {
   }
   unique_ptr(unique_ptr && src) :
      t_and_del(src.release(), move(src.get_deleter())) {
   }
   /*constexpr*/ unique_ptr(nullptr_t) :
      t_and_del(nullptr, TDel()) {
   }

   //! Destructor.
   ~unique_ptr() {
      reset();
   }

   /*! Assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   unique_ptr & operator=(unique_ptr && src) {
      if (&src != this) {
         _std::get<0>(t_and_del) = src.release();
         _std::get<1>(t_and_del) = move(src.get_deleter());
      }
      return *this;
   }
   unique_ptr & operator=(nullptr_t) {
      reset();
      return *this;
   }

   /*! Element access operator.

   @param i
      Index of the element.
   @return
      Reference to the requested element.
   */
   T & operator[](std::size_t i) const {
      return get()[i];
   }

   /*! Boolean evaluation operator.

   @return
      true if the pointer is not nullptr, or false if it is.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return get() != nullptr;
   }

   /*! Returns the wrapped pointer.

   @return
      Pointer to the owned array.
   */
   T * get() const {
      return _std::get<0>(t_and_del);
   }

   /*! Returns the stored deleter.

   @return
      Reference to the deleter.
   */
   TDel & get_deleter() {
      return _std::get<1>(t_and_del);
   }
   TDel const & get_deleter() const {
      return _std::get<1>(t_and_del);
   }

   /*! Returns the wrapped pointer, and deassociates from it.

   @return
      Pointer to the formerly-owned array.
   */
   T * release() {
      T * t = get();
      _std::get<0>(t_and_del) = nullptr;
      return t;
   }

   /*! Deletes the object currently pointed to, if any, and optionally switches to pointing to a different
   object.

   @param t
      Pointer to a new array to take ownership of.
   */
   void reset(T * t = nullptr) {
      T * old_t = get();
      _std::get<0>(t_and_del) = move(t);
      if (old_t) {
         get_deleter()(old_t);
      }
   };

private:
   /*! Wrapper for pointer and deleter. It’s a tuple, so that an empty TDel can end up taking up no space at
   all due to EBO. */
   tuple<T *, TDel> t_and_del;
};

// Relational operators for unique_ptr.
#define LOFTY_RELOP_IMPL(op) \
   template <typename T, typename TDel, typename U, typename UDel> \
   inline bool operator op(unique_ptr<T, TDel> const & left, unique_ptr<U, UDel> const & right) { \
      return left.get() op right.get(); \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Type of exception thrown by shared_ptr in case of attempt to lock an expired weak_ptr (C++11 § 20.7.2.1
“Class bad_weak_ptr”). */
class LOFTY_SYM bad_weak_ptr : public exception {
public:
   //! Default constructor.
   bad_weak_ptr();

   //! Destructor.
   virtual ~bad_weak_ptr();

   //! See exception::what().
   virtual char const * what() const override;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

/*! Base for control object classes used by shared_ptr and weak_ptr.

All strong references to this object collectively hold a weak reference to it; this prevents race conditions
upon release of the last strong reference in absence of other weak references. */
class LOFTY_SYM shared_refcount : public noncopyable {
public:
   /*! Constructor.

   @param strong_refs_
      Initial count of strong references.
   @param weak_refs_
      Initial count of weak references.
   */
   shared_refcount(unsigned strong_refs_, unsigned weak_refs_);

   //! Destructor.
   virtual ~shared_refcount();

   //! Records the creation of a new strong reference to this.
   void add_strong_ref();

   //! Records the creation of a new weak reference to this.
   void add_weak_ref() {
      weak_refs.fetch_add(1);
   }

   /*! Returns the deleter in use by this owner, if any. Used by shared_ptr::get_deleter().

   @param ti
      Expected type of deleter.
   @return
      Pointer to the deleter.
   */
   virtual void * get_deleter(type_info const & ti) const;

   //! Records the release of a strong reference to this.
   void release_strong() {
      if (strong_refs.fetch_sub(1) == 1) {
         /* All the strong references are gone: release the owned object and the weak link hold by the strong
         references. */
         delete_owned();
         release_weak();
      }
   }

   //! Records the release of a weak reference to this.
   void release_weak() {
      if (weak_refs.fetch_sub(1) == 1) {
         /* All references are gone, including the one held by all the strong references together: this object
         can go away as well. */
         delete_this();
      }
   }

   /*! Returns the number of strong references to this.

   @return
      Reference count.
   */
   long use_count() const {
      return static_cast<long>(strong_refs.load());
   }

protected:
   /*! Destructs the owned object when all the strong references are released, releasing its memory if
   possible (i.e. the owned object doesn’t live on the same memory block as *this). */
   virtual void delete_owned() = 0;

   //! Deletes *this.
   virtual void delete_this();

protected:
   //! Number of shared_ptr references to this.
   atomic<unsigned> strong_refs;
   //! Number of weak_ptr references to this.
   atomic<unsigned> weak_refs;
};

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

//! Simple control object class with no custom deleter or allocator support.
template <typename T>
class basic_shared_refcount : public shared_refcount {
public:
   /*! Constructor.

   @param t_
      Pointer to take ownership of.
   */
   basic_shared_refcount(T * t_) :
      shared_refcount(1, 0),
      t(t_) {
   }

   //! Destructor.
   virtual ~basic_shared_refcount() {
      //LOFTY_ASSERT(!t);
   }

protected:
   //! See shared_refcount::delete_owned().
   virtual void delete_owned() override {
      delete t;
      t = nullptr;
   }

protected:
   //! Pointer to the owned object.
   T * t;
};

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

//! Control object class with custom deleter support.
template <typename T, typename TDel>
class shared_refcount_with_deleter : public basic_shared_refcount<T> {
public:
   /*! Constructor.

   @param t_
      Pointer to the object to manage.
   @param del_
      Deleter to use on t_.
   */
   shared_refcount_with_deleter(T * t_, TDel const & del_) :
      basic_shared_refcount<T>(t_),
      del(del_) {
   }

   //! Destructor.
   virtual ~shared_refcount_with_deleter() {
   }

   //! See basic_shared_refcount::get_deleter().
   virtual void * get_deleter(type_info const & ti) const override {
      return ti == typeid(TDel) ? &del : nullptr;
   }

protected:
   //! See basic_shared_refcount::delete_owned().
   virtual void delete_owned() override {
      del(basic_shared_refcount<T>::t);
      basic_shared_refcount<T>::t = nullptr;
   }

protected:
   //! Deleter for t.
   TDel del;
};

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

/*! Special control object class used when instanciating a shared_ptr via make_shared(). It expects the owned
object to be allocated on the same memory block. */
template <typename T>
class prefix_shared_refcount : public shared_refcount {
public:
   //! Default constructor.
   prefix_shared_refcount() :
      shared_refcount(1, 0) {
   }

   //! Destructor.
   virtual ~prefix_shared_refcount() {
   }

protected:
   //! See shared_refcount::delete_owned().
   virtual void delete_owned() override {
      // Calculate the address of the T that follows *this.
      auto t = reinterpret_cast<T *>(
         reinterpret_cast<max_align_t *>(this) + LOFTY_ALIGNED_SIZE(sizeof(*this))
      );
      t->~T();
   }
};

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Smart resource-sharing pointer (C++11 § 20.7.2.2 “Class template shared_ptr”).
template <typename T>
class shared_ptr : public support_explicit_operator_bool<shared_ptr<T>> {
private:
   template <typename T2>
   friend class shared_ptr;

   template <typename T2>
   friend class weak_ptr;

   template <typename TDel, typename T2>
   friend TDel * get_deleter(shared_ptr<T2> const & ptr);

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename T2, typename... TArgs>
   friend shared_ptr<T2> make_shared(TArgs &&... args);
#else
   template <typename T2>
   friend shared_ptr<T2> make_shared();
   template <typename T2, typename TArg0>
   friend shared_ptr<T2> make_shared(TArg0 && arg0);
   template <typename T2, typename TArg0, typename TArg1>
   friend shared_ptr<T2> make_shared(TArg0 && arg0, TArg1 && arg1);
   template <typename T2, typename TArg0, typename TArg1, typename TArg2>
   friend shared_ptr<T2> make_shared(TArg0 && arg0, TArg1 && arg1, TArg2 && arg2);
   template <typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
   friend shared_ptr<T2> make_shared(TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3);
   template <typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4>
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4
   );
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5
   >
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5
   );
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6
   >
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6
   );
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7
   >
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
      TArg7 && arg7
   );
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8
   >
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
      TArg7 && arg7, TArg8 && arg8
   );
   template <
      typename T2, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4,
      typename TArg5, typename TArg6, typename TArg7, typename TArg8, typename TArg9
   >
   friend shared_ptr<T2> make_shared(
      TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
      TArg7 && arg7, TArg8 && arg8, TArg9 && arg9
   );
#endif

public:
   //! Type of the element pointed to.
   typedef T element_type;

public:
   //! Default constructor.
   /*constexpr*/ shared_ptr() :
      sr(nullptr),
      t(nullptr) {
   }

   /*! Constructor from a raw pointer with implicit pointer conversion.

   @param u
      Source pointer.
   */
   template <typename U>
   explicit shared_ptr(U * u) :
      sr(new _pvt::basic_shared_refcount<U>(u)),
      t(u) {
   }

   /*! Constructor from a raw pointer with implicit pointer conversion, with deleter.

   @param u
      Source pointer.
   @param del
      Deleter to store.
   */
   template <typename U, typename TDel>
   shared_ptr(U * u, TDel del) :
      sr(new _pvt::shared_refcount_with_deleter<U, TDel>(u, del)),
      t(u) {
   }

   /*! Constructor from a raw pointer with implicit pointer conversion, with deleter and allocator.

   @param u
      Source pointer.
   @param del
      Deleter to store.
   @param alloc
      Allocator to store.
   */
   template <typename U, typename TDel, class TAllocator>
   shared_ptr(U * u, TDel del, TAllocator alloc); // TODO

   /*! Move constructor.

   @param src
      Source object.
   */
   shared_ptr(shared_ptr && src) :
      sr(src.sr),
      t(src.t) {
      src.sr = nullptr;
      src.t = nullptr;
   }

   /*! Move constructor with implicit pointer conversion.

   @param src
      Source object.
   */
   template <typename U>
   shared_ptr(shared_ptr<U> && src) :
      sr(src.sr),
      t(src.t) {
      src.sr = nullptr;
      src.t = nullptr;
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   shared_ptr(shared_ptr const & src) :
      sr(src.sr),
      t(src.t) {
      if (sr) {
         sr->add_strong_ref();
      }
   }

   /*! Copy constructor with implicit pointer conversion.

   @param src
      Source object.
   */
   template <typename U>
   shared_ptr(shared_ptr<U> const & src) :
      sr(src.sr),
      t(src.t) {
      if (sr) {
         sr->add_strong_ref();
      }
   }

   /*! Constructor that shares the reference count with another shared_ptr instance, but sets a different
   pointer that the other instance.

   @param src
      Source object.
   @param t_
      Source pointer.
   */
   template <typename U>
   shared_ptr(shared_ptr<U> const & src, T * t_) :
      sr(src.sr),
      t(t_) {
      if (sr) {
         sr->add_strong_ref();
      }
   }

   /*! Constructor that creates a strong pointer from a weak one.

   @param ptr
      Source object.
   */
   template <typename U>
   explicit shared_ptr(weak_ptr<U> const & ptr);

   /*! Constructor that transfers ownership from a unique_ptr.

   @param ptr
      Pointer to transfer ownership from.
   */
   template <typename U, typename TDel>
   shared_ptr(unique_ptr<U, TDel> && ptr) :
      sr(new _pvt::shared_refcount_with_deleter<U, TDel>(ptr.get(), ptr.get_deleter())),
      t(ptr.release()) {
   }

   //! Constructor from a null pointer.
   /*constexpr*/ shared_ptr(nullptr_t) :
      sr(nullptr),
      t(nullptr) {
   }

   /*! Constructor that assigns a deleter but not a pointer.

   @param del
      Deleter to store.
   */
   template <typename TDel>
   shared_ptr(nullptr_t, TDel del) :
      sr(new _pvt::shared_refcount_with_deleter<nullptr_t, TDel>(nullptr, del)),
      t(nullptr) {
   }

   /*! Constructor that assigns a deleter and an allocator, but not a pointer.

   @param del
      Deleter to store.
   @param alloc
      Allocator to store.
   */
   template <typename TDel, class TAllocator>
   shared_ptr(nullptr_t, TDel del, TAllocator alloc); // TODO

   //! Destructor.
   ~shared_ptr() {
      if (sr) {
         sr->release_strong();
      }
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   shared_ptr & operator=(shared_ptr && src) {
      if (&src != this) {
         if (sr) {
            sr->release_strong();
         }
         sr = src.sr;
         t = src.t;
         src.sr = nullptr;
         src.t = nullptr;
      }
      return *this;
   }

   /*! Move-assignment operator with implicit pointer conversion.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   shared_ptr & operator=(shared_ptr<U> && src) {
      return operator=(shared_ptr(move(src)));
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   shared_ptr & operator=(shared_ptr const & src) {
      if (&src != this) {
         auto new_sr = src.sr;
         if (new_sr) {
            new_sr->add_strong_ref();
         }
         if (sr) {
            sr->release_strong();
         }
         sr = new_sr;
         t = src.t;
      }
      return *this;
   }

   /*! Copy-assignment operator with implicit pointer conversion.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   shared_ptr & operator=(shared_ptr<U> const & src) {
      return operator=(shared_ptr(src));
   }

   /*! Move-assignment operator that transfers ownership from a unique_ptr with implicit pointer conversion.

   @param ptr
      Source object.
   @return
      *this.
   */
   template <typename U, typename TDel>
   shared_ptr & operator=(unique_ptr<U, TDel> && ptr) {
      return operator=(shared_ptr(move(ptr)));
   }

   /*! Dereference operator.

   @return
      Reference to the owned object.
   */
   typename add_lvalue_reference<T>::type operator*() const {
      return *t;
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the owned object.
   */
   T * operator->() const {
      return t;
   }

   /*! Boolean evaluation operator.

   @return
      true if the object points to a valid object, or false if it points to nullptr.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return t != nullptr;
   }

   //! Releases the pointed-to object, optionally assigning a new pointer.
   void reset() {
      if (sr) {
         sr->release_strong();
         sr = nullptr;
         t = nullptr;
      }
   }
   template <typename U>
   void reset(U * u) {
      operator=(shared_ptr(u));
   }
   template <typename U, typename TDel>
   void reset(U * u, TDel del) {
      operator=(shared_ptr(u, del));
   }
   template <typename U, typename TDel, class TAllocator>
   void reset(U * u, TDel del, TAllocator alloc) {
      operator=(shared_ptr(u, del, alloc));
   }

   /*! Returns the wrapped pointer.

   @return
      Pointer to the owned object.
   */
   T * get() const {
      return t;
   }

   /*! Returns true if no other pointers are referring to the object pointed to.

   @return
      true if *this is the only pointer to the owned object, or false otherwise.
   */
   bool unique() const {
      return use_count() == 1;
   }

   /*! Returns the number of references to the object pointed to.

   @return
      Reference count.
   */
   long use_count() const {
      return sr ? sr->use_count() : 0;
   }

private:
   /*! Constructor. Non-standard.

   @param sr_
      Pointer to the reference-counting object to use.
   @param t_
      Pointer to adopt.
   */
   shared_ptr(_pvt::shared_refcount * sr_, T * t_) :
      sr(sr_),
      t(t_) {
      // No need to call sr->add_strong_ref().
   }

private:
   //! Shared reference count. We hold a strong reference to it.
   _pvt::shared_refcount * sr;
   //! Owned object.
   T * t;
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Non-owning pointer that providess access to shared_ptr (C++11 § 20.7.2.3 “Class template weak_ptr”).
template <typename T>
class weak_ptr {
private:
   template <typename T2>
   friend class shared_ptr;

public:
   //! Type of the element pointed to.
   typedef T element_type;

public:
   //! Default constructor.
   weak_ptr() :
      sr(nullptr),
      t(nullptr) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   weak_ptr(weak_ptr && src) :
      sr(src.sr),
      t(src.t) {
      src.sr = nullptr;
      src.t = nullptr;
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   weak_ptr(weak_ptr const & src) :
      sr(src.sr),
      t(src.t) {
      if (sr) {
         sr->add_weak_ref();
      }
   }

   /*! Copy constructor with implicit pointer conversion.

   @param src
      Source object.
   */
   template <typename U>
   weak_ptr(weak_ptr<U> const & src) :
      sr(src.sr),
      t(src.t) {
      if (sr) {
         sr->add_weak_ref();
      }
   }

   /*! Constructor from a shared_ptr with implicit pointer conversion.

   @param ptr
      Source object.
   */
   template <typename U>
   weak_ptr(shared_ptr<U> const & ptr) :
      sr(ptr.sr),
      t(ptr.get()) {
      if (sr) {
         sr->add_weak_ref();
      }
   }

   //! Destructor.
   ~weak_ptr() {
      if (sr) {
         sr->release_weak();
      }
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   weak_ptr & operator=(weak_ptr && src) {
      if (&src != this) {
         if (sr) {
            sr->release_weak();
         }
         sr = src.sr;
         t = src.t;
         src.sr = nullptr;
         src.t = nullptr;
      }
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   weak_ptr & operator=(weak_ptr const & src) {
      if (&src != this) {
         T * new_sr = src.sr;
         if (new_sr) {
            new_sr->add_weak_ref();
         }
         if (sr) {
            sr->release_weak();
         }
         sr = new_sr;
         t = src.t;
      }
      return *this;
   }

   /*! Copy-assignment operator with implicit pointer conversion.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   weak_ptr & operator=(weak_ptr<U> const & src) {
      return operator=(weak_ptr(src));
   }

   /*! Copy-assignment operator from a shared_ptr with implicit pointer conversion.

   @param ptr
      Source object.
   @return
      *this.
   */
   template <typename U>
   weak_ptr & operator=(shared_ptr<U> const & ptr) {
      return operator=(weak_ptr(ptr));
   }

   /*! Returns true if the object pointed to no longer exists.

   @return
      true if the object pointed to has been deleted, or false otherwise.
   */
   bool expired() const {
      return use_count() == 0;
   }

   /*! Returns a non-weak pointer to the object pointed to.

   @return
      Shared pointer to the watched object.
   */
   shared_ptr<T> lock() const {
      return expired() ? shared_ptr<T>() : shared_ptr<T>(*this);
   }

   //! Releases the object pointed to.
   void reset() {
      if (sr) {
         sr->release_weak();
         sr = nullptr;
         t = nullptr;
      }
   }

   /*! Returns the number of strong references to the object pointed to.

   @return
      Count of strong references to the watched object.
   */
   long use_count() const {
      return sr ? sr->use_count() : 0;
   }

private:
   //! Shared reference count. We hold a weak reference to it.
   _pvt::shared_refcount * sr;
   /*! Weakly-owned object. Not to be used directly; we only keep it to pass it when constructing a
   shared_ptr. */
   T * t;
};


// Now this can be defined.

template <typename T>
template <typename U>
inline shared_ptr<T>::shared_ptr(weak_ptr<U> const & ptr) :
   sr(ptr.sr),
   t(ptr.t) {
   if (sr) {
      sr->add_strong_ref();
   }
}

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Base class for objects that need to repeatedly create shared_ptr instances pointing to themselves (C++11
§ 20.7.2.4 “Class template enable_shared_from_this”). */
// TODO: incomplete and not working; needs shared_ptr::shared_ptr() support to initialize sr.
template <typename T>
class enable_shared_from_this {
public:
   /*! Returns a shared pointer to this.

   @return
      Pointer to *this.
   */
   shared_ptr<T> shared_from_this() {
      shared_ptr<T> this_ptr(sr, static_cast<T *>(this));
      sr->add_strong_ref();
      return move(this_ptr);
   }

   /*! Returns a shared const pointer to this.

   @return
      Const pointer to *this.
   */
   shared_ptr<T const> shared_from_this() const {
      shared_ptr<T const> this_ptr(sr, static_cast<T const *>(this));
      sr->add_strong_ref();
      return move(this_ptr);
   }

protected:
   //! Default constructor.
   enable_shared_from_this() :
      sr(nullptr) {
   }

   //! Copy constructor.
   enable_shared_from_this(enable_shared_from_this const &) :
      sr(nullptr) {
   }

   //! Destructor.
   ~enable_shared_from_this() {
      if (sr) {
         sr->release_weak();
      }
   }

   /*! Copy-assignment operator.

   @return
      *this.
   */
   enable_shared_from_this & operator=(enable_shared_from_this const &) {
      return *this;
   }

private:
   //! Shared reference count. We hold a weak reference to it.
   _pvt::shared_refcount * sr;
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

/*! Helper for make_shared(); handles everything except calling T::T().

@return
   Pointer referencing a prefix_shared_refcount followed by storage for a T instance.
*/
template <typename T>
inline tuple<unique_ptr<void>, T *> make_unconstructed_shared() {
   typedef _pvt::prefix_shared_refcount<T> prefix_shared_refcount;
   /* Allocate a block of memory large enough to contain a refcount object and a T instance, making sure the T
   has proper alignment. */
   unique_ptr<max_align_t> p(new max_align_t[
      LOFTY_ALIGNED_SIZE(sizeof(prefix_shared_refcount)) + LOFTY_ALIGNED_SIZE(sizeof(T))
   ]);
   /* Construct the shared_refcount. Its destructor won’t be called if an exception is thrown before p gets
   stored in a shared_ptr. */
   ::new(p.get()) prefix_shared_refcount();
   T * t = reinterpret_cast<T *>(p.get() + LOFTY_ALIGNED_SIZE(sizeof(prefix_shared_refcount)));
   return make_tuple(move(p), t);
}

}}} //namespace lofty::_std::_pvt

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Returns the deleter in use by the specified shared_ptr, if any (C++11 § 20.7.2.2.10 “get_deleter”).

@param ptr
   Pointer from which to extract a deleter.
@return
   Pointer to ptr’s deleter.
*/
template <typename TDel, typename T>
inline TDel * get_deleter(shared_ptr<T> const & ptr) {
   _pvt::shared_refcount * psr = ptr.sr;
   if (psr) {
      return static_cast<TDel *>(psr->get_deleter(typeid(TDel)));
   } else {
      return nullptr;
   }
}

/*! Dynamically allocates an object, using the same memory block to store any dynamic memory used by the
returned shared pointer to it.

@param args
   Arguments to T’s constructor.
@return
   Shared pointer to the newly-constructed T instance.
*/
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename T, typename... TArgs>
inline shared_ptr<T> make_shared(TArgs &&... args) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(forward<TArgs>(args)...);
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename T>
inline shared_ptr<T> make_shared() {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T();
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 1-argument T::T().
template <typename T, typename TArg0>
inline shared_ptr<T> make_shared(TArg0 && arg0) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(forward<TArg0>(arg0));
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 2-argument T::T().
template <typename T, typename TArg0, typename TArg1>
inline shared_ptr<T> make_shared(TArg0 && arg0, TArg1 && arg1) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(forward<TArg0>(arg0), forward<TArg1>(arg1));
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 3-argument T::T().
template <typename T, typename TArg0, typename TArg1, typename TArg2>
inline shared_ptr<T> make_shared(TArg0 && arg0, TArg1 && arg1, TArg2 && arg2) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2));
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 4-argument T::T().
template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3>
inline shared_ptr<T> make_shared(TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 5-argument T::T().
template <typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4>
inline shared_ptr<T> make_shared(TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 6-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5
>
inline shared_ptr<T> make_shared(
   TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5
) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4), forward<TArg5>(arg5)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 7-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5,
   typename TArg6
>
inline shared_ptr<T> make_shared(
   TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6
) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4), forward<TArg5>(arg5), forward<TArg6>(arg6)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 8-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5,
   typename TArg6, typename TArg7
>
inline shared_ptr<T> make_shared(
   TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
   TArg7 && arg7
) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4), forward<TArg5>(arg5), forward<TArg6>(arg6), forward<TArg7>(arg7)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 9-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5,
   typename TArg6, typename TArg7, typename TArg8
>
inline shared_ptr<T> make_shared(
   TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
   TArg7 && arg7, TArg8 && arg8
) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4), forward<TArg5>(arg5), forward<TArg6>(arg6), forward<TArg7>(arg7),
      forward<TArg8>(arg8)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}
// Overload for 10-argument T::T().
template <
   typename T, typename TArg0, typename TArg1, typename TArg2, typename TArg3, typename TArg4, typename TArg5,
   typename TArg6, typename TArg7, typename TArg8, typename TArg9
>
inline shared_ptr<T> make_shared(
   TArg0 && arg0, TArg1 && arg1, TArg2 && arg2, TArg3 && arg3, TArg4 && arg4, TArg5 && arg5, TArg6 && arg6,
   TArg7 && arg7, TArg8 && arg8, TArg9 && arg9
) {
   auto tpl(_pvt::make_unconstructed_shared<T>());
   ::new(get<1>(tpl)) T(
      forward<TArg0>(arg0), forward<TArg1>(arg1), forward<TArg2>(arg2), forward<TArg3>(arg3),
      forward<TArg4>(arg4), forward<TArg5>(arg5), forward<TArg6>(arg6), forward<TArg7>(arg7),
      forward<TArg8>(arg8), forward<TArg9>(arg9)
   );
   return shared_ptr<T>(static_cast<_pvt::shared_refcount *>(
      static_cast<_pvt::prefix_shared_refcount<T> *>(get<0>(tpl).release())
   ), get<1>(tpl));
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/* Perform a const_cast<>() on a shared_ptr instance (C++11 § 20.7.2.2.9 “shared_ptr casts”).

@param src
   Source pointer.
@return
   Resulting pointer.
*/
template <typename T, typename U>
inline shared_ptr<T> const_pointer_cast(shared_ptr<U> const & src) {
   return shared_ptr<T>(src, const_cast<T *>(src.get()));
}

/* Perform a dynamic_cast<>() on a shared_ptr instance (C++11 § 20.7.2.2.9 “shared_ptr casts”).

@param src
   Source pointer.
@return
   Resulting pointer.
*/
template <typename T, typename U>
inline shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const & src) {
   return shared_ptr<T>(src, dynamic_cast<T *>(src.get()));
}

/* Perform a static_cast<>() on a shared_ptr instance (C++11 § 20.7.2.2.9 “shared_ptr casts”).

@param src
   Source pointer.
@return
   Resulting pointer.
*/
template <typename T, typename U>
inline shared_ptr<T> static_pointer_cast(shared_ptr<U> const & src) {
   return shared_ptr<T>(src, static_cast<T *>(src.get()));
}

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600
   #include <memory>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::allocator;
   using ::std::bad_weak_ptr;
   using ::std::const_pointer_cast;
   using ::std::default_delete;
   using ::std::dynamic_pointer_cast;
   using ::std::enable_shared_from_this;
   using ::std::get_deleter;
   using ::std::make_shared;
   using ::std::shared_ptr;
   using ::std::static_pointer_cast;
   using ::std::unique_ptr;
   using ::std::weak_ptr;

   #ifdef _LOFTY_STD_TYPE_TRAITS_IS_COPY_CONSTRUCTIBLE
      /* Partially-specialize is_copy_constructible (when defined by Lofty) for stock STL types (when not
      defined by Lofty). */
      template <typename T, typename TDeleter>
      struct is_copy_constructible<unique_ptr<T, TDeleter>> : public false_type {};
   #endif

   }}}
#endif //if LOFTY_HOST_STL_LOFTY || LOFTY_HOST_STL_MSVCRT == 1600 … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_MEMORY_HXX_NOPUB

#ifdef _LOFTY_STD_MEMORY_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::allocator;
   using _pub::bad_weak_ptr;
   using _pub::const_pointer_cast;
   using _pub::default_delete;
   using _pub::dynamic_pointer_cast;
   using _pub::enable_shared_from_this;
   using _pub::get_deleter;
   using _pub::make_shared;
   using _pub::shared_ptr;
   using _pub::static_pointer_cast;
   using _pub::unique_ptr;
   using _pub::weak_ptr;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_MEMORY_HXX

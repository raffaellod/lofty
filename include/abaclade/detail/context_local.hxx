/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

// Forward declarations.
class context_local_storage_node_impl;

//! Type containing data members for context_local_storage_registrar_impl.
struct context_local_storage_registrar_impl_extra_members {
   //! Count of variables registered with calls to add_var().
   unsigned m_cVars;
   //! Cumulative storage size registered with calls to add_var().
   std::size_t m_cb;
   /*! Tracks the value of cb when context_local_storage_impl was instantiated. Changes occurring
   after that first time are a problem. */
   std::size_t m_cbFrozen;
};

/*! Implementation of a variable registrar for abc::detail::thread_local_storage and
abc::detail::coroutine_local_storage. */
class ABACLADE_SYM context_local_storage_registrar_impl :
   public collections::static_list_impl_base,
   public context_local_storage_registrar_impl_extra_members {
private:
   friend class context_local_storage_impl;

public:
   //! Data members to be declared as a static member of the most-derived class.
   struct data_members {
      //! Basic list members.
      collections::static_list_impl_base slib;
      //! Additional members for this class.
      context_local_storage_registrar_impl_extra_members clsridm;
   };

public:
   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified context_local_var_impl instance; it also initializes the members of the latter. This
   function will be called during initialization of a new dynamic library as it’s being loaded, not
   during normal run-time.

   @param pclsni
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   void add_var(context_local_storage_node_impl * pclsni, std::size_t cb);
};

/*! Initial value for the data members of an
abc::detail::context_local_storage_registrar_impl::data_members variable. */
#define ABC_DETAIL_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER \
   { \
      ABC_COLLECTIONS_STATIC_LIST_IMPL_BASE_INITIALIZER, \
      { 0, 0, 0 } \
   }

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Common implementation for abc::detail::thread_local_storage and
abc::detail::coroutine_local_storage.

TODO: this will need changes to support dynamic loading and unloading of libraries that depend on
Abaclade:

The m_pb byte array should be replaced with a map from library address/name to library-specific TLS/
CRLS, and each library would have its own byte array (keyed in the same way).
Loading a new library would add a new element in the maps (and in the TLS/CRLS block for each
existing thread/coroutine), and unloading it would remove the library from all maps (and in the TLS/
CRLS block for each thread/coroutine). */
class ABACLADE_SYM context_local_storage_impl {
public:
   /*! Runs a single destruction iteration over the stored variables, using the specified registrar.

   @param clsri
      Registrar to provide the list of variables to destruct.
   @return
      true if any variables were destructed or no constructed ones were found, or false otherwise.
   */
   bool destruct_vars(context_local_storage_registrar_impl const & clsri);

   /*! Returns a pointer to the specified variable in the context-local data store.

   @param clsni
      Variable to retrieve.
   @return
      Corresponding pointer.
   */
   void * get_storage(context_local_storage_node_impl const & clsni);

protected:
   /*! Constructor.

   @param pclsri
      Pointer to the variable registrar.
   */
   context_local_storage_impl(context_local_storage_registrar_impl * pclsri);

   //! Destructor.
   ~context_local_storage_impl();

private:
   //! Array of flags indicating whether each storage slot has been constructed.
   _std::unique_ptr<bool[]> m_pbConstructed;
   //! Raw byte storage.
   _std::unique_ptr<std::int8_t[]> m_pb;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Non-template implementation of abc::detail::context_local_storage_node.
class context_local_storage_node_impl :
   public collections::static_list_impl_base::node,
   public noncopyable {
public:
   /*! Constructs the thread-local value for a new thread. Invoked at most once for each thread.

   @param p
      Pointer to the memory block where the new value should be constructed.
   */
   void (* construct)(void * p);

   /*! Destructs the thread-local value for a terminating thread. Invoked at most once for each
   thread.

   @param p
      Pointer to the value to be destructed.
   */
   void (* destruct)(void * p);

public:
   //! Offset of this variable in the TLS/CRLS block.
   std::size_t m_ibStorageOffset;
   //! Index of this variable in the TLS/CRLS block.
   unsigned m_iStorageIndex;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Implementation of a context_local_storage_registry node, as well as base class for
context_local_var_impl. */
template <typename TStorage>
class context_local_storage_node :
   public context_local_storage_node_impl,
   public collections::static_list_impl<
      typename TStorage::registrar, context_local_storage_node<TStorage>
   >::node {
protected:
   /*! Constructor.

   @param cbObject
      Size of the object pointed to by the context_local_value/context_local_ptr subclass.
   */
   explicit context_local_storage_node(std::size_t cbObject) {
      // Initializes the members of *this.
      TStorage::registrar::instance().add_var(this, cbObject);
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Common implementation of abc::detail::context_local_value and abc::detail::context_local_ptr.
template <typename T, typename TStorage>
class context_local_var_impl : public context_local_storage_node<TStorage> {
public:
   /*! Implicit cast to T &.

   @return
      Reference to the object’s value.
   */
   operator T &() {
      return *get_ptr();
   }

   /*! Implicit cast to T const &.

   @return
      Const reference to the object’s value.
   */
   operator T const &() const {
      return *get_ptr();
   }

   /*! Explicit cast to T &.

   @return
      Reference to the object’s value.
   */
   T & get() {
      return *get_ptr();
   }

   /*! Explicit cast to T const &.

   @return
      Const reference to the object’s value.
   */
   T const & get() const {
      return *get_ptr();
   }

protected:
   //! Default constructor.
   context_local_var_impl() :
      context_local_storage_node<TStorage>(sizeof(T)) {
   }

   /*! Returns a pointer to the current thread’s copy of the variable.

   @return
      Pointer to the thread-local value for this object.
   */
   T * get_ptr() const {
      return static_cast<T *>(TStorage::instance().get_storage(*this));
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of abc::thread_local_value and abc::coroutine_local_value.
template <typename T, typename TStorage, bool t_bTrivial = _std::is_trivial<T>::value>
class context_local_value;

// Partial specialization for trivial types.
template <typename T, typename TStorage>
class context_local_value<T, TStorage, true> :
   public context_local_var_impl<T, TStorage>,
   public support_explicit_operator_bool<context_local_value<T, TStorage, true>> {
public:
   //! Default constructor.
   context_local_value() {
      this->construct = nullptr;
      this->destruct  = nullptr;
   }

   /*! Move-assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T && t) {
      *this->get_ptr() = _std::move(t);
      return *this;
   }

   /*! Copy-assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T const & t) {
      *this->get_ptr() = t;
      return *this;
   }

   /*! Boolean evaluation operator.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *this->get_ptr() ? true : false;
   }
};

// Partial specialization for non-trivial types.
template <typename T, typename TStorage>
class context_local_value<T, TStorage, false> :
   public context_local_var_impl<T, TStorage>,
   public support_explicit_operator_bool<context_local_value<T, TStorage>> {
public:
   //! Default constructor.
   context_local_value() {
      this->construct = &construct_impl;
      this->destruct  = &destruct_impl;
   }

   /*! Move-assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T && t) {
      *this->get_ptr() = _std::move(t);
      return *this;
   }

   /*! Copy-assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T const & t) {
      *this->get_ptr() = t;
      return *this;
   }

   /*! Boolean evaluation operator.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *this->get_ptr() ? true : false;
   }

private:
   //! Implementation of context_local_var_impl::construct().
   static void construct_impl(void * p) {
      new(p) T();
   }

   //! Implementation of context_local_var_impl::destruct().
   static void destruct_impl(void * p) {
      static_cast<T *>(p)->~T();
#if ABC_HOST_CXX_MSC
   // MSC18 BUG: it somehow thinks that p is not referenced, so it warns about it.
   #pragma warning(suppress: 4100)
#endif
   }
};

// Specialization for bool: doesn’t need explicit operator bool() since it has it non-explicit.
template <typename TStorage>
class context_local_value<bool, TStorage, true> : public context_local_var_impl<bool, TStorage> {
public:
   //! Default constructor.
   context_local_value() {
      this->construct = nullptr;
      this->destruct  = nullptr;
   }

   /*! Assignment operator.

   @param b
      Source value.
   @return
      *this.
   */
   context_local_value & operator=(bool b) {
      *this->get_ptr() = b;
      return *this;
   }
};

// Specialization for std::shared_ptr, which offers a few additional methods.
template <typename T, typename TStorage>
class context_local_value<_std::shared_ptr<T>, TStorage, false> :
   public context_local_var_impl<_std::shared_ptr<T>, TStorage>,
   public support_explicit_operator_bool<
      context_local_value<_std::shared_ptr<T>, TStorage, false>
   > {
public:
   //! Default constructor.
   context_local_value() {
      this->construct = &construct_impl;
      this->destruct  = &destruct_impl;
   }

   /*! Move-assignment operator.

   @param pt
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(_std::shared_ptr<T> && pt) {
      *this->get_ptr() = _std::move(pt);
      return *this;
   }

   /*! Copy-assignment operator.

   @param pt
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(_std::shared_ptr<T> const & pt) {
      *this->get_ptr() = pt;
      return *this;
   }

   /*! Implicit cast to _std::shared_ptr<T> &.

   @return
      Reference to the shared pointer.
   */
   operator _std::shared_ptr<T> &() {
      return *this->get_ptr();
   }

   /*! Implicit cast to _std::shared_ptr<T> const &.

   @return
      Const reference to the shared pointer.
   */
   operator _std::shared_ptr<T> const &() const {
      return *this->get_ptr();
   }

   /*! Boolean evaluation operator.

   @return
      true if the pointer is not nullptr, or false if it is.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *this->get_ptr() ? true : false;
   }

   /*! Explicit cast to T *.

   @return
      Pointer to the current T instance.
   */
   T * get() {
      return this->get_ptr()->get();
   }

   /*! Explicit cast to T const *.

   @return
      Const pointer to the current T instance.
   */
   T const * get() const {
      return this->get_ptr()->get();
   }

   //! Releases the pointed-to object.
   void reset() {
      this->get_ptr()->reset();
   }

   /*! Returns true if no other pointers are referring to the object pointed to.

   @return
      true if *this is the only pointer to the owned object, or false otherwise.
   */
   bool unique() const {
      return this->get_ptr()->unique();
   }

   /*! Returns the number of references to the object pointed to.

   @return
      Reference count.
   */
   long use_count() const {
      return this->get_ptr()->use_count();
   }

private:
   //! Implementation of context_local_var_impl::construct().
   static void construct_impl(void * p) {
      new(p) _std::shared_ptr<T>();
   }

   //! Implementation of context_local_var_impl::destruct().
   static void destruct_impl(void * p) {
      static_cast<_std::shared_ptr<T> *>(p)->~shared_ptr();
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Contains a T and a bool to track whether the T has been constructed.
template <typename T>
struct context_local_ptr_value {
   //! Contained value.
   T t;
   //! true if t has been constructed, or false otherwise.
   bool bConstructed;
};

//! Implementation of abc::thread_local_ptr and abc::coroutine_local_ptr.
template <typename T, typename TStorage>
class context_local_ptr :
   public context_local_var_impl<context_local_ptr_value<T>, TStorage>,
   public support_explicit_operator_bool<context_local_ptr<T, TStorage>> {
private:
public:
   //! Default constructor.
   context_local_ptr() {
      // No constructor: we’d only set bConstructed to false, which is already its value (0).
      this->construct = nullptr;
      this->destruct  = &destruct_impl;
   }

   /*! Dereference operator.

   @return
      Reference to the owned object.
   */
   T & operator*() const {
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
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return this->get_ptr()->bConstructed;
   }

   /*! Returns the address of the thread-local value this object points to.

   @return
      Internal pointer.
   */
   T * get() const {
      auto pvalue = this->get_ptr();
      return pvalue->bConstructed ? &pvalue->t : nullptr;
   }

   //! Deletes the object currently pointed to, if any, resetting the pointer to nullptr.
   void reset() {
      auto pvalue = this->get_ptr();
      if (pvalue->bConstructed) {
         pvalue->t.~T();
         pvalue->bConstructed = false;
      }
   }

   /*! Destructs the object currently pointed to, if any, and constructs a new object.

   @param tSrc
      Source object to move-construct the new object from.
   @return
      Pointer to the new object.
   */
   T * reset_new(T tSrc = T()) {
      reset();
      auto pvalue = this->get_ptr();
      // The constructor invoked is T::T(T &&), which should not throw.
      new(&pvalue->t) T(_std::move(tSrc));
      pvalue->bConstructed = true;
      return &pvalue->t;
   }

private:
   //! Implementation of context_local_var_impl::destruct().
   static void destruct_impl(void * p) {
      auto pvalue = static_cast<context_local_ptr_value<T> *>(p);
      if (pvalue->bConstructed) {
         pvalue->t.~T();
      }
   }
};

}} //namespace abc::detail

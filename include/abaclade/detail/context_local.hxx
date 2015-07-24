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
class context_local_var_impl_base;

//! Type containing data members for this class.
struct context_local_storage_registrar_impl_data_members {
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
   public context_local_storage_registrar_impl_data_members {
private:
   friend class context_local_storage_impl;

public:
   typedef context_local_storage_registrar_impl_data_members data_members;

   struct all_data_members {
      collections::static_list_data_members sldm;
      data_members clsridm;
   };

public:
   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified context_local_var_impl instance; it also initializes the members of the latter. This
   function will be called during initialization of a new dynamic library as it’s being loaded, not
   during normal run-time.

   @param pclvib
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   void add_var(context_local_var_impl_base * pclvib, std::size_t cb);
};

/*! Initial value for the data members of an
abc::detail::context_local_storage_registrar_impl::all_data_members variable. */
#define ABC_DETAIL_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER \
   { \
      ABC_COLLECTIONS_STATIC_LIST_INITIALIZER, \
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
   /*! Returns a pointer to the specified variable in the context-local data store.

   @param pclvib
      Pointer to the variable to retrieve.
   @return
      Corresponding pointer.
   */
   void * get_storage(context_local_var_impl_base const * pclvib);

protected:
   /*! Constructor.

   @param pclsri
      Pointer to the variable registrar.
   */
   context_local_storage_impl(context_local_storage_registrar_impl * pclsri);

   //! Destructor.
   ~context_local_storage_impl();

   /*! Checks whether a variable has been constructed in *this.

   @param iVar
      Index of the variable.
   @return
      true is the variable has been constructed in *this, or false otherwise.
   */
   bool is_var_constructed(unsigned iVar) const {
      return m_pbConstructed[iVar];
   }

   /*! Marks a variable as no longer constructed.

   @param iVar
      Index of the variable.
   */
   void var_destructed(unsigned iVar) {
      m_pbConstructed[iVar] = false;
   }

private:
   //! Array of flags indicating whether each storage slot has been constructed.
   std::unique_ptr<bool[]> m_pbConstructed;
   //! Raw byte storage.
   std::unique_ptr<std::int8_t[]> m_pb;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Non-template implementation of abc::detail::context_local_var_impl.
class ABACLADE_SYM context_local_var_impl_base :
   public collections::static_list_node_base,
   public noncopyable {
protected:
   //! Constructor.
   context_local_var_impl_base();

   //! Destructor.
   ~context_local_var_impl_base();

public:
   /*! Constructs the thread-local value for a new thread. Invoked at most once for each thread.

   @param p
      Pointer to the memory block where the new value should be constructed.
   */
   virtual void construct(void * p) const = 0;

   /*! Destructs the thread-local value for a terminating thread. Invoked at most once for each
   thread.

   @param p
      Pointer to the value to be destructed.
   */
   virtual void destruct(void * p) const = 0;

public:
   //! Offset of this variable in the TLS/CRLS block.
   std::size_t m_ibStorageOffset;
   //! Index of this variable in the TLS/CRLS block.
   unsigned m_iStorageIndex;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Common implementation of abc::detail::context_local_value and abc::detail::context_local_ptr.
template <typename TStorage>
class context_local_var_impl :
   public context_local_var_impl_base,
   public collections::static_list_node<
      typename TStorage::registrar, context_local_var_impl<TStorage>
   > {
protected:
   /*! Constructor.

   @param cbObject
      Size of the object pointed to by the context_local_value/context_local_ptr subclass.
   */
   explicit context_local_var_impl(std::size_t cbObject) {
      // Initializes the members of *this.
      TStorage::registrar::instance().add_var(this, cbObject);
   }

   /*! Returns a pointer to the current thread’s copy of the variable.

   @return
      Pointer to the thread-local value for this object.
   */
   template <typename T>
   T * get_ptr() const {
      return static_cast<T *>(TStorage::instance().get_storage(this));
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of abc::thread_local_value and abc::coroutine_local_value.
template <typename T, typename TStorage>
class context_local_value :
   private context_local_var_impl<TStorage>,
   public support_explicit_operator_bool<context_local_value<T, TStorage>> {
public:
   //! Constructor.
   context_local_value() :
      context_local_var_impl<TStorage>(sizeof(T)) {
   }

   /*! Assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T const & t) {
      *get_ptr() = t;
      return *this;
   }
   context_local_value & operator=(T && t) {
      *get_ptr() = std::move(t);
      return *this;
   }

   /*! Implicit cast to T &.

   @return
      Reference to the object’s value.
   */
   operator T &() {
      return *get_ptr();
   }
   operator T const &() const {
      return *get_ptr();
   }

   /*! Returns true if the object’s value evaluates to true.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *get_ptr() ? true : false;
   }

   /*! Explicit cast to T &.

   @return
      Reference to the object’s value.
   */
   T & get() {
      return *get_ptr();
   }
   T const & get() const {
      return *get_ptr();
   }

private:
   //! See context_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) T();
   }

   //! See context_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<T *>(p)->~T();
   }

   //! See context_local_var_impl::get_ptr().
   T * get_ptr() const {
      return context_local_var_impl<TStorage>::template get_ptr<T>();
   }
};

// Specialization for bool, which does not need operator bool().
template <typename TStorage>
class context_local_value<bool, TStorage> : private context_local_var_impl<TStorage> {
public:
   //! Constructor.
   context_local_value() :
      context_local_var_impl<TStorage>(sizeof(bool)) {
   }

   /*! Assignment operator.

   @param b
      Source value.
   @return
      *this.
   */
   context_local_value & operator=(bool b) {
      *get_ptr() = b;
      return *this;
   }

   /*! Implicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   operator bool &() {
      return *get_ptr();
   }
   operator bool const &() const {
      return *get_ptr();
   }

   /*! Explicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   bool & get() {
      return *get_ptr();
   }
   bool const & get() const {
      return *get_ptr();
   }

private:
   //! See context_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) bool();
   }

   //! See context_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      ABC_UNUSED_ARG(p);
   }

   //! See context_local_var_impl::get_ptr().
   bool * get_ptr() const {
      return context_local_var_impl<TStorage>::template get_ptr<bool>();
   }
};

// Specialization for std::shared_ptr, which offers a few additional methods.
template <typename T, typename TStorage>
class context_local_value<std::shared_ptr<T>, TStorage> :
   private context_local_var_impl<TStorage>,
   public support_explicit_operator_bool<context_local_value<std::shared_ptr<T>, TStorage>> {
private:
   typedef std::shared_ptr<T> value_t;

public:
   //! Constructor.
   context_local_value() :
      context_local_var_impl<TStorage>(sizeof(std::shared_ptr<T>)) {
   }

   /*! Assignment operator.

   @param pt
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(std::shared_ptr<T> const & pt) {
      *get_ptr() = pt;
      return *this;
   }
   context_local_value & operator=(std::shared_ptr<T> && pt) {
      *get_ptr() = std::move(pt);
      return *this;
   }

   /*! Implicit cast to std::shared_ptr<T> &.

   @return
      Reference to the shared pointer.
   */
   operator std::shared_ptr<T> &() {
      return *get_ptr();
   }
   operator std::shared_ptr<T> const &() const {
      return *get_ptr();
   }

   /*! Returns true if the pointer is not nullptr.

   @return
      false if the pointer is nullptr, or true otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *get_ptr() ? true : false;
   }

   /*! Explicit cast to T *.

   @return
      Pointer to the current T instance.
   */
   T * get() {
      return get_ptr()->get();
   }
   T const * get() const {
      return get_ptr()->get();
   }

   //! Releases the pointed-to object.
   void reset() {
      get_ptr()->reset();
   }

   /*! Returns true if no other pointers are referring to the object pointed to.

   @return
      true if *this is the only pointer to the owned object, or false otherwise.
   */
   bool unique() const {
      return get_ptr()->unique();
   }

   /*! Returns the number of references to the object pointed to.

   @return
      Reference count.
   */
   long use_count() const {
      return get_ptr()->use_count();
   }

private:
   //! See context_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) std::shared_ptr<T>();
   }

   //! See context_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<std::shared_ptr<T> *>(p)->~shared_ptr();
   }

   //! See context_local_var_impl::get_ptr().
   std::shared_ptr<T> * get_ptr() const {
      return context_local_var_impl<TStorage>::template get_ptr<std::shared_ptr<T>>();
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Implementation of abc::thread_local_ptr and abc::coroutine_local_ptr.
template <typename T, typename TStorage>
class context_local_ptr :
   private context_local_var_impl<TStorage>,
   public support_explicit_operator_bool<context_local_ptr<T, TStorage>> {
private:
   //! Contains a T and a bool to track whether the T has been constructed.
   struct value_t {
      T t;
      bool bConstructed;
   };

public:
   //! Constructor.
   context_local_ptr() :
      context_local_var_impl<TStorage>(sizeof(value_t)) {
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
      true if get() != nullptr, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return get_ptr()->bConstructed;
   }

   /*! Returns the address of the thread-local value this object points to.

   @return
      Internal pointer.
   */
   T * get() const {
      value_t * pValue = get_ptr();
      return pValue->bConstructed ? &pValue->t : nullptr;
   }

   /*! Deletes the object currently pointed to, if any, resetting the pointer to nullptr.

   @param pt
      Pointer to a new object to take ownership of.
   */
   void reset() {
      value_t * pValue = get_ptr();
      if (pValue->bConstructed) {
         pValue->t.~T();
         pValue->bConstructed = false;
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
      value_t * pValue = get_ptr();
      // The constructor invoked is T::T(T &&), which should not throw.
      new(&pValue->t) T(std::move(tSrc));
      pValue->bConstructed = true;
      return &pValue->t;
   }

private:
   //! See context_local_var_impl::construct().
   virtual void construct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      pValue->bConstructed = false;
      // Note that pValue.t is left uninitialized.
   }

   //! See context_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      if (pValue->bConstructed) {
         pValue->t.~T();
      }
   }

   //! See context_local_var_impl::get_ptr().
   value_t * get_ptr() const {
      return context_local_var_impl<TStorage>::template get_ptr<value_t>();
   }
};

}} //namespace abc::detail

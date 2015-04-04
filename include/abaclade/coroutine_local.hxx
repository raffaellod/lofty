/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
// abc::detail::coroutine_local_storage

namespace abc {
namespace detail {

// Forward declaration.
class coroutine_local_var_impl;

//! Abaclade’s CRLS (TLS for coroutines) slot data manager.
/* TODO: this will need changes to support dynamic loading and unloading of libraries that depend on
Abaclade.

The m_pb byte array should be replaced with a map from library address/name to library-specific
CRLS, and each library would have its own byte array (keyed in the same way).
Loading a new library would add a new element in the maps (and in the CRLS block for each existing
coroutine), and unloading it would remove the library from all maps (and in the CRLS block for each
coroutine). */
class ABACLADE_SYM coroutine_local_storage :
   public collections::static_list<coroutine_local_storage, coroutine_local_var_impl>,
   public noncopyable {
private:
   friend class coroutine_scheduler_impl;

public:
   //! Constructor.
   coroutine_local_storage();

   //! Destructor.
   ~coroutine_local_storage();

   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified coroutine_local_var_impl instance; it also initializes the m_pcrlviNext and
   m_ibStorageOffset members of the latter. This function will be called during initialization of a
   new dynamic library as it’s being loaded, not during normal run-time.

   @param pcrlvi
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   static void add_var(coroutine_local_var_impl * pcrlvi, std::size_t cb);

   /*! Used by coroutine_scheduler_impl, it changes sm_pcrls to switch to a different
   coroutine_local_storage instance.

   @param pcrlsActive
      Pointer to the to-be active coroutine’s local storage, or nullptr to use sm_crls.get().
   */
   static void set_active(coroutine_local_storage * pcrlsActive);

   /*! Returns a pointer to the specified offset in the storage.

   @return
      Pointer to the data store.
   */
   static coroutine_local_storage * get();

   /*! Returns a pointer to the specified offset in the coroutine-local data store.

   @param ibOffset
      Desired offset.
   @return
      Corresponding pointer.
   */
   void * get_storage(std::size_t ibOffset) const {
      return &m_pb[ibOffset];
   }

   ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)

private:
   //! Raw byte storage.
   std::unique_ptr<std::int8_t[]> m_pb;
   //! Cumulative storage size registered with calls to add_var().
   static std::size_t sm_cb;
   /*! Per-thread storage for the active coroutine. If a coroutine_scheduler is running on a thread,
   this is replaced on each change of coroutine_scheduler::sm_pcoroctxActive. */
   static thread_local_ptr<coroutine_local_storage> sm_crls;
   /*! Normally a pointer to sm_crls (set for each thread by sm_crls’s existence, which instantiates
   a coroutine_local_storage which in turn sets this), but replaced while a coroutine is being
   actively executed. */
   static thread_local_value<coroutine_local_storage *> sm_pcrls;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::coroutine_local_var_impl

namespace abc {
namespace detail {

//! Non-template implementation of abc::coroutine_local_value and abc::coroutine_local_ptr.
class ABACLADE_SYM coroutine_local_var_impl :
   public collections::static_list<coroutine_local_storage, coroutine_local_var_impl>::node,
   public noncopyable {
private:
   friend class coroutine_local_storage;

protected:
   /*! Constructor.

   @param cbObject
      Size of the object pointed to by the coroutine_local_value/coroutine_local_ptr subclass.
   */
   coroutine_local_var_impl(std::size_t cbObject);

   /*! Constructs the coroutine-local value for a new coroutine. Invoked at most once for each
   coroutine.

   @param p
      Pointer to the memory block where the new value should be constructed.
   */
   virtual void construct(void * p) const = 0;

   /*! Destructs the coroutine-local value for a terminating coroutine. Invoked at most once for
   each coroutine.

   @param p
      Pointer to the value to be destructed.
   */
   virtual void destruct(void * p) const = 0;

   /*! Returns a pointer to the current coroutine’s copy of the variable.

   @return
      Pointer to the coroutine-local value for this object.
   */
   template <typename T>
   T * get_ptr() const {
      return static_cast<T *>(coroutine_local_storage::get()->get_storage(m_ibStorageOffset));
   }

private:
   //! Offset of this variable in the CRLS block.
   std::size_t m_ibStorageOffset;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine_local_value

namespace abc {

/*! Variable with separate per-coroutine values. Variables of this type cannot be non-static class
members. */
template <typename T>
class coroutine_local_value :
   private detail::coroutine_local_var_impl,
   public support_explicit_operator_bool<coroutine_local_value<T>> {
public:
   //! Constructor.
   coroutine_local_value() :
      detail::coroutine_local_var_impl(sizeof(T)) {
   }

   /*! Assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   coroutine_local_value & operator=(T t) {
      get() = std::move(t);
      return *this;
   }

   /*! Implicit cast to T &.

   @return
      Reference to the object’s value.
   */
   operator T &() {
      return get();
   }
   operator T const &() const {
      return get();
   }

   /*! Returns true if the object’s value evaluates to true.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return get() ? true : false;
   }

   /*! Explicit cast to T &.

   @return
      Reference to the object’s value.
   */
   T & get() {
      return *get_ptr<T>();
   }
   T const & get() const {
      return *get_ptr<T>();
   }

private:
   //! See detail::coroutine_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) T();
   }

   //! See detail::coroutine_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<T *>(p)->~T();
   }
};

// Specialization for bool, which does not need operator bool().
template <>
class coroutine_local_value<bool> : private detail::coroutine_local_var_impl {
public:
   //! Constructor.
   coroutine_local_value() :
      detail::coroutine_local_var_impl(sizeof(bool)) {
   }

   /*! Assignment operator.

   @param b
      Source value.
   @return
      *this.
   */
   coroutine_local_value & operator=(bool b) {
      get() = b;
      return *this;
   }

   /*! Implicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   operator bool &() {
      return get();
   }
   operator bool const &() const {
      return get();
   }

   /*! Explicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   bool & get() {
      return *get_ptr<bool>();
   }
   bool const & get() const {
      return *get_ptr<bool>();
   }

private:
   //! See detail::coroutine_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) bool();
   }

   //! See detail::coroutine_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      ABC_UNUSED_ARG(p);
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::coroutine_local_ptr

namespace abc {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each
coroutine, and an instance of this class lets each coroutine access its own private copy of the
value pointed to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class coroutine_local_ptr :
   private detail::coroutine_local_var_impl,
   public support_explicit_operator_bool<coroutine_local_ptr<T>> {
private:
   //! Contains a T and a bool to track whether the T has been constructed.
   struct value_t {
      T t;
      bool bConstructed;
   };

public:
   //! Constructor.
   coroutine_local_ptr() :
      detail::coroutine_local_var_impl(sizeof(value_t)) {
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
      return get_ptr<value_t>()->bConstructed;
   }

   /*! Returns the address of the coroutine-local value this object points to.

   @return
      Internal pointer.
   */
   T * get() const {
      value_t * pValue = get_ptr<value_t>();
      return pValue->bConstructed ? &pValue->t : nullptr;
   }

   /*! Deletes the object currently pointed to, if any, resetting the pointer to nullptr.

   @param pt
      Pointer to a new object to take ownership of.
   */
   void reset() {
      value_t * pValue = get_ptr<value_t>();
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
      value_t * pValue = get_ptr<value_t>();
      // The constructor invoked is T::T(T &&), which should not throw.
      new(&pValue->t) T(std::move(tSrc));
      pValue->bConstructed = true;
      return &pValue->t;
   }

private:
   //! See detail::coroutine_local_var_impl::construct().
   virtual void construct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      pValue->bConstructed = false;
      // Note that pValue.t is left uninitialized.
   }

   //! See detail::coroutine_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      if (pValue->bConstructed) {
         pValue->t.~T();
      }
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

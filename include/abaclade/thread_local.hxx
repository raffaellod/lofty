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
// abc::detail::thread_local_storage

namespace abc {
namespace detail {

// Forward declaration.
class thread_local_var_impl;

//! Abaclade’s TLS slot data manager.
/* TODO: this will need changes to support dynamic loading and unloading of libraries that depend on
Abaclade.

The m_pb byte array should be replaced with a map from library address/name to library-specific TLS,
and each library would have its own byte array (keyed in the same way).
Loading a new library would add a new element in the maps (and in the TLS block for each existing
thread), and unloading it would remove the library from all maps (and in the TLS block for each
thread). */
class ABACLADE_SYM thread_local_storage :
   public collections::static_list<thread_local_storage, thread_local_var_impl>,
   public noncopyable {
public:
   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified thread_local_var_impl instance; it also initializes the m_ptlviNext and m_ibTlsOffset
   members of the latter. This function will be called during initialization of a new dynamic
   library as it’s being loaded, not during normal run-time.

   @param ptlvi
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   static void add_var(thread_local_var_impl * ptlvi, std::size_t cb);

#if ABC_HOST_API_WIN32
   /*! Hook invoked by DllMain() in abaclade.dll.

   @param iReason
      Reason why DllMain() was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
   */
   static bool dllmain_hook(unsigned iReason);
#endif

   /*! Returns a pointer to the specified offset in the storage. On the first call from a new
   thread, this also lazily creates the thread_local_storage, unless bCreateNewIfNull is false.

   @param bCreateNewIfNull
      If the TLS slot is nullptr and bCreateNewIfNull is true, a new new TLS instance will be
      created; if bCreateNewIfNull is false, nullptr will be returned instead if the TLS slot is
      uninitialized.
   @return
      Pointer to the data store.
   */
   static thread_local_storage * get(bool bCreateNewIfNull = true);

   /*! Returns a pointer to the specified offset in the thread-local data store.

   @param ibOffset
      Desired offset.
   @return
      Corresponding pointer.
   */
   void * get_storage(std::size_t ibOffset) const {
      return &m_pb[ibOffset];
   }

private:
   //! Constructor.
   thread_local_storage();

   //! Destructor.
   ~thread_local_storage();

   //! Allocates the TLS slot for the process.
   static void alloc_slot();

#if ABC_HOST_API_POSIX
   /*! Destructs the storage instance for the current thread. Invoked by pthread_key_create() when a
   thread terminates.

   @param pThis
      Pointer to the TLS for the current thread.
   */
   static void destruct(void * pThis = get());
#endif

   //! Deallocates the TLS slot for the process.
   // TODO: call free_slot() in the POSIX Threads case using reference counting in destruct().
   static void free_slot();

public:
   ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(thread_local_storage)

private:
   //! Raw byte storage.
   std::unique_ptr<std::int8_t[]> m_pb;
   //! Cumulative storage size registered with calls to add_var().
   static std::size_t sm_cb;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_var_impl

namespace abc {
namespace detail {

//! Non-template implementation of abc::thread_local_value and abc::thread_local_ptr.
class ABACLADE_SYM thread_local_var_impl :
   public collections::static_list<thread_local_storage, thread_local_var_impl>::node,
   public noncopyable {
private:
   friend class thread_local_storage;

protected:
   /*! Constructor.

   @param cbObject
      Size of the object pointed to by the thread_local_value/thread_local_ptr subclass.
   */
   thread_local_var_impl(std::size_t cbObject);

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

   /*! Returns a pointer to the current thread’s copy of the variable.

   @return
      Pointer to the thread-local value for this object.
   */
   template <typename T>
   T * get_ptr() const {
      return static_cast<T *>(thread_local_storage::get()->get_storage(m_ibTlsOffset));
   }

private:
   //! Offset of this variable in the TLS block.
   std::size_t m_ibTlsOffset;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread_local_value

namespace abc {

/*! Variable with separate per-thread values. Variables of this type cannot be non-static class
members. */
template <typename T>
class thread_local_value : private detail::thread_local_var_impl {
public:
   /*! Constructor.

   @param tDefault
      Value that will be copied to initialize the TLS for each thread.
   */
   thread_local_value(T tDefault = T()) :
      detail::thread_local_var_impl(sizeof(T)),
      mc_tDefault(std::move(tDefault)) {
   }

   /*! Assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   thread_local_value & operator=(T t) {
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

private:
   //! See detail::thread_local_var_impl::construct().
   virtual void construct(void * p) const override {
      new(p) T(mc_tDefault);
   }

   //! See detail::thread_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<T *>(p)->~T();
   }

   /*! Returns a reference to the thread-local copy of the value.

   @return
      Reference to the value.
   */
   T & get() const {
      return *get_ptr<T>();
   }

private:
   //! Default value for each per-thread copy of the value.
   T const mc_tDefault;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread_local_ptr

namespace abc {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each
thread, and an instance of this class lets each thread access its own private copy of the value
pointed to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class thread_local_ptr :
   private detail::thread_local_var_impl,
   public support_explicit_operator_bool<thread_local_ptr<T>> {
private:
   //! Contains a T and a bool to track whether the T has been constructed.
   struct value_t {
      T t;
      bool bConstructed;
   };

public:
   //! Constructor.
   thread_local_ptr() :
      detail::thread_local_var_impl(sizeof(value_t)) {
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

   /*! Returns the address of the thread-local value this object points to.

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
   */
   void reset_new(T tSrc = T()) {
      reset();
      value_t * pValue = get_ptr<value_t>();
      // The constructor invoked is T::T(T &&), which should not throw.
      new(&pValue->t) T(std::move(tSrc));
      pValue->bConstructed = true;
   }

private:
   //! See detail::thread_local_var_impl::construct().
   virtual void construct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      pValue->bConstructed = false;
      // Note that pValue.t is left uninitialized.
   }

   //! See detail::thread_local_var_impl::destruct().
   virtual void destruct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      if (pValue->bConstructed) {
         pValue->t.~T();
      }
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

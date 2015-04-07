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
   specified thread_local_var_impl instance; it also initializes the m_ptlviNext and
   m_ibStorageOffset members of the latter. This function will be called during initialization of a
   new dynamic library as it’s being loaded, not during normal run-time.

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
      return static_cast<T *>(thread_local_storage::get()->get_storage(m_ibStorageOffset));
   }

private:
   //! Offset of this variable in the TLS block.
   std::size_t m_ibStorageOffset;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread_local_value

namespace abc {

/*! Variable with separate per-thread values. Variables of this type cannot be non-static class
members. */
template <typename T>
class thread_local_value : public detail::context_local_value<T, detail::thread_local_var_impl> {
private:
   typedef detail::context_local_value<T, detail::thread_local_var_impl> context_local;

public:
   //! See detail::context_local_value::operator=().
   thread_local_value & operator=(T const & t) {
      context_local::operator=(t);
      return *this;
   }
   thread_local_value & operator=(T && t) {
      context_local::operator=(std::move(t));
      return *this;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread_local_ptr

namespace abc {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each
thread, and an instance of this class lets each thread access its own private copy of the value
pointed to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class thread_local_ptr : public detail::context_local_ptr<T, detail::thread_local_var_impl> {
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

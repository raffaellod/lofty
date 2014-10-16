/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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
   #error Please #include <abaclade.hxx> instead of this file
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_ptr_impl

namespace abc {
namespace detail {

// Forward declaration.
class thread_local_ptr_impl;

//! Abaclade’s TLS slot data manager.
/* TODO: this will need changes to support dynamic loading and unloading of libraries that depend on
Abaclade.

The m_pb byte array should be replaced with a map from library address/name to library-specific TLS,
and each library would have its own byte array (keyed in the same way).
Loading a new library would add a new element in the maps (and in the TLS block for each existing
thread), and unloading it would remove the library from all maps (and in the TLS block for each
thread). */
class ABACLADE_SYM thread_local_storage : public noncopyable {
public:
   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified thread_local_ptr_impl instance; it also initializes the m_ptlpiNext and m_ibTlsOffset
   members of the latter. This function will be called during initialization of a new dynamic
   library as it’s being loaded, not during normal run-time.

   ptlpi
      Pointer to the new variable to assign storage to.
   cb
      Requested storage size.
   */
   static void add_var(thread_local_ptr_impl * ptlpi, std::size_t cb);

#if ABC_HOST_API_WIN32
   /*! Hook invoked by DllMain() in abaclade.dll.

   iReason
      Reason why DllMain() was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
   */
   static bool dllmain_hook(unsigned iReason);
#endif

   /*! Returns a pointer to the specified offset in the storage. On the first call from a new
   thread, this also lazily creates the thread_local_storage, unless bCreateNewIfNull is false.

   bCreateNewIfNull
      If the TLS slot is nullptr and bCreateNewIfNull is true, a new new TLS instance will be
      created; if bCreateNewIfNull is false, nullptr will be returned instead if the TLS slot is
      uninitialized.
   return
      Pointer to the data store.
   */
   static thread_local_storage * get(bool bCreateNewIfNull = true);

   /*! Returns a pointer to the specified offset in the thread-local data store.

   ibOffset
      Desired offset.
   return
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

   pThis
      Pointer to the TLS for the current thread.
   */
   static void destruct(void * pThis = get());
#endif

   //! Deallocates the TLS slot for the process.
   // TODO: call free_slot() in the POSIX Threads case using reference counting in destruct().
   static void free_slot();

private:
   //! Raw byte storage.
   std::unique_ptr<std::int8_t[]> m_pb;
   //! OS-defined TLS key.
#if ABC_HOST_API_POSIX
   static pthread_key_t sm_pthkey;
#elif ABC_HOST_API_WIN32
   static DWORD sm_iTls;
#endif
#if ABC_HOST_API_POSIX
   //! One-time initializer for sm_pthkey.
   static pthread_once_t sm_pthonce;
#endif
   //! Pointer to the first thread_local_ptr_impl instance.
   static thread_local_ptr_impl const * sm_ptlpiHead;
   //! Cumulative storage size registered with calls to add_var().
   static std::size_t sm_cb;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_ptr_impl

namespace abc {
namespace detail {

//! Non-template implementation of abc::thread_local_ptr.
class ABACLADE_SYM thread_local_ptr_impl : public noncopyable {
private:
   friend class thread_local_storage;

protected:
   /*! Constructor.

   pfnConstruct
      Pointer to the subclass-provided constructor.
   pfnDestruct
      Pointer to the subclass-provided destructor.
   cbObject
      Size of the object pointed to by the thread_local_ptr subclass.
   */
   thread_local_ptr_impl(
      void (* pfnConstruct)(void *), void (* pfnDestruct)(void *), std::size_t cbObject
   );

   /*! Implementation of thread_local_ptr::get().

   return
      Pointer to the thread-local value for this object.
   */
   template <typename T>
   T * get() const {
      return static_cast<T *>(thread_local_storage::get()->get_storage(m_ibTlsOffset));
   }

private:
   //! Pointer to the next thread_local_ptr_impl instance.
   thread_local_ptr_impl const * m_ptlpiNext;
   //! Subclass-provided constructor; used to construct the thread-local value for new threads.
   void (* m_pfnConstruct)(void *);
   //! Subclass-provided destructor; used to destruct the thread-local for terminating threads.
   void (* m_pfnDestruct)(void *);
   //! Offset of this variable in the TLS block.
   std::size_t m_ibTlsOffset;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread_local_ptr

namespace abc {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each
thread, and an instance of this class lets each thread access its own private copy of the value
pointed to by it.

Variables of this type cannot be non-static class members. */
template <typename T>
class thread_local_ptr : private detail::thread_local_ptr_impl {
public:
   //! Constructor.
   thread_local_ptr() :
      detail::thread_local_ptr_impl(construct, destruct, sizeof(T)) {
   }

   /*! Returns the address of the thread-local value this object points to.

   return
      Internal pointer.
   */
   T * get() const {
      return detail::thread_local_ptr_impl::get<T>();
   }

private:
   /*! Constructs a T at the specified address. Invoked at most once for each thread.

   p
      Address where a T should be constructed.
   */
   static void construct(void * p) {
      new(p) T();
   }

   /*! Destructs the specified T. Invoked at most once for each thread.

   p
      Pointer to the T that should be destructed.
   */
   static void destruct(void * p) {
      static_cast<T *>(p)->~T();
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////


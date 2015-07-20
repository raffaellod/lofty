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
class thread_local_storage;

//! Abaclade’s TLS slot data manager.
class ABACLADE_SYM thread_local_storage :
   public collections::static_list<
      thread_local_storage, context_local_var_impl<thread_local_storage>
   >,
   public context_local_storage_impl {
private:
   friend class coroutine_local_storage;

public:
   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified context_local_var_impl instance; it also initializes the m_ptlviNext and
   m_ibStorageOffset members of the latter. This function will be called during initialization of a
   new dynamic library as it’s being loaded, not during normal run-time.

   @param ptlvi
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   static void add_var(context_local_var_impl<thread_local_storage> * ptlvi, std::size_t cb) {
      context_local_storage_impl::add_var(&sm_sm, ptlvi, cb);
   }

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
   /*! Storage for the active coroutine. If a coroutine::scheduler is running on a thread, this is
   replaced on each change of coroutine::scheduler::sm_pcoroctxActive. */
   coroutine_local_storage m_crls;
   //! Normally a pointer to m_crls, but replaced while a coroutine is being actively executed.
   coroutine_local_storage * m_pcrls;

   static static_members_t sm_sm;
};


// Now these can be defined.

/*static*/ inline coroutine_local_storage * coroutine_local_storage::get() {
   return thread_local_storage::get()->m_pcrls;
}

/*static*/ inline void coroutine_local_storage::get_default_and_current_pointers(
   coroutine_local_storage ** ppcrlsDefault, coroutine_local_storage *** pppcrlsCurrent
) {
   auto ptls = thread_local_storage::get();
   *ppcrlsDefault = &ptls->m_crls;
   *pppcrlsCurrent = &ptls->m_pcrls;
}

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Variable with separate per-thread values. Variables of this type cannot be non-static class
members. */
template <typename T>
class thread_local_value : public detail::context_local_value<T, detail::thread_local_storage> {
private:
   typedef detail::context_local_value<T, detail::thread_local_storage> context_local;

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

namespace abc {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each
thread, and an instance of this class lets each thread access its own private copy of the value
pointed to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class thread_local_ptr : public detail::context_local_ptr<T, detail::thread_local_storage> {
};

} //namespace abc

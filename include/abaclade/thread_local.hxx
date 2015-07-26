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

// Forward declaration.
class thread_local_storage;

//! Abaclade’s TLS variable registrar.
class ABACLADE_SYM thread_local_storage_registrar :
   public context_local_storage_registrar_impl,
   public collections::static_list_impl<
      thread_local_storage_registrar, context_local_storage_node<thread_local_storage>
   > {
public:
   /*! Returns the one and only instance of this class.

   @return
      *this.
   */
   static thread_local_storage_registrar & instance() {
      return static_cast<thread_local_storage_registrar &>(sm_dm.slib);
   }

private:
   //! Only instance of this class’ data.
   static data_members sm_dm;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Abaclade’s TLS slot data.
class ABACLADE_SYM thread_local_storage : public context_local_storage_impl {
private:
   friend class coroutine_local_storage;

public:
   //! Registrar that variables will register with at program startup.
   typedef thread_local_storage_registrar registrar;

public:
#if ABC_HOST_API_WIN32
   /*! Hook invoked by DllMain() in abaclade.dll.

   @param iReason
      Reason why DllMain() was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
   */
   static bool dllmain_hook(unsigned iReason);
#endif

   /*! Returns the thread_local_storage instance for the current thread. On the first call from a
   new thread, this also lazily creates the thread_local_storage instance, unless bCreateNewIfNull
   is false.

   @param bCreateNewIfNull
      If the TLS slot is nullptr and bCreateNewIfNull is true, a new new TLS instance will be
      created; if bCreateNewIfNull is false, nullptr will be returned instead if the TLS slot is
      uninitialized.
   @return
      Reference to the data store.
   */
   static thread_local_storage & instance(bool bCreateNewIfNull = true);

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
   static void destruct(void * pThis = &instance());
#endif

private:
   /*! Storage for the active coroutine. If a coroutine::scheduler is running on a thread, this is
   replaced on each change of coroutine::scheduler::sm_pcoroctxActive. */
   coroutine_local_storage m_crls;
   //! Normally a pointer to m_crls, but replaced while a coroutine is being actively executed.
   coroutine_local_storage * m_pcrls;
#if ABC_HOST_API_POSIX
   //! Counts how many storage instances exist, so that there’s a way to release the TLS slot.
   static std::atomic<unsigned> sm_cInstances;
#endif
};


// Now these can be defined.

/*static*/ inline coroutine_local_storage & coroutine_local_storage::instance() {
   return *thread_local_storage::instance().m_pcrls;
}

/*static*/ inline void coroutine_local_storage::get_default_and_current_pointers(
   coroutine_local_storage ** ppcrlsDefault, coroutine_local_storage *** pppcrlsCurrent
) {
   auto & tls = thread_local_storage::instance();
   *ppcrlsDefault = &tls.m_crls;
   *pppcrlsCurrent = &tls.m_pcrls;
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

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

// Forward declaration.
class thread_local_storage;

//! Lofty’s TLS variable registrar.
class LOFTY_SYM thread_local_storage_registrar :
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
      return static_cast<thread_local_storage_registrar &>(data_members_.list);
   }

private:
   //! Only instance of this class’ data.
   static data_members data_members_;
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Lofty’s TLS slot data.
class LOFTY_SYM thread_local_storage : public context_local_storage_impl {
private:
   friend class coroutine_local_storage;

public:
   //! Registrar that variables will register with at program startup.
   typedef thread_local_storage_registrar registrar;

public:
   /*! Default constructor. Publicly accessible so that under POSIX, lofty::thread::impl and lofty::app::run()
   can instantiate this class instead of using the lazy initialization. */
   thread_local_storage();

   //! Destructor.
   ~thread_local_storage();

#if LOFTY_HOST_API_WIN32
   /*! Hook invoked by DllMain() in lofty.dll.

   @param reason
      Reason why DllMain() was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
   */
   static bool dllmain_hook(unsigned reason);
#endif

   /*! Returns the thread_local_storage instance for the current thread. On the first call from a new thread,
   this also lazily creates the thread_local_storage instance, unless create_new_if_null is false.

   @param create_new_if_null
      If the TLS slot is nullptr and create_new_if_null is true, a new new TLS instance will be created; if
      create_new_if_null is false, nullptr will be returned instead if the TLS slot is uninitialized.
   @return
      Reference to the data store.
   */
   static thread_local_storage & instance(bool create_new_if_null = true);

private:
#if LOFTY_HOST_API_POSIX
   /*! Destructs the storage instance for the current thread. Invoked by pthread_key_create() when a thread
   terminates.

   @param thread_this
      Pointer to the TLS for the current thread.
   */
   static void destruct(void * thread_this);
#endif

private:
   /*! Default coroutine local storage for the thread. If a coroutine::scheduler is running on a thread,
   *current_crls is used instead, replaced on each change of coroutine::scheduler::active_coro_pimpl. */
   coroutine_local_storage default_crls;
   //! Normally a pointer to default_crls, but replaced while a coroutine is being actively executed.
   coroutine_local_storage * current_crls;
#if LOFTY_HOST_API_POSIX
   //! Count of instances.
   static _std::atomic<unsigned> instances_count;
#endif
};


// Now these can be defined.

/*static*/ inline coroutine_local_storage & coroutine_local_storage::instance() {
   return *thread_local_storage::instance().current_crls;
}

/*static*/ inline void coroutine_local_storage::get_default_and_current_pointers(
   coroutine_local_storage ** default_crls, coroutine_local_storage *** current_crls
) {
   auto & tls = thread_local_storage::instance();
   *default_crls = &tls.default_crls;
   *current_crls = &tls.current_crls;
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Variable with separate per-thread values. Variables of this type cannot be non-static class members.
template <typename T>
class thread_local_value : public _pvt::context_local_value<T, _pvt::thread_local_storage> {
private:
   typedef _pvt::context_local_value<T, _pvt::thread_local_storage> context_local;

public:
   //! See _pvt::context_local_value::operator=().
   thread_local_value & operator=(T const & t) {
      context_local::operator=(t);
      return *this;
   }

   //! See _pvt::context_local_value::operator=().
   thread_local_value & operator=(T && t) {
      context_local::operator=(_std::move(t));
      return *this;
   }
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Thread-local pointer to an object. The memory this points to is permanently allocated for each thread, and
an instance of this class lets each thread access its own private copy of the value pointed to by it.
Variables of this type cannot be non-static class members. */
template <typename T>
class thread_local_ptr : public _pvt::context_local_ptr<T, _pvt::thread_local_storage> {
};

} //namespace lofty

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>

#if LOFTY_HOST_API_POSIX
   #include <pthread.h>
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

thread_local_storage_registrar::data_members thread_local_storage_registrar::data_members_ =
   LOFTY__PVT_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

#if LOFTY_HOST_API_POSIX
   //! TLS key.
   static pthread_key_t tls_key;
   _std::atomic<unsigned> thread_local_storage::instances_count(0);
#elif LOFTY_HOST_API_WIN32
   //! TLS index.
   static ::DWORD tls_index = TLS_OUT_OF_INDEXES;
#endif

thread_local_storage::thread_local_storage() :
   context_local_storage_impl(&thread_local_storage_registrar::instance()),
   current_crls(&default_crls) {

#if LOFTY_HOST_API_POSIX
   if (instances_count++ == 0) {
      if (int err = pthread_key_create(&tls_key, &destruct)) {
         LOFTY_UNUSED_ARG(err);
         // throw an exception (err).
      }
   }
   pthread_setspecific(tls_key, this);
#elif LOFTY_HOST_API_WIN32
   ::TlsSetValue(tls_index, this);
#endif
}

thread_local_storage::~thread_local_storage() {
   unsigned remaining_attempts = 10;
   bool any_destructed;
   do {
      // Destruct CRLS for this thread.
      any_destructed = default_crls.destruct_vars(coroutine_local_storage_registrar::instance());
      if (destruct_vars(thread_local_storage_registrar::instance())) {
         any_destructed = true;
      }
   } while (--remaining_attempts > 0 && any_destructed);

#if LOFTY_HOST_API_POSIX
   pthread_setspecific(tls_key, nullptr);
   if (--instances_count == 0) {
      pthread_key_delete(tls_key);
   }
#elif LOFTY_HOST_API_WIN32
   ::TlsSetValue(tls_index, nullptr);
#endif
}

#if LOFTY_HOST_API_POSIX
/*static*/ void thread_local_storage::destruct(void * thread_this) {
   /* This is necessary (at least under Linux/glibc) to prevent creating a duplicate (which will be
   leaked) due to re-entrant calls to instance() in the destructor. The destructor ensures that this
   pointer is eventually cleared. */
   pthread_setspecific(tls_key, thread_this);
   delete static_cast<thread_local_storage *>(thread_this);
}
#endif

#if LOFTY_HOST_API_WIN32
/*static*/ bool thread_local_storage::dllmain_hook(unsigned reason) {
   if (reason == DLL_PROCESS_ATTACH) {
      tls_index = ::TlsAlloc();
      if (tls_index == TLS_OUT_OF_INDEXES) {
         // throw an exception (::GetLastError()).
      }
   } else if (reason == DLL_THREAD_DETACH || reason == DLL_PROCESS_DETACH) {
      /* Allow instance() to return nullptr if the TLS slot was not initialized for this thread, in
      which case nothing will happen. */
      delete &instance(false);
      if (reason == DLL_PROCESS_DETACH) {
         ::TlsFree(tls_index);
      }
   }
   // TODO: handle errors and return false in case.
   return true;
}
#endif //if LOFTY_HOST_API_WIN32

/*static*/ thread_local_storage & thread_local_storage::instance(bool create_new_if_null /*= true*/) {
   void * thread_this =
#if LOFTY_HOST_API_POSIX
      pthread_getspecific(tls_key);
#elif LOFTY_HOST_API_WIN32
      ::TlsGetValue(tls_index);
#endif
   if (thread_this || !create_new_if_null) {
      return *static_cast<thread_local_storage *>(thread_this);
   } else {
      // First call for this thread: initialize the TLS slot.
      return *(new thread_local_storage);
   }
}

}} //namespace lofty::_pvt

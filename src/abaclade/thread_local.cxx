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

#include <abaclade.hxx>

#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

thread_local_storage_registrar::data_members thread_local_storage_registrar::sm_dm =
   ABC_DETAIL_CONTEXT_LOCAL_STORAGE_REGISTRAR_INITIALIZER;

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

#if ABC_HOST_API_POSIX
   //! One-time initializer for g_pthkey.
   static pthread_once_t g_pthonce = PTHREAD_ONCE_INIT;
   //! TLS key.
   static pthread_key_t g_pthkey;
#elif ABC_HOST_API_WIN32
   //! TLS index.
   static ::DWORD g_iTls = TLS_OUT_OF_INDEXES;
#endif

thread_local_storage::thread_local_storage() :
   context_local_storage_impl(&thread_local_storage_registrar::instance()),
   m_pcrls(&m_crls) {

#if ABC_HOST_API_POSIX
   pthread_setspecific(g_pthkey, this);
#elif ABC_HOST_API_WIN32
   ::TlsSetValue(g_iTls, this);
#endif
}

thread_local_storage::~thread_local_storage() {
   unsigned iRemainingAttempts = 10;
   bool bAnyDestructed;
   do {
      // Destruct CRLS for this thread.
      bAnyDestructed = m_crls.destruct_vars(coroutine_local_storage_registrar::instance());
      if (destruct_vars(thread_local_storage_registrar::instance())) {
         bAnyDestructed = true;
      }
   } while (--iRemainingAttempts > 0 && bAnyDestructed);

#if ABC_HOST_API_POSIX
   pthread_setspecific(g_pthkey, nullptr);
#elif ABC_HOST_API_WIN32
   ::TlsSetValue(g_iTls, nullptr);
#endif
}

/*static*/ void thread_local_storage::alloc_slot() {
#if ABC_HOST_API_POSIX
   if (int iErr = pthread_key_create(&g_pthkey, &destruct)) {
      ABC_UNUSED_ARG(iErr);
      // throw an exception (iErr).
   }
#elif ABC_HOST_API_WIN32
   g_iTls = ::TlsAlloc();
   if (g_iTls == TLS_OUT_OF_INDEXES) {
      // throw an exception (::GetLastError()).
   }
#endif
}

#if ABC_HOST_API_POSIX
/*static*/ void thread_local_storage::destruct(void * pThis /*= &instance()*/) {
   delete static_cast<thread_local_storage *>(pThis);
}
#endif

#if ABC_HOST_API_WIN32
/*static*/ bool thread_local_storage::dllmain_hook(unsigned iReason) {
   if (iReason == DLL_PROCESS_ATTACH || iReason == DLL_THREAD_ATTACH) {
      if (iReason == DLL_PROCESS_ATTACH) {
         alloc_slot();
      }
      // Not calling construct() since initialization of TLS is lazy.
   } else if (iReason == DLL_THREAD_DETACH || iReason == DLL_PROCESS_DETACH) {
      /* Allow instance() to return nullptr if the TLS slot was not initialized for this thread, in
      which case nothing will happen. */
      delete &instance(false);
      if (iReason == DLL_PROCESS_DETACH) {
         free_slot();
      }
   }
   // TODO: handle errors and return false in case.
   return true;
}
#endif //if ABC_HOST_API_WIN32

/*static*/ void thread_local_storage::free_slot() {
#if ABC_HOST_API_POSIX
   pthread_key_delete(g_pthkey);
#elif ABC_HOST_API_WIN32
   ::TlsFree(g_iTls);
#endif
}

/*static*/ thread_local_storage & thread_local_storage::instance(bool bCreateNewIfNull /*= true*/) {
   void * pThis;
#if ABC_HOST_API_POSIX
   // With POSIX Threads we need a one-time call to alloc_slot().
   pthread_once(&g_pthonce, &alloc_slot);
   pThis = pthread_getspecific(g_pthkey);
#elif ABC_HOST_API_WIN32
   // Under Win32, alloc_slot() has already been called by dllmain_hook().
   pThis = ::TlsGetValue(g_iTls);
#endif
   if (pThis || !bCreateNewIfNull) {
      return *static_cast<thread_local_storage *>(pThis);
   } else {
      // First call for this thread: initialize the TLS slot.
      return *(new thread_local_storage);
   }
}

}} //namespace abc::detail

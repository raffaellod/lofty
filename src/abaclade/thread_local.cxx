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
#include <abaclade/bitmanip.hxx>

#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_storage

namespace abc {
namespace detail {

namespace {

#if ABC_HOST_API_POSIX
   //! One-time initializer for g_pthkey.
   pthread_once_t g_pthonce = PTHREAD_ONCE_INIT;
   //! TLS key.
   pthread_key_t g_pthkey;
#elif ABC_HOST_API_WIN32
   //! TLS index.
   DWORD g_iTls = TLS_OUT_OF_INDEXES;
#endif

} //namespace

ABC_COLLECTIONS_STATIC_LIST_DEFINE_SUBCLASS_STATIC_MEMBERS(thread_local_storage)
std::size_t thread_local_storage::sm_cb = 0;

thread_local_storage::thread_local_storage() :
   m_pb(new std::int8_t[sm_cb]) {

   // Iterate over the list to construct TLS for this thread.
   for (auto it(begin()), itEnd(end()); it != itEnd; ++it) {
      it->construct(get_storage(it->m_ibTlsOffset));
   }

#if ABC_HOST_API_POSIX
   pthread_setspecific(g_pthkey, this);
#elif ABC_HOST_API_WIN32
   ::TlsSetValue(g_iTls, this);
#endif
}

thread_local_storage::~thread_local_storage() {
   // Iterate backwards over the list to destruct TLS for this thread.
   for (auto it(rbegin()), itEnd(rend()); it != itEnd; ++it) {
      it->destruct(get_storage(it->m_ibTlsOffset));
   }
}

/*static*/ void thread_local_storage::add_var(thread_local_var_impl * ptlvi, std::size_t cb) {
   // Calculate the offset for *ptlvi’s storage and increase sm_cb accordingly.
   ptlvi->m_ibTlsOffset = sm_cb;
   sm_cb += bitmanip::ceiling_to_pow2_multiple(cb, sizeof(abc::max_align_t));
}

/*static*/ void thread_local_storage::alloc_slot() {
#if ABC_HOST_API_POSIX
   if (int iErr = pthread_key_create(&g_pthkey, destruct)) {
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
/*static*/ void thread_local_storage::destruct(void * pThis /*= get()*/) {
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
      // Allow get() to return nullptr if the TLS slot was not initialized for this thread, in which
      // case nothing will happen.
      delete get(false);
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

/*static*/ thread_local_storage * thread_local_storage::get(bool bCreateNewIfNull /*= true*/) {
   void * pThis;
#if ABC_HOST_API_POSIX
   // With POSIX Threads we need a one-time call to alloc_slot().
   pthread_once(&g_pthonce, alloc_slot);
   pThis = pthread_getspecific(g_pthkey);
#elif ABC_HOST_API_WIN32
   // Under Win32, alloc_slot() has already been called by dllmain_hook().
   pThis = ::TlsGetValue(g_iTls);
#endif
   if (pThis || !bCreateNewIfNull) {
      return static_cast<thread_local_storage *>(pThis);
   } else {
      // First call for this thread: initialize the TLS slot.
      return new thread_local_storage;
   }
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_var_impl

namespace abc {
namespace detail {

thread_local_var_impl::thread_local_var_impl(std::size_t cbObject) {
   // Initializes m_ptlviNext and m_ibTlsOffset.
   thread_local_storage::add_var(this, cbObject);
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

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

#include <abaclade.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::thread_local_storage

namespace abc {
namespace detail {

#if ABC_HOST_API_POSIX
pthread_once_t thread_local_storage::sm_pthonce = PTHREAD_ONCE_INIT;
pthread_key_t thread_local_storage::sm_pthkey;
#elif ABC_HOST_API_WIN32
DWORD thread_local_storage::sm_iTls = TLS_OUT_OF_INDEXES;
#endif
thread_local_ptr_impl const * thread_local_storage::sm_ptlpiHead = nullptr;
std::size_t thread_local_storage::sm_cb = 0;

thread_local_storage::thread_local_storage() :
   m_pb(new std::int8_t[sm_cb]) {

   // Iterate over the list of thread_local_ptr constructors to initialize TLS for this thread.
   for (thread_local_ptr_impl const * ptlpi = sm_ptlpiHead; ptlpi; ptlpi = ptlpi->m_ptlpiNext) {
      ptlpi->m_pfnConstruct(get_storage(ptlpi->m_ibTlsOffset));
   }

#if ABC_HOST_API_POSIX
   pthread_setspecific(sm_pthkey, this);
#elif ABC_HOST_API_WIN32
   ::TlsSetValue(sm_iTls, this);
#endif
}

thread_local_storage::~thread_local_storage() {
   // Iterate over the list of thread_local_ptr destructors to deinitialize TLS for this thread.
   for (thread_local_ptr_impl const * ptlpi = sm_ptlpiHead; ptlpi; ptlpi = ptlpi->m_ptlpiNext) {
      ptlpi->m_pfnDestruct(get_storage(ptlpi->m_ibTlsOffset));
   }
}

/*static*/ void thread_local_storage::add_var(thread_local_ptr_impl * ptlpi, std::size_t cb) {
   // Insert *ptlpi as the first element in the list.
   ptlpi->m_ptlpiNext = sm_ptlpiHead;
   sm_ptlpiHead = ptlpi;
   // Calculate the offset for *ptlpi’s storage and increase sm_cb accordingly.
   ptlpi->m_ibTlsOffset = sm_cb;
   sm_cb += ABC_ALIGNED_SIZE(cb);
}

/*static*/ void thread_local_storage::alloc_slot() {
#if ABC_HOST_API_POSIX
   if (int iErr = pthread_key_create(&sm_pthkey, destruct)) {
      ABC_UNUSED_ARG(iErr);
      // throw an exception (iErr).
   }
#elif ABC_HOST_API_WIN32
   sm_iTls = ::TlsAlloc();
   if (sm_iTls == TLS_OUT_OF_INDEXES) {
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
      // get() may return nullptr, in which case nothing will happen.
      delete get(true);
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
   pthread_key_delete(sm_pthkey);
#elif ABC_HOST_API_WIN32
   ::TlsFree(sm_iTls);
#endif
}

/*static*/ thread_local_storage * thread_local_storage::get(bool bCreateNewIfNull /*= true*/) {
   void * pThis;
#if ABC_HOST_API_POSIX
   // With POSIX Threads we need a one-time call to alloc_slot().
   pthread_once(&sm_pthonce, alloc_slot);
   pThis = pthread_getspecific(sm_pthkey);
#elif ABC_HOST_API_WIN32
   // Under Win32, alloc_slot() has already been called by dllmain_hook().
   pThis = ::TlsGetValue(sm_iTls);
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
// abc::detail::thread_local_ptr_impl

namespace abc {
namespace detail {

thread_local_ptr_impl::thread_local_ptr_impl(
   void (* pfnConstruct)(void *), void (* pfnDestruct)(void *), std::size_t cbObject
) :
   m_pfnConstruct(pfnConstruct),
   m_pfnDestruct(pfnDestruct) {
   // Initializes m_ptlpiNext and m_ibTlsOffset.
   thread_local_storage::add_var(this, cbObject);
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////


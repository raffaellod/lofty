/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_ATOMIC_HXX
#define _ABACLADE_ATOMIC_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#if ABC_HOST_API_POSIX
   #include <pthread.h>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::atomic globals

namespace abc {
namespace atomic {

//! Integer type of optimal size for atomic operations (usually the machine’s word size).
#if ABC_HOST_API_POSIX
   // No preference really, since we use don’t use atomic intrinsics.
   typedef int int_t;
#elif ABC_HOST_API_WIN32
   // Win32 uses long to mean 32 bits, always.
   typedef long int_t;
#else
   #error HOST_API
#endif

#if ABC_HOST_API_POSIX
extern pthread_mutex_t g_mtx;
#endif

/*! Atomically add the second argument to the number pointed to by the first argument, storing the
result in *piDst and returning it.

piDst
   Pointer to an integer variable whose value is the left addend and that will receive the sum upon
   return.
iAddend
   Right addend.
return
   Sum of *piDst and iAddend.
*/
template <typename I>
inline I add(I volatile * piDst, I iAddend) {
#if ABC_HOST_API_POSIX
   I iRet;
   pthread_mutex_lock(&g_mtx);
   iRet = (*piDst += iAddend);
   pthread_mutex_unlock(&g_mtx);
   return iRet;
#elif ABC_HOST_API_WIN32
   switch (sizeof(I)) {
      case sizeof(long):
         return ::InterlockedAdd(reinterpret_cast<long volatile *>(piDst), iAddend);
#if _WIN32_WINNT >= 0x0502
      case sizeof(long long):
         return ::InterlockedAdd64(reinterpret_cast<long long volatile *>(piDst), iAddend);
#endif //if _WIN32_WINNT >= 0x0502
   }
#else
   #error HOST_API
#endif
}

/*! Atomically add the second argument to the number pointed to by the first argument, storing the
result in *pi and returning it.

piDst
   Pointer to an integer variable whose value is to be replaced with iNewValue if and only if it
   matches iComparand.
iNewValue
   Value to assign to *piDst.
iComparand
   Expected current value of *piDst.
return
   Previous value of *pi, as well as the current one if *pi was not changed.
*/
template <typename I>
inline I compare_and_swap(I volatile * piDst, I iNewValue, I iComparand) {
#if ABC_HOST_API_POSIX
   I iOldValue;
   pthread_mutex_lock(&g_mtx);
   if ((iOldValue = *piDst) == iComparand) {
      *piDst = iNewValue;
   }
   pthread_mutex_unlock(&g_mtx);
   return iOldValue;
#elif ABC_HOST_API_WIN32
   switch (sizeof(I)) {
      case sizeof(long):
         return ::InterlockedCompareExchange(
            reinterpret_cast<long volatile *>(piDst), iNewValue, iComparand
         );
#if _WIN32_WINNT >= 0x0502
      case sizeof(long long):
         return ::InterlockedCompareExchange64(
            reinterpret_cast<long long volatile *>(piDst), iNewValue, iComparand
         );
#endif //if _WIN32_WINNT >= 0x0502
   }
#else
   #error HOST_API
#endif
}

/*! Atomically decrements the number pointed to by the argument, storing the result in *pi and
returning it.

pi
   Pointer to an integer variable whose value is to be decremented by 1.
return
   New value of *pi.
*/
template <typename I>
inline I decrement(I volatile * pi) {
#if ABC_HOST_API_POSIX
   I iRet;
   pthread_mutex_lock(&g_mtx);
   iRet = --*pi;
   pthread_mutex_unlock(&g_mtx);
   return iRet;
#elif ABC_HOST_API_WIN32
   switch (sizeof(I)) {
      case sizeof(long):
         return I(::InterlockedDecrement(reinterpret_cast<long volatile *>(pi)));
#if _WIN32_WINNT >= 0x0502
      case sizeof(long long):
         return I(::InterlockedDecrement64(reinterpret_cast<long long volatile *>(pi)));
#endif //if _WIN32_WINNT >= 0x0502
   }
#else
   #error HOST_API
#endif
}

/*! Atomically increments the number pointed to by the argument, storing the result in *pi and
returning it.

pi
   Pointer to an integer variable whose value is to be incremented by 1.
return
   New value of *pi.
*/
template <typename I>
inline I increment(I volatile * pi) {
#if ABC_HOST_API_POSIX
   I iRet;
   pthread_mutex_lock(&g_mtx);
   iRet = ++*pi;
   pthread_mutex_unlock(&g_mtx);
   return iRet;
#elif ABC_HOST_API_WIN32
   switch (sizeof(I)) {
      case sizeof(long):
         return I(::InterlockedIncrement(reinterpret_cast<long volatile *>(pi)));
#if _WIN32_WINNT >= 0x0502
      case sizeof(long long):
         return I(::InterlockedIncrement64(reinterpret_cast<long long volatile *>(pi)));
#endif //if _WIN32_WINNT >= 0x0502
   }
#else
   #error HOST_API
#endif
}

/*! Atomically subtracts the second argument from the number pointed to by the first argument,
storing the result in *pi and returning it.

piDst
   Pointer to an integer variable whose value is the minuend and that will receive the difference
   upon return.
iAddend
   Subtrahend.
return
   Difference between *piDst and iAddend.
*/
template <typename I>
inline I subtract(I volatile * piDst, I iSubtrahend) {
#if ABC_HOST_API_POSIX
   I iRet;
   pthread_mutex_lock(&g_mtx);
   iRet = (*piDst -= iSubtrahend);
   pthread_mutex_unlock(&g_mtx);
   return iRet;
#elif ABC_HOST_API_WIN32
   switch (sizeof(I)) {
      case sizeof(long):
         return ::InterlockedAdd(
            reinterpret_cast<long volatile *>(piDst), -long(iSubtrahend)
         );
#if _WIN32_WINNT >= 0x0502
      case sizeof(long long):
         return ::InterlockedAdd64(
            reinterpret_cast<long long volatile *>(piDst), -long long(iSubtrahend)
         );
#endif //if _WIN32_WINNT >= 0x0502
   }
#else
   #error HOST_API
#endif
}

} //namespace atomic
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_ATOMIC_HXX


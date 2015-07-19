/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#if ABC_HOST_API_WIN32
/*! Entry point for abaclade.dll.

hinst
   Module’s instance handle.
iReason
   Reason why this function was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
return
   true in case of success, or false otherwise.
*/
extern "C" ::BOOL WINAPI DllMain(::HINSTANCE hinst, ::DWORD iReason, void * pReserved) {
   ABC_UNUSED_ARG(hinst);
   ABC_UNUSED_ARG(pReserved);
   if (!abc::detail::thread_local_storage::dllmain_hook(static_cast<unsigned>(iReason))) {
      return false;
   }
   return true;
}
#endif //if ABC_HOST_API_WIN32

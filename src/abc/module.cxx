/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc.hxx>
#include <abc/module.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::app_module


namespace abc {

/*static*/ app_module * app_module::sm_pappmod;


app_module::app_module() {
   // Asserting here is okay, because if the assertion is true, nothing will happen, and if it’s
   // not, that means that there’s already a module with all the infrastructure needed to handle a
   // failed assertion.
   ABC_ASSERT(!sm_pappmod, SL("multiple instantiation of abc::app_module singleton class"));
   sm_pappmod = this;
}


/*virtual*/ app_module::~app_module() {
   sm_pappmod = nullptr;
}


/*static*/ void app_module::_build_args(
   int cArgs, char_t ** ppszArgs, mvector<istr const> * pvsRet
) {
   ABC_TRACE_FN((cArgs, ppszArgs, pvsRet));

   pvsRet->set_capacity(size_t(cArgs), false);
   // Make each string not allocate a new character array.
   for (int i(0); i < cArgs; ++i) {
      pvsRet->append(istr(unsafe, ppszArgs[i]));
   }
}
#if ABC_HOST_API_WIN32
/*static*/ void app_module::_build_args(mvector<istr const> * pvsRet) {
   ABC_TRACE_FN((pvsRet));

   // TODO: call ::GetCommandLine() and parse its result.
}
#endif

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


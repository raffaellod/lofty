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

#include <abc/core.hxx>
#include <abc/module.hxx>
#if ABC_HOST_API_POSIX
   #include <dlfcn.h> // RTLD_LAZY dlclose() dlopen() dlerror() dlsym()
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dynamic_module


#if ABC_HOST_API_WIN32

namespace abc {

dynamic_module::dynamic_module(dynamic_module && dm) :
   m_hdynmod(dm.m_hdynmod),
   m_bOwn(dm.m_bOwn) {
   dm.m_hdynmod = nullptr;
   dm.m_bOwn = false;
}
dynamic_module::dynamic_module(file_path const & fp, bool bInit) :
   m_hdynmod(::LoadLibraryEx(
      fp.os_str().c_str().get(), nullptr, DWORD(bInit ? 0 : LOAD_LIBRARY_AS_DATAFILE)
   )),
   m_bOwn(true) {
   ABC_TRACE_FN((this, /*fp, */bInit));

   if (!m_hdynmod) {
      throw_os_error();
   }
}


dynamic_module & dynamic_module::operator=(dynamic_module && dm) {
   ABC_TRACE_FN((this/*, dm*/));

   m_hdynmod = dm.m_hdynmod;
   m_bOwn = dm.m_bOwn;
   dm.m_hdynmod = nullptr;
   dm.m_bOwn = false;
   return *this;
}


file_path dynamic_module::file_name() const {
   ABC_TRACE_FN((this));

   dmstr s;
   hdynmod_t hdynmod(m_hdynmod);
   s.grow_for([hdynmod] (char_t * pch, size_t cchMax) -> size_t {
      // Since ::GetModuleFileName() does not include the terminating NUL in the returned character
      // count, it has to return at most cchMax - 1 characters; if it returns cchMax, the buffer was
      // not large enough.

      size_t cchRet(::GetModuleFileName(hdynmod, pch, DWORD(cchMax)));
      if (!cchRet) {
         throw_os_error();
      }
      return cchRet;
   });
   return std::move(s);
}

} //namespace abc

#endif //if ABC_HOST_API_WIN32


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::resource_module


namespace abc {

resource_module::resource_module(file_path const & fp)
#if ABC_HOST_API_POSIX
{
   ABC_UNUSED_ARG(fp);
#elif ABC_HOST_API_WIN32
   : dynamic_module(fp, false) {
#else
   #error HOST_API
#endif
}
resource_module::resource_module(resource_module && rm)
#if ABC_HOST_API_POSIX
{
   ABC_UNUSED_ARG(rm);
#elif ABC_HOST_API_WIN32
   : dynamic_module(std::move(rm)) {
#else
   #error HOST_API
#endif
}


resource_module::~resource_module() {
}


size_t resource_module::load_string(short id, char_t * psz, size_t cchMax) const {
   ABC_TRACE_FN((this, id, psz, cchMax));

#if ABC_HOST_API_POSIX
   ABC_UNUSED_ARG(id);
   ABC_UNUSED_ARG(psz);
   ABC_UNUSED_ARG(cchMax);
   return 0;
#elif ABC_HOST_API_WIN32
   return size_t(::LoadString(m_hdynmod, WORD(id), psz, int(cchMax)));
#else
   #error HOST_API
#endif
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::code_module


namespace abc {

code_module::code_module(file_path const & fp) :
#if ABC_HOST_API_POSIX
   m_hdynmod(::dlopen(fp.os_str().c_str().get(), RTLD_LAZY)) {
   ABC_TRACE_FN((this/*, fp*/));

   if (!m_hdynmod) {
      throw_os_error();
   }
#elif ABC_HOST_API_WIN32
   dynamic_module(fp, true) {
#else
   #error HOST_API
#endif
}
code_module::code_module(code_module && cm) :
#if ABC_HOST_API_POSIX
   m_hdynmod(cm.m_hdynmod) {
   cm.m_hdynmod = nullptr;
#elif ABC_HOST_API_WIN32
   dynamic_module(std::move(cm)) {
#else
   #error HOST_API
#endif
}


code_module::~code_module() {
#if ABC_HOST_API_POSIX
   if (m_hdynmod) {
      ::dlclose(m_hdynmod);
   }
#endif
}


void * code_module::_get_symbol(istr const & sSymbol) {
   ABC_TRACE_FN((this, sSymbol));

   void * pfn;
#if ABC_HOST_API_POSIX
   ::dlerror();
   pfn = ::dlsym(m_hdynmod, sSymbol.c_str().get());
   if (char * pszError = ::dlerror()) {
      // TODO: we have a description, but no error code.
      ABC_UNUSED_ARG(pszError);
//    throw_os_error();
      throw 123;
   }
#elif ABC_HOST_API_WIN32
   // TODO: FIXME: translate sSymbol.c_str().get() from istr to istr8.
   pfn = ::GetProcAddress(m_hdynmod, nullptr);
   if (!pfn) {
      throw_os_error();
   }
#else
   #error HOST_API
#endif
   return pfn;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::module_impl_base


namespace abc {

#if ABC_HOST_API_WIN32
HINSTANCE module_impl_base::sm_hinst;
#endif


module_impl_base::module_impl_base() :
#if ABC_HOST_API_POSIX
   code_module(),
   resource_module() {
#elif ABC_HOST_API_WIN32
   code_module(sm_hinst),
   resource_module(sm_hinst),
   m_cRefs(0) {
#else
   #error HOST_API
#endif
}


/*static*/ void module_impl_base::_build_args(
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
/*static*/ void module_impl_base::_build_args(mvector<istr const> * pvsRet) {
   ABC_TRACE_FN((pvsRet));

   // TODO: call ::GetCommandLine() and parse its result.
}
#endif


} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


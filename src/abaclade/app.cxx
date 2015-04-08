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
#include <abaclade/app.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::app

namespace abc {

/*static*/ app * app::sm_papp;

app::app() {
   /* Asserting here is okay because if the assertion is true nothing will happen, and if it’s not
   that means that there’s already an app instance with all the infrastructure needed to handle a
   failed assertion. */
   ABC_ASSERT(!sm_papp, ABC_SL("multiple instantiation of abc::app singleton class"));
   sm_papp = this;
}

/*virtual*/ app::~app() {
   sm_papp = nullptr;
}

/*static*/ void app::_build_args(
   int cArgs, char_t ** ppszArgs, collections::mvector<istr const> * pvsRet
) {
   ABC_TRACE_FUNC(cArgs, ppszArgs, pvsRet);

   pvsRet->set_capacity(static_cast<std::size_t>(cArgs), false);
   // Make each string not allocate a new character array.
   for (int i = 0; i < cArgs; ++i) {
      pvsRet->push_back(istr(external_buffer, ppszArgs[i]));
   }
}
#if ABC_HOST_API_WIN32
/*static*/ void app::_build_args(collections::mvector<istr const> * pvsRet) {
   ABC_TRACE_FUNC(pvsRet);

   // TODO: call ::GetCommandLine() and parse its result.
}
#endif

/*static*/ bool app::initialize_stdio() {
   try {
      io::binary::stderr = io::binary::detail::make_stderr();
      io::binary::stdin  = io::binary::detail::make_stdin ();
      io::binary::stdout = io::binary::detail::make_stdout();
      io::text::stderr = io::text::detail::make_stderr();
      io::text::stdin  = io::text::detail::make_stdin ();
      io::text::stdout = io::text::detail::make_stdout();
      return true;
   } catch (std::exception const & x) {
      // Exceptions can’t be reported at this point.
      return false;
   } catch (...) {
      // Exceptions can’t be reported at this point.
      return false;
   }
}

/*static*/ bool app::deinitialize_stdio() {
   ABC_TRACE_FUNC();

   try {
      io::text::stdout->flush();
      io::binary::stdout->flush();
      io::text::stderr->flush();
      io::binary::stderr->flush();

      io::text::stdin.reset();
      io::binary::stdin.reset();
      io::text::stdout.reset();
      io::binary::stdout.reset();
      io::text::stderr.reset();
      io::binary::stderr.reset();
      return true;
   } catch (std::exception const & x) {
      if (io::text::stderr) {
         exception::write_with_scope_trace(nullptr, &x);
      }
      // Else, exceptions can’t be reported at this point, since we just closed stderr.
      return false;
   } catch (...) {
      if (io::text::stderr) {
         exception::write_with_scope_trace();
      }
      // Else, exceptions can’t be reported at this point, since we just closed stderr.
      return false;
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

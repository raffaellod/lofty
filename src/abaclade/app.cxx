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
#include "exception-fault_converter.hxx"
#include "thread-tracker.hxx"


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*static*/ app * app::sm_papp = nullptr;

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

/*static*/ int app::call_main(app * papp, _args_t * pargs) {
   ABC_TRACE_FUNC(papp, pargs);

   collections::smvector<istr, 8> vsArgs;
// TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
#if ABC_HOST_API_WIN32 && defined(ABC_HOST_API_WIN32_GUI)
   // TODO: call ::GetCommandLine() and parse its result.
#else
   vsArgs.set_capacity(static_cast<std::size_t>(pargs->cArgs), false);
   // Make each string not allocate a new character array.
   for (int i = 0; i < pargs->cArgs; ++i) {
      vsArgs.push_back(istr(external_buffer, pargs->ppszArgs[i]));
   }
#endif

   // Invoke the program-defined main().
   return papp->main(vsArgs);
}

/*static*/ bool app::deinitialize_stdio() {
   ABC_TRACE_FUNC();

   bool bErrors = false;
   io::text::stdin.reset();
   io::binary::stdin.reset();
   try {
      io::text::stdout->finalize();
   } catch (std::exception const & x) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   } catch (...) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   }
   io::text::stdout.reset();
   try {
      io::binary::stdout->finalize();
   } catch (std::exception const & x) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   } catch (...) {
      if (io::text::stderr) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
      }
      bErrors = true;
   }
   io::binary::stdout.reset();
   try {
      io::text::stderr->finalize();
   } catch (std::exception const &) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   } catch (...) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   }
   io::text::stderr.reset();
   try {
      io::binary::stderr->finalize();
   } catch (std::exception const &) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   } catch (...) {
      // FIXME: EXC-SWALLOW
      bErrors = true;
   }
   io::binary::stderr.reset();
   return !bErrors;
}

/*static*/ bool app::initialize_stdio() {
   try {
      io::binary::stderr = io::binary::detail::make_stderr();
      io::binary::stdin  = io::binary::detail::make_stdin ();
      io::binary::stdout = io::binary::detail::make_stdout();
      io::text::stderr = io::text::detail::make_stderr();
      io::text::stdin  = io::text::detail::make_stdin ();
      io::text::stdout = io::text::detail::make_stdout();
      return true;
   } catch (std::exception const &) {
      // Exceptions can’t be reported at this point.
      return false;
   } catch (...) {
      // Exceptions can’t be reported at this point.
      return false;
   }
}

/*static*/ int app::run(int (* pfnInstantiateAppAndCallMain)(_args_t *), _args_t * pargs) {
   // Establish these as early as possible.
   exception::fault_converter xfc;
   // Under POSIX, this also creates the TLS slot.
   detail::thread_local_storage tls;

   int iRet;
   if (initialize_stdio()) {
      thread::tracker thrtrk;
      /* Assume for now that main() will return without exceptions, in which case
      abc::app_exit_interruption will be thrown in any coroutine/thread still running. */
      exception::common_type xct = exception::common_type::app_exit_interruption;
      try {
         thrtrk.main_thread_started();
         iRet = pfnInstantiateAppAndCallMain(pargs);
      } catch (std::exception const & x) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         iRet = 123;
         xct = exception::execution_interruption_to_common_type(&x);
      } catch (...) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         iRet = 123;
         xct = exception::execution_interruption_to_common_type();
      }
      thrtrk.main_thread_terminated(xct);
      if (!deinitialize_stdio()) {
         iRet = 124;
      }
   } else {
      iRet = 122;
   }
   return iRet;
}

} //namespace abc

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/exception.hxx>
#include <lofty/io.hxx>
#include <lofty/io/text.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/_std/exception.hxx>
#include <lofty/text/str.hxx>
#include "_pvt/signal_dispatcher.hxx"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*static*/ app * app::this_instance = nullptr;

app::app() {
   /* Asserting here is okay because if the assertion is true nothing will happen, and if it’s not that means
   that there’s already an app instance with all the infrastructure needed to handle a failed assertion. */
   LOFTY_ASSERT(!this_instance, LOFTY_SL("multiple instantiation of lofty::app singleton class"));
   this_instance = this;
}

/*virtual*/ app::~app() {
   this_instance = nullptr;
}

/*static*/ int app::call_main(app * app_ptr, _args_t * args) {
   collections::vector<text::str, 8> args_vec;
// TODO: find a way to define LOFTY_HOST_API_WIN32_GUI, and maybe come up with a better name.
#if LOFTY_HOST_API_WIN32 && defined(LOFTY_HOST_API_WIN32_GUI)
   // TODO: call ::GetCommandLine() and parse its result.
#else
   args_vec.set_capacity(static_cast<std::size_t>(args->size), false);
   // Make each string not allocate a new character array.
   for (int i = 0; i < args->size; ++i) {
      args_vec.push_back(text::str(external_buffer, args->values[i]));
   }
#endif

   // Invoke the program-defined main().
   // TODO: find a way to avoid this (mis)use of vector0_ptr().
   return app_ptr->main(*args_vec.vector0_ptr());
}

/*static*/ bool app::deinitialize_stdio() {
   bool errors = false;
   io::text::stdin.reset();
   io::binary::stdin.reset();
   if (auto stdout = dynamic_cast<io::closeable *>(io::text::stdout.get())) {
      try {
         stdout->close();
      } catch (_std::exception const & x) {
         if (io::text::stderr) {
            try {
               exception::write_with_scope_trace(nullptr, &x);
            } catch (...) {
               // FIXME: EXC-SWALLOW
            }
         }
         errors = true;
      } catch (...) {
         if (io::text::stderr) {
            try {
               exception::write_with_scope_trace();
            } catch (...) {
               // FIXME: EXC-SWALLOW
            }
         }
         errors = true;
      }
   }
   io::text::stdout.reset();
   if (auto stdout = dynamic_cast<io::closeable *>(io::binary::stdout.get())) {
      try {
         stdout->close();
      } catch (_std::exception const & x) {
         if (io::text::stderr) {
            try {
               exception::write_with_scope_trace(nullptr, &x);
            } catch (...) {
               // FIXME: EXC-SWALLOW
            }
         }
         errors = true;
      } catch (...) {
         if (io::text::stderr) {
            try {
               exception::write_with_scope_trace();
            } catch (...) {
               // FIXME: EXC-SWALLOW
            }
         }
         errors = true;
      }
   }
   io::binary::stdout.reset();
   if (auto stderr = dynamic_cast<io::closeable *>(io::text::stderr.get())) {
      try {
         stderr->close();
      } catch (...) {
         // FIXME: EXC-SWALLOW
         errors = true;
      }
   }
   io::text::stderr.reset();
   if (auto stderr = dynamic_cast<io::closeable *>(io::binary::stderr.get())) {
      try {
         stderr->close();
      } catch (...) {
         // FIXME: EXC-SWALLOW
         errors = true;
      }
   }
   io::binary::stderr.reset();
   return !errors;
}

/*static*/ bool app::initialize_stdio() {
   try {
      io::binary::stderr = io::binary::_pvt::make_stderr();
      io::binary::stdin  = io::binary::_pvt::make_stdin ();
      io::binary::stdout = io::binary::_pvt::make_stdout();
      io::text::stderr = io::text::_pvt::make_stderr();
      io::text::stdin  = io::text::_pvt::make_stdin ();
      io::text::stdout = io::text::_pvt::make_stdout();
      return true;
   } catch (_std::exception const &) {
      // Exceptions can’t be reported at this point.
      return false;
   } catch (...) {
      // Exceptions can’t be reported at this point.
      return false;
   }
}

/*static*/ int app::run(int (* instantiate_app_and_call_main_fn)(_args_t *), _args_t * args) {
   // Instantiate these as early as possible.
   _pvt::thread_local_storage tls;
   _pvt::signal_dispatcher sig_disp;

   int ret;
   if (initialize_stdio()) {
      /* Assume for now that main() will return without exceptions, in which case lofty::process_exit will be
      thrown in any coroutine/thread still running. */
      exception::common_type caught_x_type = exception::common_type::process_exit;
      try {
         sig_disp.main_thread_started();
         ret = instantiate_app_and_call_main_fn(args);
      } catch (_std::exception const & x) {
         try {
            exception::write_with_scope_trace(nullptr, &x);
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         ret = 123;
         caught_x_type = exception::execution_interruption_to_common_type(&x);
      } catch (...) {
         try {
            exception::write_with_scope_trace();
         } catch (...) {
            // FIXME: EXC-SWALLOW
         }
         ret = 123;
         caught_x_type = exception::execution_interruption_to_common_type();
      }
      sig_disp.main_thread_terminated(caught_x_type);
      if (!deinitialize_stdio()) {
         ret = 124;
      }
   } else {
      ret = 122;
   }
   return ret;
}

} //namespace lofty

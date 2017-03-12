/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Classes and macros to enable application startup. */

#ifndef _LOFTY_APP_HXX
#define _LOFTY_APP_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Base for application implementation classes.

Programs using Lofty don’t declare a C-style ::main() function; instead they override lofty::app::main() in an
application-specific derived class, and announce that class to Lofty using LOFTY_APP_CLASS(). */
class LOFTY_SYM app : public noncopyable {
public:
   /*! @cond
   Collects the OS-provided arguments to a program’s entry point (e.g. main()). */
   struct _args_t {
// TODO: find a way to define LOFTY_HOST_API_WIN32_GUI, and maybe come up with a better name.
#if LOFTY_HOST_API_WIN32 && defined(LOFTY_HOST_API_WIN32_GUI)
      ::HINSTANCE hinst;
      int show_cmd;
#else
      int size;
      char_t ** values;
#endif
   };
   //! @endcond

public:
   //! Destructor.
   virtual ~app();

   /*! @cond
   Instantiates an app subclass and calls its app::main() override.

   @param args
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   template <class TApp>
   static int instantiate_app_and_call_main(_args_t * args) {
      // Create and initialize the app.
      TApp app;
      return call_main(&app, args);
   }
   //! @endcond

   /*! Entry point of the application.

   @param args
      Command-line arguments.
   @return
      Return code of the program.
   */
   virtual int main(collections::vector<str> & args) = 0;

   /*! @cond
   Runs the application, instantiating an app subclass and calling app::main().

   @param args
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   static int run(int (* instantiate_app_and_call_main_fn)(_args_t *), _args_t * args);
   //! @endcond

protected:
   //! Constructor.
   app();

private:
   /*! Invokes app::main() on the specified app subclass instance.

   @param app_ptr
      this.
   @param args
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   static int call_main(app * app_ptr, _args_t * args);

   static bool deinitialize_stdio();

   static bool initialize_stdio();

private:
   //! Pointer to the one and only instance of the application-defined app class.
   static app * this_instance;
};

} //namespace lofty


/*! Declares a lofty::app-derived class as being the app class for the application.

This defines the actual entry point of the program, using whatever protocol is supported by the host (e.g.
int main(…) on POSIX, BOOL WinMain(…) on Windows GUI). This is a very thin wrapper around static methods in
lofty::app which take care of setting up the outermost try/catch block to intercept uncaught exceptions (see
@ref stack-tracing), as well as instantiating the application-defined lofty::app-derived class, invoking its
overridden main() method, and returning.

@param cls
   Main lofty::app-derived class.
*/
#if LOFTY_HOST_API_POSIX
   #define LOFTY_APP_CLASS(cls) \
      extern "C" int main(int argc, char ** argv) { \
         ::lofty::app::_args_t args = { argc, argv }; \
         return ::lofty::app::run(&::lofty::app::instantiate_app_and_call_main<cls>, &args); \
      }
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   // TODO: find a way to define LOFTY_HOST_API_WIN32_GUI, and maybe come up with a better name.
   #ifdef LOFTY_HOST_API_WIN32_GUI
      #define LOFTY_APP_CLASS(cls) \
         extern "C" int WINAPI wWinMain(::HINSTANCE hinst, ::HINSTANCE, wchar_t * cmd_line, int show_cmd) { \
            LOFTY_UNUSED_ARG(cmd_line); \
            ::lofty::app::_args_t args = { hinst, show_cmd }; \
            return ::lofty::app::run(&::lofty::app::instantiate_app_and_call_main<cls>, &args); \
         }
   #else
      #define LOFTY_APP_CLASS(cls) \
         extern "C" int LOFTY_STL_CALLCONV wmain(int argc, wchar_t ** argv) { \
            ::lofty::app::_args_t args = { argc, argv }; \
            return ::lofty::app::run(&::lofty::app::instantiate_app_and_call_main<cls>, &args); \
         }
   #endif
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_APP_HXX

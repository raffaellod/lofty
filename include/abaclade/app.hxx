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

#ifndef _ABACLADE_APP_HXX
#define _ABACLADE_APP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

/*! @file
Classes and macros to enable application startup.
*/


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Base for application implementation classes.

Programs using Abaclade don’t declare a C-style ::main() function; instead they override
abc::app::main() in an application-specific derived class, and announce that class to Abaclade using
ABC_APP_CLASS(). */
class ABACLADE_SYM app : public noncopyable {
public:
   /*! @cond
   Collects the OS-provided arguments to a program’s entry point (e.g. main()). */
   struct _args_t {
// TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
#if ABC_HOST_API_WIN32 && defined(ABC_HOST_API_WIN32_GUI)
      HINSTANCE hinst;
      int iShowCmd;
#else
      int cArgs;
      char_t ** ppszArgs;
#endif
   };
   //! @endcond

public:
   //! Destructor.
   virtual ~app();

   /*! @cond
   Instantiates an app subclass and calls its app::main() override.

   @param pargs
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   template <class TApp>
   static int instantiate_app_and_call_main(_args_t * pargs) {
      // Create and initialize the app.
      TApp app;
      return call_main(&app, pargs);
   }
   //! @endcond

   /*! Entry point of the application.

   @param vsArgs
      Command-line arguments.
   @return
      Return code of the program.
   */
   virtual int main(collections::mvector<istr> & vsArgs) = 0;

   /*! @cond
   Runs the application, instantiating an app subclass and calling app::main().

   @param pargs
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   static int run(int (* pfnInstantiateAppAndCallMain)(_args_t *), _args_t * pargs);
   //! @endcond

protected:
   //! Constructor.
   app();

private:
   /*! Invokes app::main() on the specified app subclass instance.

   @param pargs
      Pointer to the entry point arguments.
   @return
      Return code of the program.
   */
   static int call_main(app * papp, _args_t * pargs);

   static bool deinitialize_stdio();

   static bool initialize_stdio();

private:
   //! Pointer to the one and only instance of the application-defined app class.
   static app * sm_papp;
};

} //namespace abc


/*! Declares an abc::app-derived class as being the app class for the application.

This defines the actual entry point of the program, using whatever protocol is supported by the host
(e.g. int main(…) on POSIX, BOOL WinMain(…) on Windows GUI). This is a very thin wrapper around
static methods in abc::app which take care of setting up the outermost try/catch block to intercept
uncaught exceptions (see @ref stack_tracing), as well as instantiating the application-defined
abc::app-derived class, invoking its overridden main() method, and returning.

@param cls
   Main abc::app-derived class.
*/
#if ABC_HOST_API_POSIX
   #define ABC_APP_CLASS(cls) \
      extern "C" int main(int cArgs, char ** ppszArgs) { \
         ::abc::app::_args_t args = { cArgs, ppszArgs }; \
         return ::abc::app::run(&::abc::app::instantiate_app_and_call_main<cls>, &args); \
      }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   // TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
   #ifdef ABC_HOST_API_WIN32_GUI
      #define ABC_APP_CLASS(cls) \
         extern "C" int WINAPI wWinMain( \
            HINSTANCE hinst, HINSTANCE, wchar_t * pszCmdLine, int iShowCmd \
         ) { \
            ABC_UNUSED_ARG(pszCmdLine); \
            ::abc::app::_args_t args = { hinst, iShowCmd }; \
            return ::abc::app::run(&::abc::app::instantiate_app_and_call_main<cls>, &args); \
         }
   #else
      #define ABC_APP_CLASS(cls) \
         extern "C" int ABC_STL_CALLCONV wmain(int cArgs, wchar_t ** ppszArgs) { \
            ::abc::app::_args_t args = { cArgs, ppszArgs }; \
            return ::abc::app::run(&::abc::app::instantiate_app_and_call_main<cls>, &args); \
         }
   #endif
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_APP_HXX

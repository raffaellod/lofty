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

#ifndef _ABACLADE_APP_HXX
#define _ABACLADE_APP_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::app


/*! DOC:1063 Application startup and abc::app

Programs using Abaclade declare their main() function by deriving a class from abc::app, overriding
its main() method, and declaring that the derived class contains the program’s entry point using
ABC_APP_CLASS().

The ABC_APP_CLASS() macro defines the actual entry point of the program, using whatever
protocol is supported by the host (e.g. int main(…) on POSIX, BOOL WinMain(…) on Windows GUI). This
is a very thin wrapper around a static method of abc::app which takes care of setting up the
outermost try/catch block to intercept uncaught exceptions (see [DOC:8503 Stack tracing]), as well
as instantiating the application-defined abc::app-derived class, invoking its main() method and
returning.
*/

namespace abc {

/*! Abstract application.
*/
class ABACLADE_SYM app :
   public noncopyable {
public:

   /*! Constructor.
   */
   app();


   /*! Destructor.
   */
   virtual ~app();


   /*! C-style entry point for executables.

   cArgs
      Count of arguments.
   ppszArgs
      Arguments.
   return
      Return code of the program.
   */
   template <class TApp>
   static int entry_point_main(int cArgs, char_t ** ppszArgs) {
      // Establish this as early as possible.
      exception::async_handler_manager eahm;
      try {
         // Create and initialize the app.
         TApp app;
         // Use a smvector to avoid dynamic allocation for just a few arguments.
         smvector<istr const, 8> vsArgs;
         app._build_args(cArgs, ppszArgs, &vsArgs);
         // Invoke the program-defined main().
         return app.main(vsArgs);
      } catch (std::exception const & x) {
         exception::write_with_scope_trace(nullptr, &x);
         return 123;
      } catch (...) {
         exception::write_with_scope_trace();
         return 123;
      }
   }


#if ABC_HOST_API_WIN32

   /*! Entry point for Windows executables.

   hinst
      Module’s instance handle.
   iShowCmd
      Indication on how the application’s main window should be displayed; one of SW_* flags.
   */
   template <class TApp>
   static int entry_point_win_exe(HINSTANCE hinst, int iShowCmd) {
      ABC_UNUSED_ARG(iShowCmd);

      // Establish this as early as possible.
      exception::async_handler_manager eahm;
      try {
         // Create and initialize the app.
         TApp app;
         smvector<istr const, 8> vsArgs;
//       app._build_args(&vsArgs);
         // Invoke the program-defined main().
         return app.main(vsArgs);
      } catch (std::exception const & x) {
         exception::write_with_scope_trace(nullptr, &x);
         return 123;
      } catch (...) {
         exception::write_with_scope_trace();
         return 123;
      }
   }

#endif //if ABC_HOST_API_WIN32


   /*! Entry point of the application.

   vsArgs
      Command-line arguments.
   return
      Return code of the program.
   */
   virtual int main(mvector<istr const> const & vsArgs) = 0;


protected:

   /*! Fills up a string vector from the command-line arguments.

   cArgs
      Number of arguments.
   ppszArgs
      Arguments.
   pvsRet
      Vector to receive unsafe istr instances containing each argument.
   */
   static void _build_args(int cArgs, char_t ** ppszArgs, mvector<istr const> * pvsRet);
#if ABC_HOST_API_WIN32
   // Overload that uses ::GetCommandLine() internally.
   static void _build_args(mvector<istr const> * pvsRet);
#endif


protected:

   /*! Pointer to the one and only instance of the application-defined app class. */
   static app * sm_papp;
};

} //namespace abc


/*! Declares an abc::app-derived class as being the app class for the application.

cls
   Main abc::app-derived class.
*/
#if ABC_HOST_API_POSIX
   #define ABC_APP_CLASS(cls) \
      extern "C" int main(int cArgs, char ** ppszArgs) { \
         return ::abc::app::entry_point_main<cls>(cArgs, ppszArgs); \
      }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   // TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
   #ifdef ABC_HOST_API_WIN32_GUI
      #define ABC_APP_CLASS(cls) \
         extern "C" int WINAPI wWinMain( \
            HINSTANCE hinst, HINSTANCE, wchar_t * pszCmdLine, int iShowCmd \
         ) { \
            ABC_UNUSED_ARG(pszCmdLine); \
            return ::abc::app::entry_point_win_exe<cls>(hinst, iShowCmd); \
         }
   #else
      #define ABC_APP_CLASS(cls) \
         extern "C" int ABC_STL_CALLCONV wmain(int cArgs, wchar_t ** ppszArgs) { \
            return ::abc::app::entry_point_main<cls>(cArgs, ppszArgs); \
         }
   #endif
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: OUTPUT
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABACLADE_APP_HXX


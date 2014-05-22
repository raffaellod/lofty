/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

/** DOC:0101 Hello World example

This is a basic usage example of ABC; all it does is display the canonical “Hello World” text, then
it terminates. See the source code for line-by-line comments.
*/

// This should always be the first file included in any C++ source using Abc.
#include <abc/core.hxx>
// This needs to be included in the .cxx file that defines the application module class for the
// program – see below.
#include <abc/module.hxx>
// This provides abc::io::text::stdout() and stdin(), which allow basic text interactivity for the
// program.
#include <abc/io/text/file.hxx>
// ABC does not use “using” directives in its sources or header files; if you want such convenience,
// you have to write it yourself in your own source files.
using namespace abc;



////////////////////////////////////////////////////////////////////////////////////////////////////
// example_app_module


/** This is a basic application module class. The ABC_MAIN_APP_MODULE() statement (below) indicates
that this class shall be instantiated as soon as the program is started, and its main() method
should be invoked immediately afterwards.
*/
class example_app_module :
   public app_module_impl<example_app_module> {
public:

   /** This method is invoked when the program starts; returning from this method causes the end of
   the program.

   vsArgs
      This vector contains all the arguments that were provided to the program via command line.
   return
      The return value of this method will be available to the parent process as this program’s
      return value, accessible in a shell/command prompt as $? (Linux/POSIX) or %ERRORLEVEL%
      (Win32).
   */
   int main(mvector<istr const> const & vsArgs) {
      ABC_TRACE_FN((this/*, vsArgs*/));

      // This helps avoid compiler warnings for arguments/variables that are declared, but not used.
      ABC_UNUSED_ARG(vsArgs);

      // Write “Hello World” into the stdout text writer object.
      io::text::stdout()->write(SL("Hello World"));

      // Make this program return 0 to the parent process.
      return 0;
   }
};

ABC_MAIN_APP_MODULE(example_app_module)


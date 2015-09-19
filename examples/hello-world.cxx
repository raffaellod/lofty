/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

/*! @file
Hello World example

This is a basic usage example of Abaclade; all it does is display the canonical “Hello World” text,
then it terminates. See the source code for line-by-line comments. */

// This should always be the first file included in any C++ source using Abaclade.
#include <abaclade.hxx>
/* This needs to be included in the .cxx file that defines the application class for the program –
see below. */
#include <abaclade/app.hxx>

#include <abaclade/io/text.hxx>

/* Abaclade does not use “using namespace” directives in its sources or header files; if you want
such convenience, you have to write it yourself in your own source files. */
using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

/*! This is a basic application class. The ABC_APP_CLASS() statement (below) indicates that this
class shall be instantiated as soon as the program is started, and its main() method should be
invoked immediately afterwards; see abc::app for more information on application startup. */
class hello_world_app : public app {
public:
   /*! This method is invoked when the program starts; returning from this method causes the end of
   the program.

   @param vsArgs
      This vector contains all the arguments that were provided to the program via command line.
   @return
      The return value of this method will be available to the parent process as this program’s
      return value, accessible in a shell/command prompt as $? (Linux/POSIX) or %ERRORLEVEL%
      (Win32).
   */
   virtual int main(collections::vector<str> & vsArgs) override {
      /* This should be the first line of every function/method; it allows to inspect the values of
      the method’s arguments when an exception is raised during the execution of the method.
      Since it requires the type of every argument to be fully defined, in order to trace vsArgs
      we’d need to #include <abaclade/collections/vector.hxx>, which we won’t bother to in this
      example. */
      ABC_TRACE_FUNC(this/*, vsArgs*/);

      ABC_UNUSED_ARG(vsArgs);
      /* Write “Hello World” into the stdout text writer object. ABC_SL() indicates a string literal
      in the platform-specific Unicode Transformation Format (UTF-8 or UTF-16). */
      io::text::stdout->write_line(ABC_SL("Hello World"));

      // Make this program return 0 to the parent process.
      return 0;
   }
};

ABC_APP_CLASS(hello_world_app)

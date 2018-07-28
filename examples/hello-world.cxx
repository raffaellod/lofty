/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Hello World example

This is a basic usage example of Lofty; all it does is display the canonical “Hello World” text, then it
terminates. See the source code for line-by-line comments. */

// This needs to be included in the .cxx file that defines the application class for the program – see below.
#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/io/text.hxx>
// Declares LOFTY_TRACE_*() and LOFTY_LOG().
#include <lofty/logging.hxx>
#include <lofty/text/str.hxx>

/* Lofty does not use “using namespace” directives in its sources or header files; if you want such
convenience, you can write it yourself in your own source files. */
using namespace lofty;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! This is a basic application class. The LOFTY_APP_CLASS() statement (below) indicates that this class shall
be instantiated as soon as the program is started, and its main() method should be invoked immediately
afterwards; see lofty::app for more information on application startup. */
class hello_world_app : public app {
public:
   /*! This method is invoked when the program starts; returning from this method causes the end of the
   program.

   @param args
      This vector contains all the arguments that were provided to the program via command line.
   @return
      The return value of this method will be available to the parent process as this program’s return value,
      accessible in a shell/command prompt as $? (Linux/POSIX) or %ERRORLEVEL% (Win32).
   */
   virtual int main(collections::vector<text::str> & args) override {
      /* This should be the first line of every non-minor method; it allows to generate a stack trace when an
      exception is thrown during the execution of the method. */
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);
      /* Write “Hello World” into the stdout text stream. LOFTY_SL() indicates a string literal in the
      platform-specific Unicode Transformation Format (UTF-8 or UTF-16). */
      io::text::stdout->write_line(LOFTY_SL("Hello World"));

      // Make this program return 0 to the parent process.
      return 0;
   }
};

LOFTY_APP_CLASS(hello_world_app)

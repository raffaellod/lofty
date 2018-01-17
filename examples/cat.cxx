/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
“cat” utility example

This is a basic usage example of Lofty; it shows how to read from standard input one line at a time, echoing
each to standard output. The program terminates when it reaches the end of input, which can be signaled with
Ctrl+D in Linux/macOS/FreeBSD or Ctrl+Z followed by Enter in Windows. */

#include <lofty.hxx>
#include <lofty/app.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>

using namespace lofty;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class cat_app : public app {
public:
   /*! Main function of the program.

   @param args
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<str> & args) override {
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);
      // Read one line at a time from stdin…
      LOFTY_FOR_EACH(auto & line, io::text::stdin->lines()) {
         // …and write it to stdout.
         io::text::stdout->write_line(line);
      }

      return 0;
   }
};

LOFTY_APP_CLASS(cat_app)

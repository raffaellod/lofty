/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

/*! @file
“cat” utility example

This is a basic usage example of Abaclade; it shows how to read from standard input one line at a
time, echoing each to standard output. The program terminates when it reaches the end of input,
which can be signaled with Ctrl+D in Linux/OS X/FreeBSD or Ctrl+Z followed by Enter in Windows. */

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/io/text.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

class cat_app : public app {
public:
   /*! Main function of the program.

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<str> & vsArgs) override {
      ABC_TRACE_FUNC(this/*, vsArgs*/);

      ABC_UNUSED_ARG(vsArgs);
      // Read one line at a time from stdin…
      ABC_FOR_EACH(auto & sLine, io::text::stdin->lines()) {
         // …and write it to stdout.
         io::text::stdout->write_line(sLine);
      }

      return 0;
   }
};

ABC_APP_CLASS(cat_app)

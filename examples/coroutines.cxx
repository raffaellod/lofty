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

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/io/binary.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/range.hxx>
#include <abaclade/thread.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

class coroutines_app : public app {
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
      this_thread::attach_coroutine_scheduler();

      /* Open a pipe. Since this thread now has a coroutine scheduler, the pipe will take advantage
      of it to avoid blocking on reads and writes. */
      auto pe(io::binary::pipe());
      /* Ensure that the pipe’s writing end is finalized (closed) even in case of exceptions. In a
      real application, we would check for exceptions when doing so. */
      auto deferred1(defer_to_scope_end([&pe] () {
         pe.writer->finalize();
      }));

      // Schedule the reader.
      coroutine([this, &pe] () {
         ABC_TRACE_FUNC(this/*, pe*/);

         io::text::stdout->write_line(ABC_SL("reader: starting"));
         for (;;) {
            int i;
            io::text::stdout->print(ABC_SL("reader: reading\n"));
            // This will cause a context switch if the read would block.
            std::size_t cbRead = pe.reader->read(&i, sizeof i);
            // Execution resumes here, after other coroutines have received CPU time.
            if (cbRead == 0) {
               // Detect EOF.
               break;
            }
            io::text::stdout->print(ABC_SL("reader: read {}\n"), i);

            // Consume i.
            if (i == 3) {
               // Add a coroutine that will display a message in a quarter of a second.
               coroutine([] () {
                  io::text::stdout->write_line(ABC_SL("delayed message: starting"));
                  this_coroutine::sleep_for_ms(250);
                  io::text::stdout->write_line(ABC_SL("delayed message: this is it"));
                  io::text::stdout->write_line(ABC_SL("delayed message: terminating"));
               });
            }
         }
         io::text::stdout->write_line(ABC_SL("reader: terminating"));
      });

      // Schedule the writer.
      coroutine([this, &pe] () {
         ABC_TRACE_FUNC(this/*, pe*/);

         io::text::stdout->write_line(ABC_SL("writer: starting"));
         ABC_FOR_EACH(int i, make_range(1, 10)) {
            io::text::stdout->print(ABC_SL("writer: writing {}\n"), i);
            // This will cause a context switch if the write would block.
            pe.writer->write(&i, sizeof i);
            // Execution resumes here, after other coroutines have received CPU time.

            /* Halt this coroutine for a few milliseconds. This will give the reader a chance to be
            scheduled, as well as create a more realistic non-continuous data flow into the pipe. */
            io::text::stdout->write_line(ABC_SL("writer: yielding"));
            this_coroutine::sleep_for_ms(50);
            // Execution resumes here, after other coroutines have received CPU time.
         }
         // Close the writing end of the pipe to report EOF on the reading end.
         pe.writer->finalize();
         io::text::stdout->write_line(ABC_SL("writer: terminating"));
      });

      // Schedule the stdin reader.
      /*coroutine([this] () {
         ABC_TRACE_FUNC(this);

         io::text::stdout->print(ABC_SL("stdin: starting\n"));
         ABC_FOR_EACH(auto & sLine, io::text::stdin->lines()) {
            io::text::stdout->print(ABC_SL("stdin: read {}\n"), sLine);
         }
         io::text::stdout->write_line(ABC_SL("stdin: terminating"));
      });*/

      // Switch this thread to run coroutines, until they all terminate.
      this_thread::run_coroutines();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(ABC_SL("main: terminating"));
      return 0;
   }
};

ABC_APP_CLASS(coroutines_app)

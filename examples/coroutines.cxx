/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/app.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/binary.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/range.hxx>
#include <lofty/thread.hxx>

using namespace lofty;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class coroutines_app : public app {
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
      this_thread::attach_coroutine_scheduler();

      /* Create a pipe. Since this thread now has a coroutine scheduler, the pipe will take advantage of it to
      avoid blocking on reads and writes. */
      io::binary::pipe pipe;

      // Schedule the reader.
      coroutine([this, &pipe] () {
         LOFTY_TRACE_FUNC();

         io::text::stdout->write_line(LOFTY_SL("reader: starting"));
         for (;;) {
            int i;
            io::text::stdout->print(LOFTY_SL("reader: reading\n"));
            // This will cause a context switch if the read would block.
            std::size_t bytes_read = pipe.read_end->read(&i);
            // Execution resumes here, after other coroutines have received CPU time.
            if (bytes_read == 0) {
               // Detect EOF.
               break;
            }
            io::text::stdout->print(LOFTY_SL("reader: read {}\n"), i);

            // Consume i.
            if (i == 3) {
               // Add a coroutine that will display a message in a quarter of a second.
               coroutine([] () {
                  io::text::stdout->write_line(LOFTY_SL("delayed message: starting"));
                  this_coroutine::sleep_for_ms(250);
                  io::text::stdout->write_line(LOFTY_SL("delayed message: this is it"));
                  io::text::stdout->write_line(LOFTY_SL("delayed message: terminating"));
               });
            }
         }
         io::text::stdout->write_line(LOFTY_SL("reader: terminating"));
      });

      // Schedule the writer.
      coroutine([this, &pipe] () {
         LOFTY_TRACE_FUNC();

         /* Ensure that the pipe’s write end is finalized (closed) even in case of exceptions. In a real
         application, we would check for exceptions when doing so. This will be reported as EOF on the read
         end. */
#if LOFTY_HOST_CXX_MSC == 1600
         // MSC16 BUG: does not like capturing a captured variable.
         auto pipe_write_end(pipe.write_end);
         LOFTY_DEFER_TO_SCOPE_END(pipe_write_end->finalize());
#else
         LOFTY_DEFER_TO_SCOPE_END(pipe.write_end->finalize());
#endif

         io::text::stdout->write_line(LOFTY_SL("writer: starting"));
         LOFTY_FOR_EACH(int i, make_range(1, 10)) {
            io::text::stdout->print(LOFTY_SL("writer: writing {}\n"), i);
            // This will cause a context switch if the write would block.
            pipe.write_end->write(i);
            // Execution resumes here, after other coroutines have received CPU time.

            /* Halt this coroutine for a few milliseconds. This will give the reader a chance to be scheduled,
            as well as create a more realistic non-continuous data flow into the pipe. */
            io::text::stdout->write_line(LOFTY_SL("writer: yielding"));
            this_coroutine::sleep_for_ms(50);
            // Execution resumes here, after other coroutines have received CPU time.
         }
         io::text::stdout->write_line(LOFTY_SL("writer: terminating"));
      });

      // Schedule the stdin reader.
      /*coroutine([this] () {
         LOFTY_TRACE_FUNC();

         io::text::stdout->print(LOFTY_SL("stdin: starting\n"));
         LOFTY_FOR_EACH(auto & line, io::text::stdin->lines()) {
            io::text::stdout->print(LOFTY_SL("stdin: read {}\n"), line);
         }
         io::text::stdout->write_line(LOFTY_SL("stdin: terminating"));
      });*/

      // Switch this thread to run coroutines, until they all terminate.
      this_thread::run_coroutines();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(LOFTY_SL("main: terminating"));
      return 0;
   }
};

LOFTY_APP_CLASS(coroutines_app)

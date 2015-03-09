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
#include <abaclade/range.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////
// coroutines_app

class coroutines_app : public app {
public:
   /*! Main function of the program.

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::mvector<istr const> const & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      coroutine_scheduler & corosched = coroutine_scheduler::attach_to_current_thread();

      // Open a pipe in asynchronous I/O mode.
      auto pair(io::binary::pipe(true));
      // Schedule the reader.
      corosched.add_coroutine([this, &pair] () -> void {
         ABC_TRACE_FUNC(this/*, pair*/);

         for (;;) {
            // This will cause a context switch if the read would block.
            int i;
            io::text::stdout()->print(ABC_SL("reading\n"));
            std::size_t cbRead = pair.first->read(&i, sizeof i);
            io::text::stdout()->print(ABC_SL("read {}\n"), i);
            // Execution resumes here, after other coroutines have received CPU time.
            if (!cbRead) {
               // Detect EOF.
               break;
            }
            // Consume i.
         }
      });
      // Schedule the writer.
      corosched.add_coroutine([this, &pair] () -> void {
         ABC_TRACE_FUNC(this/*, pair*/);

         ABC_FOR_EACH(int i, make_range(1, 10)) {
            io::text::stdout()->print(ABC_SL("writing {}\n"), i);
            // This will cause a context switch if the write would block.
            pair.second->write(&i, sizeof i);
            // Execution resumes here, after other coroutines have received CPU time.
         }
         // Close the writing end of the pipe to report EOF on the reading end.
         pair.second.reset();
      });
      // Switch this thread to run coroutines, until they all terminate.
      corosched.run();
      return 0;
   }
};

ABC_APP_CLASS(coroutines_app)

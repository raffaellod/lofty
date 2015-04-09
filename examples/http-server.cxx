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
#include <abaclade/net.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////
// http_server_app

class http_server_app : public app {
public:
   /*! Main function of the program.

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::mvector<istr> & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      auto & pcorosched = this_thread::attach_coroutine_scheduler();

      // Schedule a TCP server.
      pcorosched->add(coroutine([this] () -> void {
         ABC_TRACE_FUNC(this);

         io::text::stdout->print(ABC_SL("server: starting\n"));
         net::tcp_server server(ABC_SL("*"), 9082);
         for (;;) {
            io::text::stdout->print(ABC_SL("server: accepting\n"));
            auto pconn(server.accept());
            // Add a coroutine that will echo every byte sent over the newly-established connection.
            this_thread::get_coroutine_scheduler()->add(coroutine([this, pconn] () -> void {
               ABC_TRACE_FUNC(this, pconn);

               // Create text-mode reader and writer for the connection’s socket.
               auto ptbbr(io::text::make_reader(pconn->socket()));
               auto ptbbw(io::text::make_writer(pconn->socket()));
               io::text::stdout->write_line(ABC_SL("responder: reading request"));
               ABC_FOR_EACH(auto & sLine, ptbbr->lines()) {
                  if (!sLine) {
                     // The request ends on the first empty line.
                     break;
                  }
               }
               io::text::stdout->write_line(ABC_SL("responder: responding"));

               // Send the response headers.
               ptbbw->write_line("HTTP/1.0 200 OK");
               ptbbw->write_line("Content-Type: text/plain; charset=utf-8");
               ptbbw->write_line("Content-Length: 2");
               ptbbw->write_line();
               ptbbw->flush();

               // Send the response content.
               ptbbw->write("OK");
               io::text::stdout->write_line(ABC_SL("responder: terminating"));
            }));
         }
         io::text::stdout->write_line(ABC_SL("server: terminating"));
      }));

      // Switch this thread to run coroutines, until they all terminate.
      pcorosched->run();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(ABC_SL("main: terminating"));
      return 0;
   }
};

ABC_APP_CLASS(http_server_app)

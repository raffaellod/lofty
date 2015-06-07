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
#include <abaclade/thread.hxx>

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

      // Schedule a TCP server.
      coroutine([this] () {
         ABC_TRACE_FUNC(this);

         static net::port_t const sc_port = 9080;
         io::text::stdout->print(ABC_SL("server: starting\n"));
         net::tcp_server server(net::ip_address::any_ipv4, sc_port);
         for (;;) {
            io::text::stdout->print(ABC_SL("server: accepting\n"));
            auto pconn(server.accept());
            // Add a coroutine that will echo every byte sent over the newly-established connection.
            coroutine([this, pconn] () {
               ABC_TRACE_FUNC(this, pconn);

               // Create text-mode reader and writer for the connection’s socket.
               auto ptr(io::text::make_reader(pconn->socket()));
               auto ptw(io::text::make_writer(pconn->socket()));
               io::text::stdout->write_line(ABC_SL("responder: reading request"));
               ABC_FOR_EACH(auto & sLine, ptr->lines()) {
                  if (!sLine) {
                     // The request ends on the first empty line.
                     break;
                  }
               }
               io::text::stdout->write_line(ABC_SL("responder: responding"));

               // Send the response headers.
               ptw->write_line("HTTP/1.0 200 OK");
               ptw->write_line("Content-Type: text/plain; charset=utf-8");
               ptw->write_line("Content-Length: 2");
               ptw->write_line();
               ptw->flush();

               // Send the response content.
               ptw->write("OK");
               io::text::stdout->write_line(ABC_SL("responder: terminating"));

               ptw->finalize();
            });
         }
         io::text::stdout->write_line(ABC_SL("server: terminating"));
      });

      // Switch this thread to run coroutines, until they all terminate.
      this_thread::run_coroutines();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(ABC_SL("main: terminating"));
      return 0;
   }
};

ABC_APP_CLASS(http_server_app)

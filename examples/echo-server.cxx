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
#include <abaclade/thread.hxx>
#include <abaclade/net.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

class echo_server_app : public app {
public:
   /*! Main function of the program.

   @param vsArgs
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<str> & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      ABC_UNUSED_ARG(vsArgs);
      // Schedule a TCP server. To connect to it, use: socat - TCP4:127.0.0.1:9082
      coroutine([this] () {
         ABC_TRACE_FUNC(this);

         static net::port_t const sc_port = 9082;
         io::text::stdout->print(ABC_SL("server: starting, listening on port {}\n"), sc_port);
         net::tcp_server server(net::ip_address::any_ipv4, sc_port);
         try {
            for (;;) {
               io::text::stdout->write_line(ABC_SL("server: accepting"));
               // This will cause a context switch if no connections are ready to be established.
               auto pconn(server.accept());

               io::text::stdout->write_line(ABC_SL("server: connection established"));

               /* Add a coroutine that will echo every line sent over the newly-established
               connection. */
               coroutine([pconn] () {
                  ABC_TRACE_FUNC(pconn);

                  io::text::stdout->write_line(ABC_SL("responder: starting"));

                  // Create text-mode reader and writer for the connection’s socket.
                  auto ptr(io::text::make_reader(pconn->socket()));
                  auto ptw(io::text::make_writer(pconn->socket()));
                  auto deferred1(defer_to_scope_end([&ptw] () {
                     ptw->finalize();
                  }));

                  // Read lines from the socket, writing them back to it (echo).
                  ABC_FOR_EACH(auto & sLine, ptr->lines()) {
                     ptw->write_line(sLine);
                     ptw->flush();
                  }

                  io::text::stdout->write_line(ABC_SL("responder: terminating"));
                  // deferred1 will finalize *ptw.
               });
            }
         } catch (execution_interruption const &) {
            io::text::stdout->write_line(ABC_SL("server: terminating"));
            // Rethrow the exception to ensure that all remaining coroutines are terminated.
            throw;
         }
      });

      // Switch this thread to run coroutines, until they all terminate.
      this_thread::run_coroutines();
      // Execution resumes here, after all coroutines have terminated.
      io::text::stdout->write_line(ABC_SL("main: terminating"));
      return 0;
   }
};

ABC_APP_CLASS(echo_server_app)

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

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

#include <abaclade.hxx>
#include <abaclade/app.hxx>
#include <abaclade/coroutine.hxx>
#include <abaclade/defer_to_scope_end.hxx>
#include <abaclade/io/text.hxx>
#include <abaclade/net/tcp.hxx>
#include <abaclade/thread.hxx>

using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////

class http_server_app : public app {
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
      // Schedule a TCP server.
      coroutine([this] () {
         ABC_TRACE_FUNC(this);

         net::ip::port port(9080);
         io::text::stdout->print(ABC_SL("server: starting, listening on port {}\n"), port);
         net::tcp::server server(net::ip::address::any_v4, port);
         try {
            for (;;) {
               io::text::stdout->write_line(ABC_SL("server: accepting"));
               // This will cause a context switch if no connections are ready to be established.
               auto pconn(server.accept());

               io::text::stdout->write_line(ABC_SL("server: connection established"));

               // Add a coroutine that will process the newly-established connection.
               coroutine([pconn] () {
                  ABC_TRACE_FUNC(pconn);

                  io::text::stdout->print(
                     ABC_SL("responder: handling request from {}:{}\n"),
                     pconn->remote_address(), pconn->remote_port()
                  );
                  // Create text-mode input and output streams for the connection’s socket.
                  auto ptis(io::text::make_istream(pconn->socket()));
                  auto ptos(io::text::make_ostream(pconn->socket(), text::encoding::utf8));
                  ABC_DEFER_TO_SCOPE_END(ptos->finalize());
                  io::text::stdout->write_line(ABC_SL("responder: reading request"));
                  ABC_FOR_EACH(auto & sLine, ptis->lines()) {
                     if (!sLine) {
                        // The request ends on the first empty line.
                        break;
                     }
                  }
                  io::text::stdout->write_line(ABC_SL("responder: responding"));

                  // Send the response headers.
                  ptos->write_line(ABC_SL("HTTP/1.0 200 OK"));
                  ptos->write_line(ABC_SL("Content-Type: text/plain; charset=utf-8"));
                  ptos->write_line(ABC_SL("Content-Length: 2"));
                  ptos->write_line();
                  ptos->flush();

                  // Send the response content.
                  ptos->write(ABC_SL("OK"));

                  io::text::stdout->write_line(ABC_SL("responder: terminating"));
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

ABC_APP_CLASS(http_server_app)

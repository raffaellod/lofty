/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/coroutine.hxx>
#include <lofty/exception.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/net/tcp.hxx>
#include <lofty/text/str.hxx>
#include <lofty/thread.hxx>
#include <lofty/try_finally.hxx>

using namespace lofty;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class echo_server_app : public app {
public:
   /*! Main function of the program.

   @param args
      Arguments that were provided to this program via command line.
   @return
      Return value of this program.
   */
   virtual int main(collections::vector<text::str> & args) override {
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);
      // Schedule a TCP server. To connect to it, use: socat - TCP4:127.0.0.1:9082
      coroutine([this] () {
         LOFTY_TRACE_FUNC();

         net::ip::port port(9082);
         LOFTY_LOG(info, LOFTY_SL("server: starting, listening on port {}\n"), port);
         net::tcp::server server(net::ip::address::any_v4, port);
         try {
            for (;;) {
               LOFTY_LOG(info, LOFTY_SL("server: accepting\n"));
               // This will cause a context switch if no connections are ready to be established.
               auto conn(server.accept());

               LOFTY_LOG(info, LOFTY_SL("server: connection established\n"));

               // Add a coroutine that will echo every line sent over the newly-established connection.
               coroutine([conn] () {
                  LOFTY_TRACE_FUNC();

                  LOFTY_TRY {
                     LOFTY_LOG(
                        info, LOFTY_SL("responder: starting for {}:{}\n"),
                        conn->remote_address(), conn->remote_port()
                     );

                     // Create text-mode input and output streams for the connection’s socket.
                     auto socket_istream(io::text::make_istream(conn->socket()));
                     auto socket_ostream(io::text::make_ostream(conn->socket()));
                     LOFTY_TRY {
                        // Read lines from the socket, writing them back to it (echo).
                        LOFTY_FOR_EACH(auto & line, socket_istream->lines()) {
                           socket_ostream->write_line(line);
                           socket_ostream->flush();
                        }
                     } LOFTY_FINALLY {
                        socket_ostream->close();
                     };

                     LOFTY_LOG(info, LOFTY_SL("responder: terminating\n"));
                  } LOFTY_FINALLY {
                     conn->socket()->close();
                  };
               });
            }
         } catch (execution_interruption const &) {
            LOFTY_LOG(info, LOFTY_SL("server: terminating\n"));
            // Rethrow the exception to ensure that all remaining coroutines are terminated.
            throw;
         }
      });

      // Switch this thread to run coroutines, until they all terminate.
      this_thread::run_coroutines();
      // Execution resumes here, after all coroutines have terminated.
      LOFTY_LOG(info, LOFTY_SL("main: terminating\n"));
      return 0;
   }
};

LOFTY_APP_CLASS(echo_server_app)

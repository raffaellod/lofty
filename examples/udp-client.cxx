﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/io/binary/memory.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/net/ip.hxx>
#include <lofty/net/udp.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text/str.hxx>
#include <lofty/try_finally.hxx>

using namespace lofty;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

class udp_echo_client_app : public app {
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

      net::udp::client client;
      LOFTY_FOR_EACH(auto const & arg, args) {
         auto dgram_data(_std::make_shared<io::binary::memory_stream>());
         {
            auto dgram_ostream(io::text::make_ostream(dgram_data));
            LOFTY_TRY {
               dgram_ostream->print(LOFTY_SL("{}\n"), arg);
            } LOFTY_FINALLY {
               dgram_ostream->close();
            };
         }
         net::udp::datagram dgram(net::ip::address::localhost_v4, net::ip::port(9081), _std::move(dgram_data));
         LOFTY_LOG(info, LOFTY_SL("client: sending datagram\n"));
         client.send(dgram);
      }

      return 0;
   }
};

LOFTY_APP_CLASS(udp_echo_client_app)

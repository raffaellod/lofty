/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/app.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/io/text.hxx>
#include <lofty/logging.hxx>
#include <lofty/net/udp.hxx>

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
   virtual int main(collections::vector<str> & args) override {
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);

      net::udp::client client;
      LOFTY_FOR_EACH(auto const & arg, args) {
         auto dgram_data(_std::make_shared<io::binary::memory_stream>());
         {
            auto dgram_ostream(io::text::make_ostream(dgram_data));
            LOFTY_DEFER_TO_SCOPE_END(dgram_ostream->finalize());
            dgram_ostream->print(LOFTY_SL("{}\n"), arg);
         }
         net::udp::datagram dgram(net::ip::address::localhost_v4, net::ip::port(9081), _std::move(dgram_data));
         LOFTY_LOG(info, LOFTY_SL("client: sending datagram\n"));
         client.send(dgram);
      }

      return 0;
   }
};

LOFTY_APP_CLASS(udp_echo_client_app)

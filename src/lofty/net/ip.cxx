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
#include <lofty/byte_order.hxx>
#include <lofty/net/ip.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace net { namespace ip {

static_assert(
   sizeof(address::v4_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->bytes, "v4_type is too big"
);
static_assert(
   sizeof(address::v6_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->bytes, "v6_type is too big"
);

static _pvt::raw_address const raw_any_v4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v4
};
static _pvt::raw_address const raw_any_v6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v6
};

address const & address::any_v4 = static_cast<address const &>(raw_any_v4);
address const & address::any_v6 = static_cast<address const &>(raw_any_v6);

}}} //namespace lofty::net::ip

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

to_text_ostream<net::ip::address>::to_text_ostream() {
   v6_group_ttos.set_format(LOFTY_SL("x"));
}

to_text_ostream<net::ip::address>::~to_text_ostream() {
}

void to_text_ostream<net::ip::address>::set_format(str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<net::ip::address>::write(net::ip::address const & src, io::text::ostream * dst) {
   switch (src.version().base()) {
      case net::ip::version::any:
         char_ttos.write('-', dst);
         break;
      case net::ip::version::v4: {
         auto group = src.raw(), groups_end = group + sizeof(net::ip::address::v4_type);
         v4_group_ttos.write(*group, dst);
         while (++group < groups_end) {
            char_ttos.write('.', dst);
            v4_group_ttos.write(*group, dst);
         }
         break;
      }
      case net::ip::version::v6: {
         /* This implementation complies with RFC 4291 “IP Version 6 Addressing Architecture” § 2.2. “Text
         Representation of Addresses”. */

         auto groups_begin = reinterpret_cast<std::uint16_t const *>(src.raw());
         auto groups_end   = groups_begin + sizeof(net::ip::address::v6_type) / sizeof groups_begin[0];

         // Find the longest run of zeroes, so we can print “::” instead.
         auto curr_0s_begin = groups_begin, max_0s_begin = groups_begin;
         auto curr_0s_end   = groups_begin, max_0s_end   = groups_begin;
         for (auto group = groups_begin; group < groups_end; ++group) {
            if (*group == 0) {
               if (group != curr_0s_end) {
                  // Start a new “current 0s” range.
                  curr_0s_begin = group;
               }
               // Include this group in the “current 0s” range.
               curr_0s_end = group + 1;
            } else if (group == curr_0s_end) {
               // End of the “current 0s” range; save it as “max 0s” if it’s the longest.
               if (curr_0s_end - curr_0s_begin > max_0s_end - max_0s_begin) {
                  max_0s_begin = curr_0s_begin;
                  max_0s_end = curr_0s_end;
               }
            }
         }
         // Check if we ended on what should become the “max 0s” range.
         if (curr_0s_end - curr_0s_begin > max_0s_end - max_0s_begin) {
            max_0s_begin = curr_0s_begin;
            max_0s_end = curr_0s_end;
         }

         auto group = groups_begin;
         if (max_0s_end == max_0s_begin) {
            // Write the first group, not preceded by “:”.
            v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
         } else {
            if (group < max_0s_begin) {
               // Write all the groups before the “max 0s” range.
               do {
                  v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
                  char_ttos.write(':', dst);
               } while (group < max_0s_begin);
            } else {
               // Print one ”:”; the second will be printed in the loop below.
               char_ttos.write(':', dst);
            }
            if (max_0s_end == groups_end) {
               // No more groups to write; just add a second “:” and skip the second loop.
               char_ttos.write(':', dst);
               break;
            }
            group = max_0s_end;
         }
         // Write all the groups after the “max 0s” range.
         while (group < groups_end) {
            char_ttos.write(':', dst);
            v6_group_ttos.write(byte_order::be_to_host(*group++), dst);
         }
         break;
      }
   }
}

} //namespace lofty

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
#include <abaclade/byte_order.hxx>
#include <abaclade/net/ip.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

void to_text_ostream<net::ip::port>::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_text_ostream<net::ip::port>::write(net::ip::port const & port, io::text::ostream * ptos) {
   ABC_TRACE_FUNC(this/*, port*/, ptos);

   to_text_ostream<net::ip::port::type>::write(port.number(), ptos);
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace net { namespace ip {

static_assert(
   sizeof(address::v4_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->m_ab,
   "v4_type is too big"
);
static_assert(
   sizeof(address::v6_type) <= sizeof reinterpret_cast<_pvt::raw_address *>(8192)->m_ab,
   "v6_type is too big"
);

static _pvt::raw_address const gc_abAny4 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v4
};
static _pvt::raw_address const gc_abAny6 = {
   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, version::v6
};

address const & address::any_v4 = static_cast<address const &>(gc_abAny4);
address const & address::any_v6 = static_cast<address const &>(gc_abAny6);

}}} //namespace abc::net::ip

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

void to_text_ostream<net::ip::address>::set_format(str const & sFormat) {
   ABC_TRACE_FUNC(this, sFormat);

   m_ttosV6Group.set_format(ABC_SL("x"));

   auto it(sFormat.cbegin());

   // Add parsing of the format string here.

   // If we still have any characters, they are garbage.
   if (it != sFormat.cend()) {
      ABC_THROW(syntax_error, (
         ABC_SL("unexpected character"), sFormat, static_cast<unsigned>(it - sFormat.cbegin())
      ));
   }
}

void to_text_ostream<net::ip::address>::write(
   net::ip::address const & addr, io::text::ostream * ptos
) {
   ABC_TRACE_FUNC(this/*, addr*/, ptos);

   switch (addr.version().base()) {
      case net::ip::version::any:
         m_ttosChar.write('-', ptos);
         break;
      case net::ip::version::v4: {
         std::uint8_t const * piGroup = addr.raw();
         std::uint8_t const * piGroupsEnd = addr.raw() + sizeof(net::ip::address::v4_type);
         m_ttosV4Group.write(*piGroup, ptos);
         while (++piGroup < piGroupsEnd) {
            m_ttosChar.write('.', ptos);
            m_ttosV4Group.write(*piGroup, ptos);
         }
         break;
      }
      case net::ip::version::v6: {
         /* This implementation complies with RFC 4291 “IP Version 6 Addressing Architecture” § 2.2.
         “Text Representation of Addresses”. */

         std::uint16_t const * piGroupsBegin = reinterpret_cast<std::uint16_t const *>(addr.raw());
         std::uint16_t const * piGroupsEnd = reinterpret_cast<std::uint16_t const *>(
            addr.raw() + sizeof(net::ip::address::v6_type)
         );

         // Find the longest run of zeroes, so we can print “::” instead.
         std::uint16_t const * piCurr0sBegin = piGroupsBegin, * piCurr0sEnd = piGroupsBegin;
         std::uint16_t const * piMax0sBegin = piGroupsBegin, * piMax0sEnd = piGroupsBegin;
         for (auto piGroup = piGroupsBegin; piGroup < piGroupsEnd; ++piGroup) {
            if (*piGroup == 0) {
               if (piGroup != piCurr0sEnd) {
                  // Start a new “current 0s” range.
                  piCurr0sBegin = piGroup;
               }
               // Include this group in the “current 0s” range.
               piCurr0sEnd = piGroup + 1;
            } else if (piGroup == piCurr0sEnd) {
               // End of the “current 0s” range; save it as “max 0s” if it’s the longest.
               if (piCurr0sEnd - piCurr0sBegin > piMax0sEnd - piMax0sBegin) {
                  piMax0sBegin = piCurr0sBegin;
                  piMax0sEnd = piCurr0sEnd;
               }
            }
         }
         // Check if we ended on what should become the “max 0s” range.
         if (piCurr0sEnd - piCurr0sBegin > piMax0sEnd - piMax0sBegin) {
            piMax0sBegin = piCurr0sBegin;
            piMax0sEnd = piCurr0sEnd;
         }

         auto piGroup = piGroupsBegin;
         if (piMax0sEnd == piMax0sBegin) {
            // Write the first group, not preceded by “:”.
            m_ttosV6Group.write(byte_order::be_to_host(*piGroup++), ptos);
         } else {
            if (piGroup < piMax0sBegin) {
               // Write all the groups before the “max 0s” range.
               do {
                  m_ttosV6Group.write(byte_order::be_to_host(*piGroup++), ptos);
                  m_ttosChar.write(':', ptos);
               } while (piGroup < piMax0sBegin);
            } else {
               // Print one ”:”; the second will be printed in the loop below.
               m_ttosChar.write(':', ptos);
            }
            if (piMax0sEnd == piGroupsEnd) {
               // No more groups to write; just add a second “:” and skip the second loop.
               m_ttosChar.write(':', ptos);
               break;
            }
            piGroup = piMax0sEnd;
         }
         // Write all the groups after the “max 0s” range.
         while (piGroup < piGroupsEnd) {
            m_ttosChar.write(':', ptos);
            m_ttosV6Group.write(byte_order::be_to_host(*piGroup++), ptos);
         }
         break;
      }
   }
}

} //namespace abc

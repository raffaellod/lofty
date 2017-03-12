/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

/*static*/ enum_member const * enum_member::find_in_map(enum_member const * members, int value) {
   LOFTY_TRACE_FUNC(members, value);

   for (; members->name; ++members) {
      if (value == members->value) {
         return members;
      }
   }
   // TODO: provide more information in the exception.
   LOFTY_THROW(domain_error, ());
}
/*static*/ enum_member const * enum_member::find_in_map(enum_member const * members, str const & name) {
   LOFTY_TRACE_FUNC(members, name);

   for (; members->name; ++members) {
      if (name == str(external_buffer, members->name, members->name_size)) {
         return members;
      }
   }
   // TODO: provide more information in the exception.
   LOFTY_THROW(domain_error, ());
}

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

void enum_to_text_ostream_impl::set_format(str const & format) {
   LOFTY_TRACE_FUNC(this, format);

   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void enum_to_text_ostream_impl::write_impl(int i, enum_member const * members, io::text::ostream * dst) {
   LOFTY_TRACE_FUNC(this, i, members, dst);

   auto member = enum_member::find_in_map(members, i);
   dst->write(str(external_buffer, member->name));
}

}} //namespace lofty::_pvt

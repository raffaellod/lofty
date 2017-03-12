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

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

template <class T>
inline str enum_impl<T>::name() const {
   _pvt::enum_member const * member = _member();
   return str(external_buffer, member->name, member->name_size);
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Implementation of the specializations of to_text_ostream for enum_impl specializations.
class LOFTY_SYM enum_to_text_ostream_impl {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

protected:
   /*! Writes an enumeration value, applying the formatting options.

   @param i
      Value of the enumeration member to write.
   @param members
      Pointer to the enumeration members map.
   @param dst
      Pointer to the stream to output to.
   */
   void write_impl(int i, enum_member const * members, io::text::ostream * dst);
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <class T>
class to_text_ostream<enum_impl<T>> : public _pvt::enum_to_text_ostream_impl {
public:
   //! See lofty::_pvt::enum_to_text_ostream_impl::write().
   void write(enum_impl<T> src, io::text::ostream * dst) {
      _pvt::enum_to_text_ostream_impl::write_impl(src.base(), src._get_map(), dst);
   }
};

} //namespace lofty
//! @endcond

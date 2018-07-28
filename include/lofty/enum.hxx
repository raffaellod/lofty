/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_ENUM_HXX

#include <lofty/enum-0.hxx>

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_ENUM_HXX
#endif

#ifndef _LOFTY_ENUM_HXX_NOPUB
#define _LOFTY_ENUM_HXX_NOPUB

#include <lofty/from_text_istream.hxx>
#include <lofty/text/str-0.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

template <class T>
inline text::_LOFTY_PUBNS str enum_impl<T>::name() const {
   _pvt::enum_member const * member = _member();
   return text::_pub::str(external_buffer, member->name, member->name_size);
}

_LOFTY_PUBNS_END
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Implementation of the specializations of to_text_ostream for enum_impl specializations.
class LOFTY_SYM enum_to_text_ostream_impl {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(text::_LOFTY_PUBNS str const & format);

protected:
   /*! Writes an enumeration value, applying the formatting options.

   @param i
      Value of the enumeration member to write.
   @param members
      Pointer to the enumeration members map.
   @param dst
      Pointer to the stream to output to.
   */
   void write_impl(int i, enum_member const * members, io::text::_LOFTY_PUBNS ostream * dst);
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <class T>
class to_text_ostream<_LOFTY_PUBNS enum_impl<T>> : public _pvt::enum_to_text_ostream_impl {
public:
   //! See lofty::_pvt::enum_to_text_ostream_impl::write().
   void write(_LOFTY_PUBNS enum_impl<T> src, io::text::_LOFTY_PUBNS ostream * dst) {
      _pvt::enum_to_text_ostream_impl::write_impl(src.base(), src._get_map(), dst);
   }
};

}
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_ENUM_HXX_NOPUB

#ifdef _LOFTY_ENUM_HXX
   #undef _LOFTY_NOPUB

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_ENUM_HXX

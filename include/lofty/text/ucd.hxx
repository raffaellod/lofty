/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Unicode Character Database. */

#ifndef _LOFTY_TEXT_UCD_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_UCD_HXX
#endif

#ifndef _LOFTY_TEXT_UCD_HXX_NOPUB
#define _LOFTY_TEXT_UCD_HXX_NOPUB

#include <lofty/noncopyable.hxx>
#include <lofty/text-0.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty { namespace text { namespace ucd { namespace _pvt {

//! POD implementation of lofty::text::property. It has no constructor, so it can be initialized statically.
struct property_data {
   struct member_range_t {
      char32_t first_cp;
      char32_t last_cp;
   };

   text::_LOFTY_PUBNS char_t const * name;
   std::uint8_t name_size;
   member_range_t const * member_ranges_begin;
   member_range_t const * member_ranges_end;
};

}}}} //namespace lofty::text::ucd::_pvt
//! @endcond

namespace lofty { namespace text { namespace ucd {
_LOFTY_PUBNS_BEGIN

//! Unicode character (code point) property.
class LOFTY_SYM property : private _pvt::property_data, public lofty::_LOFTY_PUBNS noncopyable {
public:
   // TODO: generate this list programmatically from PropList.txt.
   static property const & white_space;
   // More uninteresting ones…

public:
   /*! Returns a string containing all the code points matching the property.

   @return
      Matching code points.
   */
   text::_LOFTY_PUBNS str members() const;

   /*! Checks whether a code point matches the property.

   @param cp
      Code point to test.
   @return
      true if the code point matches the property, or false otherwise.
   */
   bool test(char32_t cp) const;

private:
   //! Default constructor.
   property() {
   }

   //! Destructor.
   ~property() {
   }
};

_LOFTY_PUBNS_END
}}} //namespace lofty::text::ucd

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_UCD_HXX_NOPUB

#ifdef _LOFTY_TEXT_UCD_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace text { namespace ucd {

   using _pub::property;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_UCD_HXX

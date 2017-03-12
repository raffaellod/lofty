/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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

/*! @file
Unicode Character Database. */

#ifndef _LOFTY_TEXT_UCD_HXX
#define _LOFTY_TEXT_UCD_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty { namespace text { namespace ucd { namespace _pvt {

//! POD implementation of lofty::text::property. It has no constructor, so it can be initialized statically.
struct property_data {
   struct member_range_t {
      char32_t first_cp;
      char32_t last_cp;
   };

   char_t const * name;
   std::uint8_t name_size;
   member_range_t const * member_ranges_begin;
   member_range_t const * member_ranges_end;
};

}}}} //namespace lofty::text::ucd::_pvt
//! @endcond

namespace lofty { namespace text { namespace ucd {

//! Unicode character (code point) property.
class LOFTY_SYM property : private _pvt::property_data, public noncopyable {
public:
   // TODO: generate this list programmatically from PropList.txt.
   static property const & white_space;
   // More uninteresting ones…

public:
   /*! Returns a string containing all the code points matching the property.

   @return
      Matching code points.
   */
   str members() const;

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

}}} //namespace lofty::text::ucd

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_UCD_HXX

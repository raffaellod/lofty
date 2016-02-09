/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

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

/*! @file
Unicode Character Database. */

#ifndef _ABACLADE_TEXT_UCD_HXX
#define _ABACLADE_TEXT_UCD_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc { namespace text { namespace ucd { namespace _pvt {

/*! POD implementation of abc::text::property. It has no constructor, so it can be initialized
statically. */
struct property_data {
   struct member_range {
      char32_t cpFirst;
      char32_t cpLast;
   };

   char_t const * m_pszName;
   std::uint8_t m_cchName;
   member_range const * m_pmrBegin;
   member_range const * m_pmrEnd;
};

}}}} //namespace abc::text::ucd::_pvt
//! @endcond

namespace abc { namespace text { namespace ucd {

//! Unicode character (code point) property.
class ABACLADE_SYM property : private _pvt::property_data, public noncopyable {
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

}}} //namespace abc::text::ucd

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TEXT_UCD_HXX

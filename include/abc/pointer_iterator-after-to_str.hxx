/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::pointer_iterator


namespace abc {

template <typename TCont, typename TVal>
class to_str_backend<pointer_iterator<TCont, TVal>> :
   protected to_str_backend<typename pointer_iterator<TCont, TVal>::const_pointer> {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      to_str_backend<typename pointer_iterator<TCont, TVal>::const_pointer>(sFormat) {
   }


   /** Writes a NUL-terminated string, applying the formatting options.

   psz
      Pointer to the string to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(pointer_iterator<TCont, TVal> const & it, io::text::writer * ptwOut) {
      to_str_backend<typename pointer_iterator<TCont, TVal>::const_pointer>::write(
         it.base(), ptwOut
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


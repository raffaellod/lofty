/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TO_STR_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TO_STR_HXX
#endif

#ifndef _LOFTY_TO_STR_HXX_NOPUB
#define _LOFTY_TO_STR_HXX_NOPUB

#include <lofty/text/str-0.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Returns the string representation of the specified value, optionally with a custom format.

lofty::to_str() is a more advanced alternative to std::to_string() (see C++11 § 21.5 “Numeric conversions”);
here are the main differences when compared to the STL function:

•  It accepts an additional argument, controlling how the conversion to string is to be done;

•  Its default specialization relies on lofty::to_text_ostream, which outputs its result to an
   lofty::io::text::ostream instance; this means that the complete specialization is shared with
   lofty::io::text::ostream::print();

The format specification is provided to a lofty::to_text_ostream specialization by passing it a
lofty::text::str const &, so a caller can specify a non-NUL-terminated substring of a larger string without
the need for temporary strings. Once a lofty::to_text_ostream instance has been constructed, it must be able
to sequentially process an infinite number of conversions, i.e. instances of a to_text_ostream specialization
must be reusable.

The interpretation of the format specification is up to the specialization of lofty::to_text_ostream.

When code causes the compiler to instantiate a specialization of lofty::to_text_ostream that has not been
defined, GCC will generate an error:

   @verbatim
   error: ‘lofty::to_text_ostream<my_type> …’ has incomplete type
   @endverbatim

The only fix for this is to provide an explicit specialization of lofty::to_text_ostream for my_type.

@param src
   Object to generate a string representation for.
@param format
   Type-specific format string.
@return
   String representation of t according to format.
*/
template <typename T>
inline text::_LOFTY_PUBNS str to_str(
   T const & src, text::_LOFTY_PUBNS str const & format = text::_LOFTY_PUBNS str::empty
) {
   io::text::_pub::str_ostream ostream;
   {
      to_text_ostream<T> ttos;
      ttos.set_format(format);
      ttos.write(src, &ostream);
   }
   return ostream.release_content();
}

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TO_STR_HXX_NOPUB

#ifdef _LOFTY_TO_STR_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::to_str;

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TO_STR_HXX

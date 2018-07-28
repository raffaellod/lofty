/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX
#endif

#ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX_NOPUB
#define _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX_NOPUB

#include <lofty/text/str-1.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

/*! Adapter to allow printing of C-style NUL-terminated char * strings via to_text_ostream. Use this for
compatibility with STL methods such as std::exception::what(). Without this, C strings are printed only as
pointers, which is often undesirable.

lofty::text::str(external_buffer, "string") would seem to work just fine on POSIX platforms, but it’s not
equivalent on Win32, where char (used by STL) and char_t (used by Lofty) are not the same type. Even on POSIX,
constructing str instances is slower than using char_ptr_to_str_adapter.

Instances of this class don’t own the memory object they point to. */
class char_ptr_to_str_adapter {
private:
   friend class lofty::to_text_ostream<char_ptr_to_str_adapter>;

public:
   /*! Constructor.

   @param s_
      C-style NUL-terminated string.
   */
   char_ptr_to_str_adapter(char const * s_) :
      s(s_) {
   }

protected:
   //! Wrapped C-style string.
   char const * s;
};

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<text::_LOFTY_PUBNS char_ptr_to_str_adapter> :
   public text::_pvt::str_to_text_ostream {
public:
   /*! Writes a C-style NUL-terminated string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::_LOFTY_PUBNS char_ptr_to_str_adapter const & src, io::text::_LOFTY_PUBNS ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX_NOPUB

#ifdef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace text {

   using _pub::char_ptr_to_str_adapter;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX
#define _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

/*! Adapter to allow printing of C-style NUL-terminated char * strings via to_text_ostream. Use this for
compatibility with STL methods such as std::exception::what(). Without this, C strings are printed only as
pointers, which is often undesirable.

lofty::str(external_buffer, "string") would seem to work just fine on POSIX platforms, but it’s not equivalent
on Win32, where char (used by STL) and char_t (used by Lofty) are not the same type. Even on POSIX,
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

}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<text::char_ptr_to_str_adapter> : public text::_pvt::str_to_text_ostream {
public:
   /*! Writes a C-style NUL-terminated string, applying the formatting options.

   @param src
      Object to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(text::char_ptr_to_str_adapter const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

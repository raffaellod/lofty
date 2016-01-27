/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX
#define _ABACLADE_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

/*! Adapter to allow printing of C-style NUL-terminated char * strings via to_text_ostream. Use this
for compatibility with STL methods such as std::exception::what(). Without this, C strings are
printed only as pointers, which is often undesirable.

abc::str(external_buffer, "string") would seem to work just fine on POSIX platforms, but it’s not
equivalent on Win32, where char (used by STL) and char_t (used by Abaclade) are not the same type.
Even on POSIX, constructing str instances is slower than using char_ptr_to_str_adapter.

Instances of this class don’t own the memory object they point to. */
class char_ptr_to_str_adapter {
private:
   friend class abc::to_text_ostream<char_ptr_to_str_adapter>;

public:
   /*! Constructor.

   @param psz
      C-style NUL-terminated string.
   */
   char_ptr_to_str_adapter(char const * psz) :
      m_psz(psz) {
   }

protected:
   //! Wrapped C-style string.
   char const * m_psz;
};

}} //namespace abc::text

////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace abc {

template <>
class ABACLADE_SYM to_text_ostream<text::char_ptr_to_str_adapter> :
   public text::detail::str_to_text_ostream {
public:
   /*! Writes a C-style NUL-terminated string, applying the formatting options.

   @param cs
      C string to write.
   @param ptos
      Pointer to the stream to output to.
   */
   void write(text::char_ptr_to_str_adapter const & cs, io::text::ostream * ptos);
};

} //namespace abc
//! @endcond

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TEXT_CHAR_PTR_TO_STR_ADAPTER_HXX

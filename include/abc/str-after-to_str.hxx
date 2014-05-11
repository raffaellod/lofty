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

#include <abc/_vextr.hxx>
#include <abc/utf_traits.hxx>
#include <functional>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_str_to_str_backend


namespace abc {

/** Base class for the specializations of to_str_backend for string types. Not using templates, so
the implementation can be in a cxx file. This is used by string literal types as well (see below).
*/
class ABCAPI _str_to_str_backend {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   _str_to_str_backend(char_range const & crFormat);


protected:

   /** Writes a string, applying the formatting options.

   p
      Pointer to the string to write.
   cb
      Size of the string pointed to by p, in bytes.
   enc
      Text encoding of the string pointed to by p.
   posOut
      Pointer to the output stream to write to.
   */
   void write(void const * p, size_t cb, text::encoding enc, io::ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for character literal types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C) \
   /** Character literal. \
   */ \
   template <> \
   class to_str_backend<C> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Constructor.

      crFormat
         Formatting options.
      */ \
      to_str_backend(char_range const & crFormat = char_range()) : \
         _str_to_str_backend(crFormat) { \
      } \
   \
   \
      /** Writes a character, applying the formatting options.

      ch
         Character to write.
      posOut
         Pointer to the output stream to write to.
      */ \
      void write(C ch, io::ostream * posOut) { \
         _str_to_str_backend::write(&ch, sizeof(C), text::utf_traits<C>::host_encoding, posOut); \
      } \
   }; \
   \
   /** Const character literal. \

   TODO: remove the need for this.
   */ \
   template <> \
   class to_str_backend<C const> : \
      public to_str_backend<C> { \
   public: \
   \
      /** Constructor.

      crFormat
         Formatting options.
      */ \
      to_str_backend(char_range const & crFormat = char_range()) : \
         to_str_backend<C>(crFormat) { \
      } \
   };
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char)
// Specialization for wchar_t, if it’s what char16_t or char32_t map to.
#if ABC_CXX_CHAR16 == 1 || ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t)
#endif
// Specializations for char16/32_t, if they’re native types.
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t)
#endif
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for string literal types


namespace abc {

#define ABC_SPECIALIZE_to_str_backend_FOR_TYPE(C) \
   /** String literal. \
   */ \
   template <size_t t_cch> \
   class to_str_backend<C const [t_cch]> : \
      public _str_to_str_backend { \
   public: \
   \
      /** Constructor.

      crFormat
         Formatting options.
      */ \
      to_str_backend(char_range const & crFormat = char_range()) : \
         _str_to_str_backend(crFormat) { \
      } \
   \
   \
      /** Writes a string, applying the formatting options.

      ach
         String to write.
      posOut
         Pointer to the output stream to write to.
      */ \
      void write(C const (& ach)[t_cch], io::ostream * posOut) { \
         ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated")); \
         _str_to_str_backend::write( \
            ach, sizeof(C) * (t_cch - 1 /*NUL*/), text::utf_traits<C>::host_encoding, posOut \
         ); \
      } \
   }; \
   \
   /** Non-const string literal.

   TODO: remove the need for this.
   */ \
   template <size_t t_cch> \
   class to_str_backend<C [t_cch]> : \
      public to_str_backend<C const [t_cch]> { \
   public: \
   \
      /** Constructor.

      crFormat
         Formatting options.
      */ \
      to_str_backend(char_range const & crFormat = char_range()) : \
         to_str_backend<C const [t_cch]>(crFormat) { \
      } \
   };
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char)
// Specialization for wchar_t, if it’s what char16_t or char32_t map to.
#if ABC_CXX_CHAR16 == 1 || ABC_CXX_CHAR32 == 1
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(wchar_t)
#endif
// Specializations for char16/32_t, if they’re native types.
#if ABC_CXX_CHAR16 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char16_t)
#endif
#if ABC_CXX_CHAR32 == 2
ABC_SPECIALIZE_to_str_backend_FOR_TYPE(char32_t)
#endif
#undef ABC_SPECIALIZE_to_str_backend_FOR_TYPE

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::char_range_


namespace abc {

template <typename C>
class to_str_backend<char_range_<C>> :
   public _str_to_str_backend {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      _str_to_str_backend(crFormat) {
   }


   /** Writes a character range, applying the formatting options.

   cr
      Range of characters to write.
   posOut
      Pointer to the output stream to write to.
   */
   void write(char_range_<C> const & cr, io::ostream * posOut) {
      _str_to_str_backend::write(
         cr.cbegin().base(), sizeof(C) * cr.size(), text::utf_traits<C>::host_encoding, posOut
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::str_base_


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<str_base_<C, TTraits>> :
   public _str_to_str_backend {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      _str_to_str_backend(crFormat) {
   }


   /** Writes a string, applying the formatting options.

   s
      String to write.
   posOut
      Pointer to the output stream to write to.
   */
   void write(str_base_<C, TTraits> const & s, io::ostream * posOut) {
      _str_to_str_backend::write(s.data(), sizeof(C) * s.size(), TTraits::host_encoding, posOut);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::istr_


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<istr_<C, TTraits>> :
   public to_str_backend<str_base_<C, TTraits>> {
public:

   /** Constructor. See to_str_backend<str_base_<C, TTraits>>::to_str_backend().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      to_str_backend<str_base_<C, TTraits>>(crFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::mstr_


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<mstr_<C, TTraits>> :
   public to_str_backend<str_base_<C, TTraits>> {
public:

   /** Constructor. See to_str_backend<str_base_<C, TTraits>>::to_str_backend().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      to_str_backend<str_base_<C, TTraits>>(crFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::dmstr_


namespace abc {

// Specialization of to_str_backend.
template <typename C, class TTraits>
class to_str_backend<dmstr_<C, TTraits>> :
   public to_str_backend<str_base_<C, TTraits>> {
public:

   /** Constructor. See to_str_backend<str_base_<C, TTraits>>::to_str_backend().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      to_str_backend<str_base_<C, TTraits>>(crFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for abc::smstr


namespace abc {

// Specialization of to_str_backend.
template <size_t t_cchStatic, typename C, class TTraits>
class to_str_backend<smstr<t_cchStatic, C, TTraits>> :
   public to_str_backend<str_base_<C, TTraits>> {
public:

   /** Constructor. See to_str_backend<str_base_<C, TTraits>>::to_str_backend().
   */
   to_str_backend(char_range const & crFormat = char_range()) :
      to_str_backend<str_base_<C, TTraits>>(crFormat) {
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


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

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::c_str_to_str_adapter


namespace abc {

/** Adapter to allow printing of C-style NUL-terminated char * strings via to_str_backend. Use this
for compatibility with STL methods such as std::exception::what(). Without this, C strings are
printed only as pointers, which is often undesirable.

Instances of this class don’t own the memory object they point to.
*/
class c_str_to_str_adapter {

   friend class to_str_backend<c_str_to_str_adapter>;

public:

   /** Constructor.

   psz
      C-style NUL-terminated string.
   */
   c_str_to_str_adapter(char const * psz) :
      m_psz(psz) {
   }


protected:

   /** Wrapped C-style string. */
   char const * m_psz;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specialization for c_str_to_str_adapter


namespace abc {

template <>
class ABCAPI to_str_backend<c_str_to_str_adapter> :
   public _str_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      _str_to_str_backend(sFormat) {
   }


   /** Writes a C-style NUL-terminated string, applying the formatting options.

   cs
      C string to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(c_str_to_str_adapter const & cs, io::text::writer * ptwOut) {
      // TODO: FIXME: for MSC16, char * is not UTF-8.
      _str_to_str_backend::write(
         cs.m_psz, sizeof(char) * text::utf8_traits::str_len(cs.m_psz), text::encoding::utf8, ptwOut
      );
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_ptr_to_str_backend


namespace abc {

/** Base class for the specializations of to_str_backend for integer types.
*/
class ABCAPI _ptr_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   _ptr_to_str_backend(istr const & sFormat);


protected:

   /** Converts a pointer to a string representation.

   iPtr
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void _write_impl(uintptr_t iPtr, io::text::writer * ptwOut);


protected:

   /** Backend used to write the pointer as an integer. */
   to_str_backend<uintptr_t> m_tsbInt;
   /** Backend used to write a nullptr. */
   to_str_backend<istr> m_tsbStr;
   /** Format string used to display the address. */
   static char_t const smc_achFormat[];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend - specializations for pointer types


namespace abc {

// Specialization for raw pointer types.
template <typename T>
class to_str_backend<T *> :
   public _ptr_to_str_backend {
public:

   /** See _ptr_to_str_backend::_ptr_to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      _ptr_to_str_backend(sFormat) {
   }


   /** Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(T * p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<uintptr_t>(p), ptwOut);
   }
};

// Specialization for std::unique_ptr.
template <typename T, typename TDel>
class to_str_backend<std::unique_ptr<T, TDel>> :
   public _ptr_to_str_backend {
public:

   /** See _ptr_to_str_backend::_ptr_to_str_backend().
   */
   to_str_backend(istr const & sFormat = istr()) :
      _ptr_to_str_backend(sFormat) {
   }


   /** See _ptr_to_str_backend::write().
   */
   void write(std::unique_ptr<T, TDel> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<uintptr_t>(p.get()), ptwOut);
   }
};

// Specialization for std::shared_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_str_backend<std::shared_ptr<T>> :
   public _ptr_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      _ptr_to_str_backend(sFormat) {
   }


   /** Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(std::shared_ptr<T> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<uintptr_t>(p.get()), ptwOut);
   }
};

// Specialization for std::weak_ptr.
// TODO: show reference count and other info.
template <typename T>
class to_str_backend<std::weak_ptr<T>> :
   public _ptr_to_str_backend {
public:

   /** Constructor.

   sFormat
      Formatting options.
   */
   to_str_backend(istr const & sFormat = istr()) :
      _ptr_to_str_backend(sFormat) {
   }


   /** Converts a pointer to a string representation.

   p
      Pointer to write.
   ptwOut
      Pointer to the writer to output to.
   */
   void write(std::weak_ptr<T> const & p, io::text::writer * ptwOut) {
      _write_impl(reinterpret_cast<uintptr_t>(p.lock().get()), ptwOut);
   }
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


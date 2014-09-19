/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_STL_FUNCTIONAL_HXX
#define _ABACLADE_STL_FUNCTIONAL_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// std::hash standard specializations

namespace std {

//! Computes the hash of an object (C++11 § 20.8.12 “Class template hash”).
template <typename T>
struct hash;

template <>
struct hash<bool> {
   /*! Function call operator.

   b
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(bool b) const {
      return b;
   }
};

template <>
struct hash<char> {
   /*! Function call operator.

   ch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(char ch) const {
      return ch;
   }
};

template <>
struct hash<signed char> {
   /*! Function call operator.

   sch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(signed char sch) const {
      return sch;
   }
};

template <>
struct hash<unsigned char> {
   /*! Function call operator.

   uch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(unsigned char uch) const {
      return uch;
   }
};

template <>
struct hash<char16_t> {
   /*! Function call operator.

   ch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(char16_t ch) const {
      return ch;
   }
};

template <>
struct hash<char32_t> {
   /*! Function call operator.

   ch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(char32_t ch) const {
      return ch;
   }
};

template <>
struct hash<wchar_t> {
   /*! Function call operator.

   ch
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(wchar_t ch) const {
      return ch;
   }
};

template <>
struct hash<short> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(short i) const {
      return i;
   }
};

template <>
struct hash<unsigned short> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(unsigned short i) const {
      return i;
   }
};

template <>
struct hash<int> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(int i) const {
      return i;
   }
};

template <>
struct hash<unsigned int> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(unsigned int i) const {
      return i;
   }
};

template <>
struct hash<long> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(long i) const {
      return i;
   }
};

template <>
struct hash<long long> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(long long i) const {
      return i;
   }
};

template <>
struct hash<unsigned long> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(unsigned long i) const {
      return i;
   }
};

template <>
struct hash<unsigned long long> {
   /*! Function call operator.

   i
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(unsigned long long i) const {
      return i;
   }
};

template <>
struct hash<float> {
   /*! Function call operator.

   f
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(float f) const {
      return f;
   }
};

template <>
struct hash<double> {
   /*! Function call operator.

   d
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(double d) const {
      return d;
   }
};

template <>
struct hash<long double> {
   /*! Function call operator.

   ld
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(long double ld) const {
      return ld;
   }
};

template <typename T>
struct hash<T *> {
   /*! Function call operator.

   pt
      Object to computer the hash of.
   return
      Hash of the argument.
   */
   size_t operator()(T * pt) const {
      return reinterpret_cast<uintptr_t>(pt);
   }
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STL_FUNCTIONAL_HXX


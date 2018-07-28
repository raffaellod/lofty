/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_FUNCTIONAL_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_FUNCTIONAL_HXX
#endif

#ifndef _LOFTY_STD_FUNCTIONAL_HXX_NOPUB
#define _LOFTY_STD_FUNCTIONAL_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Determines the equality of two objects of the same type (C++11 § 20.8.5 “Function objects, comparisons”).
template <typename T>
struct equal_to {
   /*! Function call operator.

   @param left
      First object to compare.
   @param right
      Second object to compare.
   @return
      true if the two objects are equal, or false otherwise.
   */
   bool operator()(T const & left, T const & right) const {
       return left == right;
   }
};

//! Computes the hash of an object (C++11 § 20.8.12 “Class template hash”).
template <typename T>
struct hash;

template <>
struct hash<bool> {
   /*! Function call operator.

   @param b
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(bool b) const {
      return b;
   }
};

template <>
struct hash<char> {
   /*! Function call operator.

   @param ch
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(char ch) const {
      return ch;
   }
};

template <>
struct hash<signed char> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(signed char i) const {
      return i;
   }
};

template <>
struct hash<unsigned char> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(unsigned char i) const {
      return i;
   }
};

template <>
struct hash<char16_t> {
   /*! Function call operator.

   @param ch
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(char16_t ch) const {
      return ch;
   }
};

template <>
struct hash<char32_t> {
   /*! Function call operator.

   @param ch
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(char32_t ch) const {
      return ch;
   }
};

template <>
struct hash<wchar_t> {
   /*! Function call operator.

   @param ch
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(wchar_t ch) const {
      return ch;
   }
};

template <>
struct hash<short> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(short i) const {
      return i;
   }
};

template <>
struct hash<unsigned short> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(unsigned short i) const {
      return i;
   }
};

template <>
struct hash<int> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(int i) const {
      return i;
   }
};

template <>
struct hash<unsigned int> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(unsigned int i) const {
      return i;
   }
};

template <>
struct hash<long> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(long i) const {
      return i;
   }
};

template <>
struct hash<long long> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(long long i) const {
      return i;
   }
};

template <>
struct hash<unsigned long> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(unsigned long i) const {
      return i;
   }
};

template <>
struct hash<unsigned long long> {
   /*! Function call operator.

   @param i
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(unsigned long long i) const {
      return i;
   }
};

template <>
struct hash<float> {
   /*! Function call operator.

   @param f
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(float f) const {
      return f;
   }
};

template <>
struct hash<double> {
   /*! Function call operator.

   @param d
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(double d) const {
      return d;
   }
};

template <>
struct hash<long double> {
   /*! Function call operator.

   @param ld
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(long double ld) const {
      return ld;
   }
};

template <typename T>
struct hash<T *> {
   /*! Function call operator.

   @param t
      Object to computer the hash of.
   @return
      Hash of the argument.
   */
   std::size_t operator()(T * t) const {
      return reinterpret_cast<std::uintptr_t>(t);
   }
};

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY
   #include <functional>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::equal_to;
   using ::std::function;
   using ::std::hash;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_FUNCTIONAL_HXX_NOPUB

#ifdef _LOFTY_STD_FUNCTIONAL_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::equal_to;
   using _pub::function;
   using _pub::hash;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_FUNCTIONAL_HXX

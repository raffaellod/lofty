/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_STR_HXX

#include <lofty/text/str-1.hxx>

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TEXT_STR_HXX
#endif

#ifndef _LOFTY_TEXT_STR_HXX_NOPUB
#define _LOFTY_TEXT_STR_HXX_NOPUB

#include <lofty/io/text-0.hxx>
#include <lofty/io/text/str.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {
_LOFTY_PUBNS_BEGIN

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline void str::format(str const & fmt, Ts const &... ts) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, ts...);
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

inline void str::format(str const & fmt) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt);
}
template <typename T0>
inline void str::format(str const & fmt, T0 const & t0) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0);
}
template <typename T0, typename T1>
inline void str::format(str const & fmt, T0 const & t0, T1 const & t1) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1);
}
template <typename T0, typename T1, typename T2>
inline void str::format(str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2);
}
template <typename T0, typename T1, typename T2, typename T3>
inline void str::format(str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4, t5);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4, t5, t6, t7, t8);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline void str::format(
   str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   clear();
   io::text::_pub::str_ostream stream(lofty::_pub::external_buffer, this);
   stream.print(fmt, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_STR_HXX_NOPUB

#ifdef _LOFTY_TEXT_STR_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace text {

   using _pub::str;
   using _pub::sstr;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TEXT_STR_HXX

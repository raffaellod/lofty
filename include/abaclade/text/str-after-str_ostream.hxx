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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline void str::format(str const & sFormat, Ts const &... ts) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, ts...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline void str::format(str const & sFormat) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat);
}
template <typename T0>
inline void str::format(str const & sFormat, T0 const & t0) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0);
}
template <typename T0, typename T1>
inline void str::format(str const & sFormat, T0 const & t0, T1 const & t1) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1);
}
template <typename T0, typename T1, typename T2>
inline void str::format(str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2);
}
template <typename T0, typename T1, typename T2, typename T3>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4, t5);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline void str::format(
   str const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   clear();
   io::text::str_ostream sos(external_buffer, this);
   sos.print(sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::text

﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

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
inline str str::format(Ts const &... ts) const {
   io::text::str_writer tsw;
   tsw.print(*this, ts ...);
   return tsw.release_content();
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline str str::format() const {
   io::text::str_writer tsw;
   tsw.print(*this);
   return tsw.release_content();
}
template <typename T0>
inline str str::format(T0 const & t0) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0);
   return tsw.release_content();
}
template <typename T0, typename T1>
inline str str::format(T0 const & t0, T1 const & t1) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1);
   return tsw.release_content();
}
template <typename T0, typename T1, typename T2>
inline str str::format(T0 const & t0, T1 const & t1, T2 const & t2) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2);
   return tsw.release_content();
}
template <typename T0, typename T1, typename T2, typename T3>
inline str str::format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3);
   return tsw.release_content();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4);
   return tsw.release_content();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4, t5);
   return tsw.release_content();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4, t5, t6);
   return tsw.release_content();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4, t5, t6, t7);
   return tsw.release_content();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4, t5, t6, t7, t8);
   return tsw.release_content();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline str str::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) const {
   io::text::str_writer tsw;
   tsw.print(*this, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
   return tsw.release_content();
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::text

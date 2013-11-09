/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013
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

#ifndef ABC_STRING_IOSTREAM_HXX
#define ABC_STRING_IOSTREAM_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/iostream.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_istream


namespace abc {

/** Implementation of an read-only stream based on a string.
*/
class ABCAPI str_istream :
   public virtual istream {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit str_istream(istr const & s);
   explicit str_istream(istr && s);
   explicit str_istream(mstr && s);
   explicit str_istream(dmstr && s);


   /** Destructor.
   */
   virtual ~str_istream();


   /** See istream::read_raw().
   */
   virtual size_t read_raw(
      void * p, size_t cbMax, text::encoding enc = text::encoding::identity
   );


   /** See istream::unread_raw().
   */
   virtual void unread_raw(
      void const * p, size_t cb, text::encoding enc = text::encoding::identity
   );


protected:

   /** See istream::_read_line().
   */
   virtual void _read_line(
      _raw_str * prs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
   );


protected:

   /** Source string. */
   istr m_sBuf;
   /** Current read offset into the string, in bytes. Seeks can only change this in increments of a
   character, but internal code doesn’t have to. */
   size_t m_ibRead;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_ostream


namespace abc {

/** Implementation of an write-only stream based on a string.
*/
class ABCAPI str_ostream :
   public virtual ostream {
public:

   /** Type of the string used as buffer. */
   typedef dmstr str_type;

public:

   /** Constructor.
   */
   str_ostream();


   /** Destructor.
   */
   virtual ~str_ostream();


   /** Yields ownership of the string buffer.

   return
      Former content of the stream.
   */
   str_type release_content();


   /** See ostream::write_raw().
   */
   virtual void write_raw(
      void const * p, size_t cb, text::encoding enc = text::encoding::identity
   );


protected:

   /** Target string. */
   str_type m_sBuf;
   /** Current write offset into the string, in bytes. Seeks can only change this in increments of a
   character, but internal code doesn’t have to. */
   size_t m_ibWrite;
};


// Now these can be implemented.

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename C, class TTraits>
template <typename ... Ts>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(Ts const & ... ts) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), ts ...);
   return os.release_content();
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename C, class TTraits>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format() const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this));
   return os.release_content();
}
template <typename C, class TTraits>
template <typename T0>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(T0 const & t0) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0);
   return os.release_content();
}

template <typename C, class TTraits>
template <typename T0, typename T1>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(T0 const & t0, T1 const & t1) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1);
   return os.release_content();
}
template <typename C, class TTraits>
template <typename T0, typename T1, typename T2>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2);
   return os.release_content();
}
template <typename C, class TTraits>
template <typename T0, typename T1, typename T2, typename T3>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3);
   return os.release_content();
}
template <typename C, class TTraits>
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4);
   return os.release_content();
}
template <typename C, class TTraits>
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4, t5);
   return os.release_content();
}
template <typename C, class TTraits>
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4, t5, t6);
   return os.release_content();
}
template <typename C, class TTraits>
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4, t5, t6, t7);
   return os.release_content();
}
template <typename C, class TTraits>
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4, t5, t6, t7, t8);
   return os.release_content();
}
template <typename C, class TTraits>
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline dmstr_<C, TTraits> str_base_<C, TTraits>::format(
   T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
   T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) const {
   str_ostream os;
   os.print(*static_cast<istr_<C, TTraits> const *>(this), t0, t1, t2, t3, t4, t5, t6, t7, t8, t9);
   return os.release_content();
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


template <typename T>
inline dmstr to_str(T const & t, istr const & sFormat /*= istr()*/) {
   str_ostream os;
   to_str_backend<T> tsb(sFormat);
   tsb.write(t, &os);
   return os.release_content();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_STRING_IOSTREAM_HXX


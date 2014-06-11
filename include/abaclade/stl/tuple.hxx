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

#ifndef _ABACLADE_STL_TUPLE_HXX
#define _ABACLADE_STL_TUPLE_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/stl/utility.hxx>
#else
   #include <utility>
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::_tuple_void


#ifndef ABC_CXX_VARIADIC_TEMPLATES

namespace abc {
namespace _std {

/** Null type, used to reduce the number of tuple items from the preset maximum.
*/
struct _tuple_void {
};

} //namespace _std
} //namespace abc

#endif //ifndef ABC_CXX_VARIADIC_TEMPLATES


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::_tuple_head


namespace abc {
namespace _std {

/** Base for a tuple item. For empty T, it derives from T; otherwise, it has a T member. This allows
for empty base optimization (EBO), if the compiler is smart enough.
*/
template <size_t t_i, typename T, bool t_bEmpty = is_empty<T>::value>
class _tuple_head;

// Specialization for empty types: enable EBO.
template <size_t t_i, typename T>
class _tuple_head<t_i, T, true> :
   private T {
public:

   /** Constructor.

   th
      Source tuple head.
   t
      Source element.
   */
   _tuple_head() :
      T() {
   }
   _tuple_head(_tuple_head const & th) :
      T(static_cast<T const &>(th)) {
   }
   _tuple_head(_tuple_head && th) :
      T(static_cast<T &&>(th)) {
   }
   template <typename Tr>
   _tuple_head(Tr && t) :
      T(forward<Tr>(t)) {
   }


   /** Assignment operator.

   th
      Source tuple head.
   return
      *this.
   */
   _tuple_head & operator=(_tuple_head const & th) {
      get() = th.get();
      return *this;
   }
   _tuple_head & operator=(_tuple_head && th) {
      get() = move(th.get());
      return *this;
   }


   /** Accessor to the wrapped object.

   return
      Reference to the wrapped element.
   */
   T & get() {
      return *this;
   }
   T const & get() const {
      return *this;
   }
};

// Specialization non non-empty types.
template <size_t t_i, typename T>
class _tuple_head<t_i, T, false> {
public:

   /** Constructor.

   th
      Source tuple head.
   t
      Source element.
   */
   _tuple_head() :
      m_t() {
   }
   _tuple_head(_tuple_head const & th) :
      m_t(th.m_t) {
   }
   _tuple_head(_tuple_head && th) :
      m_t(move(th.m_t)) {
   }
   template <typename Tr>
   _tuple_head(Tr && t) :
      m_t(forward<Tr>(t)) {
   }


   /** Assignment operator.

   th
      Source tuple head.
   return
      *this.
   */
   _tuple_head & operator=(_tuple_head const & th) {
      get() = th.get();
      return *this;
   }
   _tuple_head & operator=(_tuple_head && th) {
      get() = move(th.get());
      return *this;
   }


   /** Accessor to the wrapped element.

   return
      Reference to the wrapped element.
   */
   T & get() {
      return m_t;
   }
   T const & get() const {
      return m_t;
   }


private:

   /** Internal T instance. */
   T m_t;
};

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::_tuple_tail


namespace abc {
namespace _std {

/** Internal implementation of tuple.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <size_t t_i, typename ... Ts>
class _tuple_tail;

// Base case for the template recursion.
template <size_t t_i>
class _tuple_tail<t_i> {
};

// Template recursion step.
template <size_t t_i, typename T0, typename ... Ts>
class _tuple_tail<t_i, T0, Ts ...> :
   public _tuple_head<t_i, T0>,
   public _tuple_tail<t_i + 1, Ts ...> {

   typedef _tuple_head<t_i, T0> _thead;
   typedef _tuple_tail<t_i + 1, Ts ...> _ttail;

public:

   /** Constructor.

   thead
      Source tuple head.
   ts
      Source elements.
   tt
      Source tuple tail.
   */
   _tuple_tail() :
      _thead(), _ttail() {
   }
   _tuple_tail(T0 thead, Ts ... ts) :
      _thead(move(thead)), _ttail(move(ts) ...) {
   }
   _tuple_tail(_tuple_tail const & tt) :
      _thead(tt.get_thead()), _ttail(tt.get_ttail()) {
   }
   _tuple_tail(_tuple_tail && tt) :
      _thead(move(tt.get_thead())), _ttail(move(tt.get_ttail())) {
   }


   /** Assignment operator.

   tt
      Source tuple tail.
   return
      *this.
   */
   _tuple_tail & operator=(_tuple_tail const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }
   _tuple_tail & operator=(_tuple_tail && tt) {
      get_thead() = move(tt.get_thead());
      get_ttail() = move(tt.get_ttail());
      return *this;
   }


   /** Returns the embedded _tuple_head.

   return
      Reference to the embedded tuple head.
   */
   _thead & get_thead() {
      return *static_cast<_thead *>(this);
   }
   _thead const & get_thead() const {
      return *static_cast<_thead const *>(this);
   }


   /** Returns the embedded _tuple_tail.

   return
      Reference to the embedded tuple tail.
   */
   _ttail & get_ttail() {
      return *static_cast<_ttail *>(this);
   }
   _ttail const & get_ttail() const {
      return *static_cast<_ttail const *>(this);
   }
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   size_t t_i, typename T0 = _tuple_void, typename T1 = _tuple_void, typename T2 = _tuple_void,
   typename T3 = _tuple_void, typename T4 = _tuple_void, typename T5 = _tuple_void,
   typename T6 = _tuple_void, typename T7 = _tuple_void, typename T8 = _tuple_void,
   typename T9 = _tuple_void
>
class _tuple_tail :
   public _tuple_head<t_i, T0>,
   public _tuple_tail<t_i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, _tuple_void> {

   typedef _tuple_head<t_i, T0> _thead;
   typedef _tuple_tail<t_i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, _tuple_void> _ttail;

public:

   /** Constructor.

   tt
      Source tuple tail.
   t0...t9
      Source elements.
   */
   _tuple_tail() :
      _thead(), _ttail() {
   }
   _tuple_tail(_tuple_tail const & tt) :
      _thead(tt.get_thead()),
      _ttail(tt.get_ttail()) {
   }
   _tuple_tail(_tuple_tail && tt) :
      _thead(move(tt.get_thead())),
      _ttail(move(tt.get_ttail())) {
   }
   _tuple_tail(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) :
      _thead(move(t0)),
      _ttail(
         move(t1), move(t2), move(t3), move(t4), move(t5), move(t6), move(t7), move(t8), move(t9),
         _tuple_void()
      ) {
   }


   /** Assignment operator.

   tt
      Source tuple tail.
   return
      *this.
   */
   _tuple_tail & operator=(_tuple_tail const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }
   _tuple_tail & operator=(_tuple_tail && tt) {
      get_thead() = move(tt.get_thead());
      get_ttail() = move(tt.get_ttail());
      return *this;
   }


   /** Returns the embedded _tuple_head.

   return
      Reference to the embedded tuple head.
   */
   _thead & get_thead() {
      return *static_cast<_thead *>(this);
   }
   _thead const & get_thead() const {
      return *static_cast<_thead const *>(this);
   }


   /** Returns the embedded _tuple_tail.

   return
      Reference to the embedded tuple tail.
   */
   _ttail & get_ttail() {
      return *static_cast<_ttail *>(this);
   }
   _ttail const & get_ttail() const {
      return *static_cast<_ttail const *>(this);
   }
};

// Base case for the template recursion.
template <size_t t_i>
class _tuple_tail<
   t_i, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void,
   _tuple_void, _tuple_void, _tuple_void
> {
public:

   /** Constructor.

   tt
      Source tuple tail.
   */
   _tuple_tail() {
   }
   _tuple_tail(_tuple_tail const &) {
   }
   _tuple_tail(
      _tuple_void const &, _tuple_void const &, _tuple_void const &, _tuple_void const &,
      _tuple_void const &, _tuple_void const &, _tuple_void const &, _tuple_void const &,
      _tuple_void const &, _tuple_void const &
   ) {
   }


   /** Assignment operator.

   return
      *this.
   */
   _tuple_tail & operator=(_tuple_tail const &) {
      return *this;
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::tuple


namespace abc {
namespace _std {

/** Fixed-size ordered collection of heterogeneous objects (C++11 § 20.4.2 “Class template tuple”).
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
class tuple :
   public _tuple_tail<0, Ts ...> {

   typedef _tuple_tail<0, Ts ...> _timpl;

public:

   /** Constructor.

   ts
      Source elements.
   tpl
      Source tuple.
   */
   tuple() :
      _timpl() {
   }
   explicit tuple(Ts ... ts) :
      _timpl(move(ts) ...) {
   }
   tuple(tuple const & tpl) :
      _timpl(static_cast<_timpl const &>(tpl)) {
   }
   tuple(tuple && tpl) :
      _timpl(static_cast<_timpl &&>(tpl)) {
   }


   /** Assignment operator.

   tpl
      Source tuple.
   return
      *this.
   */
   tuple & operator=(tuple const & tpl) {
      _timpl::operator=(static_cast<_timpl const &>(tpl));
      return *this;
   }
   tuple & operator=(tuple && tpl) {
      _timpl::operator=(static_cast<_timpl &&>(tpl));
      return *this;
   }
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0 = _tuple_void, typename T1 = _tuple_void, typename T2 = _tuple_void,
   typename T3 = _tuple_void, typename T4 = _tuple_void, typename T5 = _tuple_void,
   typename T6 = _tuple_void, typename T7 = _tuple_void, typename T8 = _tuple_void,
   typename T9 = _tuple_void
>
class tuple :
   public _tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> {

   typedef _tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _timpl;

public:

   /** Constructor.

   t0...t9
      Source elements.
   tpl
      Source tuple.
   */
   tuple() :
      _timpl(
         _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 1.
   explicit tuple(T0 t0) :
      _timpl(
         move(t0), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 2.
   tuple(T0 t0, T1 t1) :
      _timpl(
         move(t0), move(t1), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 3.
   tuple(T0 t0, T1 t1, T2 t2) :
      _timpl(
         move(t0), move(t1), move(t2), _tuple_void(), _tuple_void(), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 4.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), _tuple_void(), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 5.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 6.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), move(t5), _tuple_void(), _tuple_void(),
         _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 7.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), move(t5), move(t6), _tuple_void(),
         _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 8.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), move(t5), move(t6), move(t7),
         _tuple_void(), _tuple_void()
      ) {
   }
   // Overload for tuple of 9.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), move(t5), move(t6), move(t7), move(t8),
         _tuple_void()
      ) {
   }
   // Overload for tuple of 10.
   tuple(T0 t0, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9) :
      _timpl(
         move(t0), move(t1), move(t2), move(t3), move(t4), move(t5), move(t6), move(t7), move(t8),
         move(t9)
      ) {
   }
   tuple(tuple const & tpl) :
      _timpl(static_cast<_timpl const &>(tpl)) {
   }
   tuple(tuple && tpl) :
      _timpl(static_cast<_timpl &&>(tpl)) {
   }


   /** Assignment operator.

   tpl
      Source tuple.
   return
      *this.
   */
   tuple & operator=(tuple const & tpl) {
      _timpl::operator=(static_cast<_timpl const &>(tpl));
      return *this;
   }
   tuple & operator=(tuple && tpl) {
      _timpl::operator=(static_cast<_timpl &&>(tpl));
      return *this;
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::tuple_element


namespace abc {
namespace _std {

/** Defines as its member type the type of the Nth element in the tuple (C++11 § 20.4.2.5 “Tuple
helper classes”).
*/
template <size_t t_i, typename T>
struct tuple_element;

#ifdef ABC_CXX_VARIADIC_TEMPLATES

// Recursion: remove 1 from the index, and 1 item from the tuple.
template <size_t t_i, typename T0, typename ... Ts>
struct tuple_element<t_i, tuple<T0, Ts ...>> :
   public tuple_element<t_i - 1, tuple<Ts ...>> {
};

// Base recursion step.
template <typename T0, typename ... Ts>
struct tuple_element<0, tuple<T0, Ts ...>> {
   typedef T0 type;
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <size_t t_i, typename T>
struct tuple_element;

#define ABC_SPECIALIZE_tuple_element_FOR_INDEX(i) \
   template < \
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, \
      typename T7, typename T8, typename T9 \
   > \
   struct tuple_element<i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> { \
      typedef T ## i type; \
   };
ABC_SPECIALIZE_tuple_element_FOR_INDEX(0)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(1)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(2)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(3)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(4)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(5)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(6)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(7)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(8)
ABC_SPECIALIZE_tuple_element_FOR_INDEX(9)
#undef ABC_SPECIALIZE_tuple_element_FOR_INDEX

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::get (tuple)


namespace abc {
namespace _std {

#ifndef ABC_CXX_VARIADIC_TEMPLATES

/** Helper for get<>(tuple). Being a class, it can be partially specialized, which is necessary to
make it work.
*/
template <
   size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
struct _tuple_get_helper;

#define ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(i) \
   template < \
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, \
      typename T7, typename T8, typename T9 \
   > \
   struct _tuple_get_helper<i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> { \
   \
      inline static T ## i & get( \
         tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl \
      ) { \
         return static_cast<_tuple_head<i, T ## i> &>(tpl).get(); \
      } \
      inline static T ## i const & get( \
         tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl \
      ) { \
         return static_cast<_tuple_head<i, T ## i> const &>(tpl).get(); \
      } \
   };
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(0)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(1)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(2)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(3)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(4)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(5)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(6)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(7)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(8)
ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(9)
#undef ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX

#endif //ifndef ABC_CXX_VARIADIC_TEMPLATES


/** Retrieves an element from a tuple (C++11 § 20.4.2.6 “Element access”).

tpl
   Tuple from which to extract an element.
return
   Reference to the tuple element.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <size_t t_i, typename ... Ts>
inline typename tuple_element<t_i, tuple<Ts ...>>::type & get(tuple<Ts ...> & tpl) {
   return static_cast<_tuple_head<
      t_i, typename tuple_element<t_i, tuple<Ts ...>>::type
   > &>(tpl).get();
}
template <size_t t_i, typename ... Ts>
inline typename tuple_element<t_i, tuple<Ts ...>>::type const & get(tuple<Ts ...> const & tpl) {
   return static_cast<_tuple_head<
      t_i, typename tuple_element<t_i, tuple<Ts ...>>::type
   > const &>(tpl).get();
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline typename tuple_element<t_i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl
) {
   return _tuple_get_helper<t_i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}
template <
   size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline typename tuple_element<t_i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type const & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl
) {
   return _tuple_get_helper<t_i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::tuple_size (tuple)


namespace abc {
namespace _std {

/** Defines the member value as the size of the specified type (C++11 § 20.4.2.5 “Tuple helper
classes”).
*/
template <class T>
struct tuple_size;

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <class ... Ts>
struct tuple_size<tuple<Ts ...>> : std::integral_constant<size_t, sizeof ...(Ts)> {};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> :
   std::integral_constant<size_t, 10> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, _tuple_void>> :
   std::integral_constant<size_t, 9> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, _tuple_void, _tuple_void>> :
   std::integral_constant<size_t, 8> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, _tuple_void, _tuple_void, _tuple_void>> :
   std::integral_constant<size_t, 7> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, T5, _tuple_void, _tuple_void, _tuple_void, _tuple_void
>> :
   std::integral_constant<size_t, 6> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void
>> : std::integral_constant<size_t, 5> {};
template <typename T0, typename T1, typename T2, typename T3>
struct tuple_size<tuple<
   T0, T1, T2, T3, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void
>> : std::integral_constant<size_t, 4> {};
template <typename T0, typename T1, typename T2>
struct tuple_size<tuple<
   T0, T1, T2, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void,
   _tuple_void
>> : std::integral_constant<size_t, 3> {};
template <typename T0, typename T1>
struct tuple_size<tuple<
   T0, T1, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void,
   _tuple_void, _tuple_void
>> : std::integral_constant<size_t, 2> {};
template <typename T0>
struct tuple_size<tuple<
   T0, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void,
   _tuple_void, _tuple_void
>> : std::integral_constant<size_t, 1> {};
template <>
struct tuple_size<tuple<
   _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void, _tuple_void,
   _tuple_void, _tuple_void, _tuple_void
>> : std::integral_constant<size_t, 0> {};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::ignore


namespace abc {
namespace _std {

/** Internal (implementation-defined) type of ignore. It supports construction and assignment from
any type, and silently discards everything.
*/
class __ignore_t {
public:

   /** Constructor.
   */
   __ignore_t() {
   }
   __ignore_t(__ignore_t const &) {
   }
   template <typename T>
   __ignore_t(T const &) {
   }


   /** Assignment operator.
   */
   __ignore_t const & operator=(__ignore_t const &) const {
      return *this;
   }
   template <typename T>
   __ignore_t const & operator=(T const &) const {
      return *this;
   }
};


/** Used with tie(), it allows to ignore individual values in the tuple being unpacked (C++11 § 20.4
“Tuples”).
*/
extern __ignore_t const ignore;

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_std::tie


namespace abc {
namespace _std {

/** Supports unpacking a tuple into the specified variables (C++11 § 20.4.2.4 “Tuple creation
functions”).

ts
   Variables to unpack to.
return
   Tuple containing references to each argument.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
inline /*constexpr*/ tuple<Ts & ...> tie(Ts & ... ts) {
   return tuple<Ts & ...>(ts ...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename T0>
inline /*constexpr*/ tuple<T0 &> tie(T0 & t0) {
   return tuple<T0 &>(t0);
}
template <typename T0, typename T1>
inline /*constexpr*/ tuple<T0 &, T1 &> tie(T0 & t0, T1 & t1) {
   return tuple<T0 &, T1 &>(t0, t1);
}
template <typename T0, typename T1, typename T2>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &> tie(T0 & t0, T1 & t1, T2 & t2) {
   return tuple<T0 &, T1 &, T2 &>(t0, t1, t2);
}
template <typename T0, typename T1, typename T2, typename T3>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &> tie(T0 & t0, T1 & t1, T2 & t2, T3 & t3) {
   return tuple<T0 &, T1 &, T2 &, T3 &>(t0, t1, t2, t3);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &>(t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &>(t0, t1, t2, t3, t4, t5);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &>(t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &>(t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace _std
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABACLADE_STL_TUPLE_HXX


/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013, 2014, 2015
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
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#ifdef ABC_STLIMPL
   #include <abaclade/_std/utility.hxx>
#else
   #include <utility>
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ABC_CXX_VARIADIC_TEMPLATES

namespace abc { namespace _std { namespace detail {

//! “Null” type used to reduce the number of tuple items from the preset maximum.
struct tuple_void {};

}}} //namespace abc::_std::detail

#endif //ifndef ABC_CXX_VARIADIC_TEMPLATES

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace detail {

/*! Base for a tuple item. For empty T, it derives from T; otherwise, it has a T member. This allows
for empty base optimization (EBO), if the compiler is smart enough. */
template <std::size_t t_i, typename T, bool t_bEmpty = std::is_empty<T>::value>
class tuple_head;

// Specialization for empty types: enable EBO.
template <std::size_t t_i, typename T>
class tuple_head<t_i, T, true> : private T {
public:
   //! Default constructor.
   tuple_head() {
   }

   /*! Move constructor.

   @param th
      Source object.
   */
   tuple_head(tuple_head && th) :
      T(std::move(th.get())) {
   }

   /*! Copy constructor.

   @param th
      Source object.
   */
   tuple_head(tuple_head const & th) :
      T(th.get()) {
   }

   /*! Move constructor.

   @param th
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<t_i, U> && th) :
      T(std::move(th.get())) {
   }

   /*! Copy constructor.

   @param th
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<t_i, U> const & th) :
      T(th.get()) {
   }

   /*! Element-moving constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U && u) :
      T(std::forward<U>(u)) {
   }

   /*! Element-copying constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U const & u) :
      T(u) {
   }

   /*! Move-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head && th) {
      get() = std::move(th.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head const & th) {
      get() = th.get();
      return *this;
   }

   /*! Move-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<t_i, U> && th) {
      get() = std::move(th.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<t_i, U> const & th) {
      get() = th.get();
      return *this;
   }

   /*! Accessor to the wrapped element.

   @return
      Reference to the wrapped element.
   */
   T & get() {
      return *this;
   }

   /*! Const accessor to the wrapped element.

   @return
      Const reference to the wrapped element.
   */
   T const & get() const {
      return *this;
   }
};

// Specialization non non-empty types.
template <std::size_t t_i, typename T>
class tuple_head<t_i, T, false> {
public:
   //! Default constructor.
   tuple_head() {
   }

   /*! Move constructor.

   @param th
      Source object.
   */
   tuple_head(tuple_head && th) :
      m_t(std::move(th.get())) {
   }

   /*! Copy constructor.

   @param th
      Source object.
   */
   tuple_head(tuple_head const & th) :
      m_t(th.get()) {
   }

   /*! Move constructor.

   @param th
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<t_i, U> && th) :
      m_t(std::move(th.get())) {
#if ABC_HOST_CXX_MSC == 1600
   // MSC16 BUG: thinks that th is not used?!
   #pragma warning(suppress: 4100)
#endif
   }

   /*! Copy constructor.

   @param th
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<t_i, U> const & th) :
      m_t(th.get()) {
   }

   /*! Element-moving constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U && u) :
      m_t(std::forward<U>(u)) {
#if ABC_HOST_CXX_MSC == 1600
   // MSC16 BUG: thinks that u is not used?!
   #pragma warning(suppress: 4100)
#endif
   }

   /*! Element-copying constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U const & u) :
      m_t(u) {
   }

   /*! Move-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head && th) {
      get() = std::move(th.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head const & th) {
      get() = th.get();
      return *this;
   }

   /*! Move-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<t_i, U> && th) {
      get() = std::move(th.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param th
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<t_i, U> const & th) {
      get() = th.get();
      return *this;
   }

   /*! Accessor to the wrapped element.

   @return
      Reference to the wrapped element.
   */
   T & get() {
      return m_t;
   }

   /*! Const accessor to the wrapped element.

   @return
      Const reference to the wrapped element.
   */
   T const & get() const {
      return m_t;
   }

private:
   //! Internal T instance.
   T m_t;
};

}}} //namespace abc::_std::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace detail {

//! Internal implementation of tuple.
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <std::size_t t_i, typename... Ts>
class tuple_tail;

// Base case for the template recursion.
template <std::size_t t_i>
class tuple_tail<t_i> {
};

// Template recursion step.
template <std::size_t t_i, typename T0, typename... Ts>
class tuple_tail<t_i, T0, Ts...> :
   public tuple_head<t_i, T0>,
   public tuple_tail<t_i + 1, Ts...> {
private:
   typedef tuple_head<t_i, T0> _thead;
   typedef tuple_tail<t_i + 1, Ts...> _ttail;

public:
   //! Default constructor.
   tuple_tail() {
   }

   /*! Move constructor.

   @param tt
      Source object.
   */
   tuple_tail(tuple_tail && tt) :
      _thead(std::move(tt.get_thead())),
      _ttail(std::move(tt.get_ttail())) {
   }

   /*! Copy constructor.

   @param tt
      Source object.
   */
   tuple_tail(tuple_tail const & tt) :
      _thead(tt.get_thead()),
      _ttail(tt.get_ttail()) {
   }

   /*! Move constructor.

   @param tt
      Source object.
   */
   template <typename U0, typename... Us>
   tuple_tail(tuple_tail<t_i, U0, Us> && tt) :
      _thead(std::move(tt.get_thead())),
      _ttail(std::move(tt.get_ttail())) {
   }

   /*! Copy constructor.

   @param tt
      Source object.
   */
   template <typename U0, typename... Us>
   tuple_tail(tuple_tail<t_i, U0, Us> const & tt) :
      _thead(tt.get_thead()),
      _ttail(tt.get_ttail()) {
   }

   /*! Element-moving constructor.

   @param thead
      Source tuple head.
   @param us
      Source elements.
   */
   template <typename U0, typename... Us>
   explicit tuple_tail(U0 && thead, Us &&... us) :
      _thead(std::forward<U0>(thead)),
      _ttail(std::forward<Us>(us)...) {
   }

   /*! Element-copying constructor.

   @param thead
      Source tuple head.
   @param us
      Source elements.
   */
   template <typename U0, typename... Us>
   explicit tuple_tail(U0 const & thead, Us const &... us) :
      _thead(thead),
      _ttail(us...) {
   }

   /*! Move-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail && tt) {
      get_thead() = std::move(tt.get_thead());
      get_ttail() = std::move(tt.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }

   /*! Move-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple_tail & operator=(tuple_tail<Us...> && tt) {
      get_thead() = std::move(tt.get_thead());
      get_ttail() = std::move(tt.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple_tail & operator=(tuple_tail<Us...> const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }

   /*! Returns the embedded tuple_head.

   @return
      Const reference to the embedded tuple head.
   */
   _thead & get_thead() {
      return *this;
   }

   /*! Returns the embedded tuple_head.

   @return
      Const reference to the embedded tuple head.
   */
   _thead const & get_thead() const {
      return *this;
   }

   /*! Returns the embedded tuple_tail.

   @return
      Const reference to the embedded tuple tail.
   */
   _ttail & get_ttail() {
      return *this;
   }

   /*! Returns the embedded tuple_tail.

   @return
      Const reference to the embedded tuple tail.
   */
   _ttail const & get_ttail() const {
      return *this;
   }
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   std::size_t t_i,
   typename T0 = detail::tuple_void, typename T1 = detail::tuple_void,
   typename T2 = detail::tuple_void, typename T3 = detail::tuple_void,
   typename T4 = detail::tuple_void, typename T5 = detail::tuple_void,
   typename T6 = detail::tuple_void, typename T7 = detail::tuple_void,
   typename T8 = detail::tuple_void, typename T9 = detail::tuple_void
>
class tuple_tail :
   public tuple_head<t_i, T0>,
   public tuple_tail<t_i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, detail::tuple_void> {
private:
   typedef tuple_head<t_i, T0> _thead;
   typedef tuple_tail<t_i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, detail::tuple_void> _ttail;

public:
   //! Default constructor.
   tuple_tail() {
   }

   /*! Move constructor.

   @param tt
      Source object.
   */
   tuple_tail(tuple_tail && tt) :
      _thead(std::move(tt.get_thead())),
      _ttail(std::move(tt.get_ttail())) {
   }

   /*! Copy constructor.

   @param tt
      Source object.
   */
   tuple_tail(tuple_tail const & tt) :
      _thead(tt.get_thead()),
      _ttail(tt.get_ttail()) {
   }

   /*! Move constructor.

   @param tt
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail(tuple_tail<t_i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && tt) :
      _thead(std::move(tt.get_thead())),
      _ttail(std::move(tt.get_ttail())) {
   }

   /*! Copy constructor.

   @param tt
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail(tuple_tail<t_i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & tt) :
      _thead(tt.get_thead()),
      _ttail(tt.get_ttail()) {
   }

   /*! Element-moving constructor.

   @param u0...u9
      Source elements.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail(
      U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7, U8 && u8,
      U9 && u9
   ) :
      _thead(std::forward<U0>(u0)),
      _ttail(
         std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3), std::forward<U4>(u4),
         std::forward<U5>(u5), std::forward<U6>(u6), std::forward<U7>(u7), std::forward<U8>(u8),
         std::forward<U9>(u9), detail::tuple_void()
      ) {
   }

   /*! Element-copying constructor.

   @param u0...u9
      Source elements.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5,
      U6 const & u6, U7 const & u7, U8 const & u8, U9 const & u9
   ) :
      _thead(u0),
      _ttail(u1, u2, u3, u4, u5, u6, u7, u8, u9, detail::tuple_void()) {
   }

   /*! Move-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail && tt) {
      get_thead() = std::move(tt.get_thead());
      get_ttail() = std::move(tt.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }

   /*! Move-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail & operator=(tuple_tail<t_i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && tt) {
      get_thead() = std::move(tt.get_thead());
      get_ttail() = std::move(tt.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param tt
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple_tail & operator=(tuple_tail<t_i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & tt) {
      get_thead() = tt.get_thead();
      get_ttail() = tt.get_ttail();
      return *this;
   }

   /*! Returns the embedded tuple_head.

   @return
      Reference to the embedded tuple head.
   */
   _thead & get_thead() {
      return *this;
   }

   /*! Returns the embedded tuple_head.

   @return
      Const reference to the embedded tuple head.
   */
   _thead const & get_thead() const {
      return *this;
   }

   /*! Returns the embedded tuple_tail.

   @return
      Reference to the embedded tuple tail.
   */
   _ttail & get_ttail() {
      return *this;
   }

   /*! Returns the embedded tuple_tail.

   @return
      Const reference to the embedded tuple tail.
   */
   _ttail const & get_ttail() const {
      return *this;
   }
};

// Base case for the template recursion.
template <std::size_t t_i>
class tuple_tail<
   t_i,
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void
> {
public:
   //! Default constructor.
   tuple_tail() {
   }

   //! Copy constructor.
   tuple_tail(tuple_tail const &) {
   }

   //! Constructor.
   tuple_tail(
      detail::tuple_void const &, detail::tuple_void const &, detail::tuple_void const &,
      detail::tuple_void const &, detail::tuple_void const &, detail::tuple_void const &,
      detail::tuple_void const &, detail::tuple_void const &, detail::tuple_void const &,
      detail::tuple_void const &
   ) {
   }

   /*! Copy-assignment operator.

   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail const &) {
      return *this;
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}}} //namespace abc::_std::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Fixed-size ordered collection of heterogeneous objects (C++11 § 20.4.2 “Class template tuple”).
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
class tuple : public detail::tuple_tail<0, Ts...> {
private:
   typedef detail::tuple_tail<0, Ts...> _timpl;

public:
   //! Default constructor.
   /*constexpr*/ tuple() {
   }

   /*! Move constructor.

   @param tpl
      Source object.
   */
   tuple(tuple && tpl) :
      _timpl(static_cast<_timpl &&>(tpl)) {
   }

   /*! Copy constructor.

   @param tpl
      Source object.
   */
   tuple(tuple const & tpl) :
      _timpl(static_cast<_timpl const &>(tpl)) {
   }

   /*! Move constructor.

   @param tpl
      Source object.
   */
   template <typename... Us>
   tuple(tuple<Us...> && tpl) :
      _timpl(static_cast<detail::tuple_tail<0, Us...> &&>(tpl)) {
   }

   /*! Copy constructor.

   @param tpl
      Source object.
   */
   template <typename... Us>
   tuple(tuple<Us...> const & tpl) :
      _timpl(static_cast<detail::tuple_tail<0, Us...> const &>(tpl)) {
   }

   /*! Element-moving constructor.

   @param us
      Source elements.
   */
   template <typename... Us>
   explicit tuple(Us &&... us) :
      _timpl(std::forward<Us>(us)...) {
   }

   /*! Element-copying constructor.

   @param ts
      Source elements.
   */
   template <typename... Us>
   explicit tuple(Us const &... us) :
      _timpl(us...) {
   }

   /*! Move-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple && tpl) {
      _timpl::operator=(static_cast<_timpl &&>(tpl));
      return *this;
   }

   /*! Copy-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple const & tpl) {
      _timpl::operator=(static_cast<_timpl const &>(tpl));
      return *this;
   }

   /*! Move-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple & operator=(tuple<Us...> && tpl) {
      _timpl::operator=(static_cast<detail::tuple_tail<0, Us...> &&>(tpl));
      return *this;
   }

   /*! Copy-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple & operator=(tuple<Us...> const & tpl) {
      _timpl::operator=(static_cast<detail::tuple_tail<0, Us...> const &>(tpl));
      return *this;
   }
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0 = detail::tuple_void, typename T1 = detail::tuple_void,
   typename T2 = detail::tuple_void, typename T3 = detail::tuple_void,
   typename T4 = detail::tuple_void, typename T5 = detail::tuple_void,
   typename T6 = detail::tuple_void, typename T7 = detail::tuple_void,
   typename T8 = detail::tuple_void, typename T9 = detail::tuple_void
>
class tuple : public detail::tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> {
private:
   typedef detail::tuple_void _tvoid;
   typedef detail::tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _timpl;

public:
   //! Default constructor.
   /*constexpr*/ tuple() {
   }

   /*! Move constructor.

   @param tpl
      Source object.
   */
   tuple(tuple && tpl) :
      _timpl(static_cast<_timpl &&>(tpl)) {
   }

   /*! Copy constructor.

   @param tpl
      Source object.
   */
   tuple(tuple const & tpl) :
      _timpl(static_cast<_timpl const &>(tpl)) {
   }

   /*! Move constructor.

   @param tpl
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && tpl) :
      _timpl(static_cast<detail::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> &&>(tpl)) {
   }

   /*! Copy constructor.

   @param tpl
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & tpl) :
      _timpl(
         static_cast<detail::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const &>(tpl)
      ) {
   }

   /*! Element-moving constructor.

   Note: every extra element is initialized to _tvoid() to ensure one can’t select an overload with
   a number of argument that doesn’t match the element count of the tuple.

   @param u0...u9
      Source elements.
   */
   template <typename U0>
   explicit tuple(U0 && u0) :
      _timpl(
         std::forward<U0>(u0), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1>
   tuple(U0 && u0, U1 && u1) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2>
   tuple(U0 && u0, U1 && u1, U2 && u2) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), _tvoid(), _tvoid(),
         _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), std::forward<U5>(u5), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), std::forward<U5>(u5), std::forward<U6>(u6), _tvoid(), _tvoid(),
         _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), std::forward<U5>(u5), std::forward<U6>(u6), std::forward<U7>(u7),
         _tvoid(), _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7, U8 && u8) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), std::forward<U5>(u5), std::forward<U6>(u6), std::forward<U7>(u7),
         std::forward<U8>(u8), _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(
      U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7, U8 && u8,
      U9 && u9
   ) :
      _timpl(
         std::forward<U0>(u0), std::forward<U1>(u1), std::forward<U2>(u2), std::forward<U3>(u3),
         std::forward<U4>(u4), std::forward<U5>(u5), std::forward<U6>(u6), std::forward<U7>(u7),
         std::forward<U8>(u8), std::forward<U9>(u9)
      ) {
   }

   /*! Element-copying constructor.

   Note: every extra element is initialized to _tvoid() to ensure one can’t select an overload with
   a number of argument that doesn’t match the element count of the tuple.

   @param u0...u9
      Source elements.
   */
   template <typename U0>
   explicit tuple(U0 const & u0) :
      _timpl(
         u0, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid()
      ) {
   }
   template <typename U0, typename U1>
   tuple(U0 const & u0, U1 const & u1) :
      _timpl(
         u0, u1, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2>
   tuple(U0 const & u0, U1 const & u1, U2 const & u2) :
      _timpl(u0, u1, u2, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   tuple(U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3) :
      _timpl(u0, u1, u2, u3, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   tuple(U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4) :
      _timpl(u0, u1, u2, u3, u4, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   tuple(U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5) :
      _timpl(u0, u1, u2, u3, u4, u5, _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5,
      U6 const & u6
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, _tvoid(), _tvoid(), _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5,
      U6 const & u6, U7 const & u7
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, _tvoid(), _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5,
      U6 const & u6, U7 const & u7, U8 const & u8
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, u8, _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5,
      U6 const & u6, U7 const & u7, U8 const & u8, U9 const & u9
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9) {
   }

   /*! Copy-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple const & tpl) {
      _timpl::operator=(static_cast<_timpl const &>(tpl));
      return *this;
   }

   /*! Move-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple && tpl) {
      _timpl::operator=(static_cast<_timpl &&>(tpl));
      return *this;
   }

   /*! Copy-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple & operator=(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & tpl) {
      _timpl::operator=(static_cast<detail::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const &>(tpl));
      return *this;
   }

   /*! Move-assignment operator.

   @param tpl
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple & operator=(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && tpl) {
      _timpl::operator=(static_cast<detail::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> &&>(tpl));
      return *this;
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/*! Defines as its member type the type of the Nth element in the tuple (C++11 § 20.4.2.5 “Tuple
helper classes”). */
template <std::size_t t_i, typename T>
struct tuple_element;

#ifdef ABC_CXX_VARIADIC_TEMPLATES

// Recursion: remove 1 from the index, and 1 item from the tuple.
template <std::size_t t_i, typename T0, typename... Ts>
struct tuple_element<t_i, tuple<T0, Ts...>> : public tuple_element<t_i - 1, tuple<Ts...>> {
};

// Base recursion step.
template <typename T0, typename... Ts>
struct tuple_element<0, tuple<T0, Ts...>> {
   typedef T0 type;
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <std::size_t t_i, typename T>
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

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ABC_CXX_VARIADIC_TEMPLATES

namespace abc { namespace _std { namespace detail {

/*! Helper for get<>(tuple). Being a class, it can be partially specialized, which is necessary to
make it work. */
template <
   std::size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
struct tuple_get_helper;

#define ABC_SPECIALIZE_tuple_get_helper_FOR_INDEX(i) \
   template < \
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, \
      typename T7, typename T8, typename T9 \
   > \
   struct tuple_get_helper<i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> { \
      inline static T ## i & get( \
         tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl \
      ) { \
         return static_cast<tuple_head<i, T ## i> &>(tpl).get(); \
      } \
      inline static T ## i const & get( \
         tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl \
      ) { \
         return static_cast<tuple_head<i, T ## i> const &>(tpl).get(); \
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

}}} //namespace abc::_std::detail

#endif //ifndef ABC_CXX_VARIADIC_TEMPLATES

namespace abc { namespace _std {

/*! Retrieves an element from a tuple (C++11 § 20.4.2.6 “Element access”).

@param tpl
   Tuple from which to extract an element.
@return
   Reference to the tuple element.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <std::size_t t_i, typename... Ts>
inline typename tuple_element<t_i, tuple<Ts...>>::type & get(tuple<Ts...> & tpl) {
   return static_cast<tuple_head<
      t_i, typename tuple_element<t_i, tuple<Ts...>>::type
   > &>(tpl).get();
}
template <std::size_t t_i, typename... Ts>
inline typename tuple_element<t_i, tuple<Ts...>>::type const & get(tuple<Ts...> const & tpl) {
   return static_cast<tuple_head<
      t_i, typename tuple_element<t_i, tuple<Ts...>>::type
   > const &>(tpl).get();
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   std::size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline typename tuple_element<t_i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl
) {
   return detail::tuple_get_helper<t_i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}
template <
   std::size_t t_i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5,
   typename T6, typename T7, typename T8, typename T9
>
inline typename tuple_element<t_i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type const & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl
) {
   return detail::tuple_get_helper<t_i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/*! Defines the member value as the size of the specified type (C++11 § 20.4.2.5 “Tuple helper
classes”). */
template <class T>
struct tuple_size;

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <class... Ts>
struct tuple_size<tuple<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> :
   std::integral_constant<std::size_t, 10> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, detail::tuple_void>> :
   std::integral_constant<std::size_t, 9> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, detail::tuple_void, detail::tuple_void>> :
   std::integral_constant<std::size_t, 8> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, T5, T6, detail::tuple_void, detail::tuple_void, detail::tuple_void
>> :
   std::integral_constant<std::size_t, 7> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, T5, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void
>> : std::integral_constant<std::size_t, 6> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void
>> : std::integral_constant<std::size_t, 5> {};
template <typename T0, typename T1, typename T2, typename T3>
struct tuple_size<tuple<
   T0, T1, T2, T3, detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void
>> : std::integral_constant<std::size_t, 4> {};
template <typename T0, typename T1, typename T2>
struct tuple_size<tuple<
   T0, T1, T2, detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void, detail::tuple_void
>> : std::integral_constant<std::size_t, 3> {};
template <typename T0, typename T1>
struct tuple_size<tuple<
   T0, T1, detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void
>> : std::integral_constant<std::size_t, 2> {};
template <typename T0>
struct tuple_size<tuple<
   T0, detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void
>> : std::integral_constant<std::size_t, 1> {};
template <>
struct tuple_size<tuple<
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void, detail::tuple_void, detail::tuple_void,
   detail::tuple_void, detail::tuple_void
>> : std::integral_constant<std::size_t, 0> {};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

// TODO: comment.
// TODO: complete and move to <type_traits>.
template <typename T>
struct decay {
    typedef typename std::remove_reference<T>::type U;
    typedef typename std::conditional<
        std::is_array<U>::value,
        typename std::remove_extent<U>::type *,
        typename std::conditional<
            std::is_function<U>::value,
            typename std::add_pointer<U>::type,
            typename std::remove_cv<U>::type
        >::type
    >::type type;
};

/*! TODO: comment.

@param ts
   Variables to store in a tuple.
@return
   Tuple containing each argument.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline /*constexpr*/ tuple<Ts...> make_tuple(Ts &&... ts) {
   return tuple<Ts...>(std::forward<Ts>(ts)...);
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline /*constexpr*/ tuple<> make_tuple() {
   return tuple<>();
}
template <typename T0>
inline /*constexpr*/ tuple<typename decay<T0>::type> make_tuple(T0 && t0) {
   return tuple<typename decay<T0>::type>(std::forward<T0>(t0));
}
template <typename T0, typename T1>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type
> make_tuple(T0 && t0, T1 && t1) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type
   >(std::forward<T0>(t0), std::forward<T1>(t1));
}
template <typename T0, typename T1, typename T2>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type
   >(std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2));
}
template <typename T0, typename T1, typename T2, typename T3>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type
   >(std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3));
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4)
   );
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4), std::forward<T5>(t5)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
   typename decay<T6>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
      typename decay<T6>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4), std::forward<T5>(t5), std::forward<T6>(t6)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
   typename decay<T6>::type, typename decay<T7>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
      typename decay<T6>::type, typename decay<T7>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4), std::forward<T5>(t5), std::forward<T6>(t6), std::forward<T7>(t7)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
   typename decay<T6>::type, typename decay<T7>::type, typename decay<T8>::type
> make_tuple(
   T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8
) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
      typename decay<T6>::type, typename decay<T7>::type, typename decay<T8>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4), std::forward<T5>(t5), std::forward<T6>(t6), std::forward<T7>(t7),
      std::forward<T8>(t8)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
   typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
   typename decay<T6>::type, typename decay<T7>::type, typename decay<T8>::type,
   typename decay<T9>::type
> make_tuple(
   T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8,
   T9 && t9
) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type,
      typename decay<T3>::type, typename decay<T4>::type, typename decay<T5>::type,
      typename decay<T6>::type, typename decay<T7>::type, typename decay<T8>::type,
      typename decay<T9>::type
   >(
      std::forward<T0>(t0), std::forward<T1>(t1), std::forward<T2>(t2), std::forward<T3>(t3),
      std::forward<T4>(t4), std::forward<T5>(t5), std::forward<T6>(t6), std::forward<T7>(t7),
      std::forward<T8>(t8), std::forward<T9>(t9)
   );
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace detail {

/*! Internal (implementation-defined) type of ignore. It supports construction and assignment from
any type, and silently discards everything. */
class ignore_t {
public:
   //! Constructor.
   ignore_t() {
   }
   ignore_t(ignore_t const &) {
   }
   template <typename T>
   ignore_t(T const &) {
   }

   //! Assignment operator.
   ignore_t const & operator=(ignore_t const &) const {
      return *this;
   }
   template <typename T>
   ignore_t const & operator=(T const &) const {
      return *this;
   }
};

}}} //namespace abc::_std::detail

namespace abc { namespace _std {

/*! Used with tie(), it allows to ignore individual values in the tuple being unpacked (C++11 § 20.4
“Tuples”). */
extern detail::ignore_t const ignore;

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/*! Supports unpacking a tuple into the specified variables (C++11 § 20.4.2.4 “Tuple creation
functions”).

@param ts
   Variables to unpack to.
@return
   Tuple containing references to each argument.
*/
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline /*constexpr*/ tuple<Ts &...> tie(Ts &... ts) {
   return tuple<Ts &...>(ts...);
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

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STL_TUPLE_HXX

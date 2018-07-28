/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_TUPLE_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_STD_TUPLE_HXX
#endif

#ifndef _LOFTY_STD_TUPLE_HXX_NOPUB
#define _LOFTY_STD_TUPLE_HXX_NOPUB

#include <lofty/_pvt/lofty.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_STL_LOFTY || !defined(LOFTY_CXX_VARIADIC_TEMPLATES)

#include <lofty/_std/type_traits.hxx>
#include <lofty/_std/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LOFTY_CXX_VARIADIC_TEMPLATES

namespace lofty { namespace _std { namespace _pvt {

//! “Null” type used to reduce the number of tuple items from the preset maximum.
struct tuple_void {};

}}}

#endif //ifndef LOFTY_CXX_VARIADIC_TEMPLATES

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

/*! Base for a tuple item. For empty T, it derives from T; otherwise, it has a T member. This allows for empty
base optimization (EBO), if the compiler is smart enough. */
template <std::size_t i, typename T, bool empty = is_empty<T>::value>
class tuple_head;

// Specialization for empty types: enable EBO.
template <std::size_t i, typename T>
class tuple_head<i, T, true> : private T {
public:
   //! Default constructor.
   tuple_head() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple_head(tuple_head && src) :
      T(move(src.get())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple_head(tuple_head const & src) :
      T(src.get()) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<i, U> && src) :
      T(move(src.get())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<i, U> const & src) :
      T(src.get()) {
   }

   /*! Element-moving constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U && u) :
      T(forward<U>(u)) {
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

   @param src
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head && src) {
      get() = move(src.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head const & src) {
      get() = src.get();
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<i, U> && src) {
      get() = move(src.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<i, U> const & src) {
      get() = src.get();
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
template <std::size_t i, typename T>
class tuple_head<i, T, false> {
public:
   //! Default constructor.
   tuple_head() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple_head(tuple_head && src) :
      t(move(src.get())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple_head(tuple_head const & src) :
      t(src.get()) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<i, U> && src) :
      t(move(src.get())) {
#if LOFTY_HOST_CXX_MSC == 1600
   // MSC16 BUG: thinks that src is not used?!
   #pragma warning(suppress: 4100)
#endif
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename U>
   tuple_head(tuple_head<i, U> const & src) :
      t(src.get()) {
   }

   /*! Element-moving constructor.

   @param u
      Source element.
   */
   template <typename U>
   explicit tuple_head(U && u) :
      t(forward<U>(u)) {
#if LOFTY_HOST_CXX_MSC == 1600
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
      t(u) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head && src) {
      get() = move(src.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_head & operator=(tuple_head const & src) {
      get() = src.get();
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<i, U> && src) {
      get() = move(src.get());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename U>
   tuple_head & operator=(tuple_head<i, U> const & src) {
      get() = src.get();
      return *this;
   }

   /*! Accessor to the wrapped element.

   @return
      Reference to the wrapped element.
   */
   T & get() {
      return t;
   }

   /*! Const accessor to the wrapped element.

   @return
      Const reference to the wrapped element.
   */
   T const & get() const {
      return t;
   }

private:
   //! Internal T instance.
   T t;
};

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

//! Internal implementation of tuple.
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <std::size_t i, typename... Ts>
class tuple_tail;

// Base case for the template recursion.
template <std::size_t i>
class tuple_tail<i> {
};

// Template recursion step.
template <std::size_t i, typename T0, typename... Ts>
class tuple_tail<i, T0, Ts...> :
   public tuple_head<i, T0>,
   public tuple_tail<i + 1, Ts...> {
private:
   typedef tuple_head<i, T0> _thead;
   typedef tuple_tail<i + 1, Ts...> _ttail;

public:
   //! Default constructor.
   tuple_tail() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple_tail(tuple_tail && src) :
      _thead(move(src.get_thead())),
      _ttail(move(src.get_ttail())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple_tail(tuple_tail const & src) :
      _thead(src.get_thead()),
      _ttail(src.get_ttail()) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <typename U0, typename... Us>
   tuple_tail(tuple_tail<i, U0, Us> && src) :
      _thead(move(src.get_thead())),
      _ttail(move(src.get_ttail())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename U0, typename... Us>
   tuple_tail(tuple_tail<i, U0, Us> const & src) :
      _thead(src.get_thead()),
      _ttail(src.get_ttail()) {
   }

   /*! Element-moving constructor.

   @param thead
      Source tuple head.
   @param us
      Source elements.
   */
   template <typename U0, typename... Us>
   explicit tuple_tail(U0 && thead, Us &&... us) :
      _thead(forward<U0>(thead)),
      _ttail(forward<Us>(us)...) {
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

   @param src
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail && src) {
      get_thead() = move(src.get_thead());
      get_ttail() = move(src.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail const & src) {
      get_thead() = src.get_thead();
      get_ttail() = src.get_ttail();
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple_tail & operator=(tuple_tail<Us...> && src) {
      get_thead() = move(src.get_thead());
      get_ttail() = move(src.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple_tail & operator=(tuple_tail<Us...> const & src) {
      get_thead() = src.get_thead();
      get_ttail() = src.get_ttail();
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

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <
   std::size_t i,
   typename T0 = _pvt::tuple_void, typename T1 = _pvt::tuple_void, typename T2 = _pvt::tuple_void,
   typename T3 = _pvt::tuple_void, typename T4 = _pvt::tuple_void, typename T5 = _pvt::tuple_void,
   typename T6 = _pvt::tuple_void, typename T7 = _pvt::tuple_void, typename T8 = _pvt::tuple_void,
   typename T9 = _pvt::tuple_void
>
class tuple_tail :
   public tuple_head<i, T0>,
   public tuple_tail<i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, _pvt::tuple_void> {
private:
   typedef tuple_head<i, T0> _thead;
   typedef tuple_tail<i + 1, T1, T2, T3, T4, T5, T6, T7, T8, T9, _pvt::tuple_void> _ttail;

public:
   //! Default constructor.
   tuple_tail() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple_tail(tuple_tail && src) :
      _thead(move(src.get_thead())),
      _ttail(move(src.get_ttail())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple_tail(tuple_tail const & src) :
      _thead(src.get_thead()),
      _ttail(src.get_ttail()) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail(tuple_tail<i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && src) :
      _thead(move(src.get_thead())),
      _ttail(move(src.get_ttail())) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail(tuple_tail<i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & src) :
      _thead(src.get_thead()),
      _ttail(src.get_ttail()) {
   }

   /*! Element-moving constructor.

   @param u0...u9
      Source elements.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail(
      U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7, U8 && u8, U9 && u9
   ) :
      _thead(forward<U0>(u0)),
      _ttail(
         forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4), forward<U5>(u5), forward<U6>(u6),
         forward<U7>(u7), forward<U8>(u8), forward<U9>(u9), _pvt::tuple_void()
      ) {
   }

   /*! Element-copying constructor.

   @param u0...u9
      Source elements.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5, U6 const & u6,
      U7 const & u7, U8 const & u8, U9 const & u9
   ) :
      _thead(u0),
      _ttail(u1, u2, u3, u4, u5, u6, u7, u8, u9, _pvt::tuple_void()) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail && src) {
      get_thead() = move(src.get_thead());
      get_ttail() = move(src.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple_tail & operator=(tuple_tail const & src) {
      get_thead() = src.get_thead();
      get_ttail() = src.get_ttail();
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail & operator=(tuple_tail<i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && src) {
      get_thead() = move(src.get_thead());
      get_ttail() = move(src.get_ttail());
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple_tail & operator=(tuple_tail<i, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & src) {
      get_thead() = src.get_thead();
      get_ttail() = src.get_ttail();
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
template <std::size_t i>
class tuple_tail<
   i, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void
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
      _pvt::tuple_void const &, _pvt::tuple_void const &, _pvt::tuple_void const &, _pvt::tuple_void const &,
      _pvt::tuple_void const &, _pvt::tuple_void const &, _pvt::tuple_void const &, _pvt::tuple_void const &,
      _pvt::tuple_void const &, _pvt::tuple_void const &
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

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

}}} //namespace lofty::_std::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Fixed-size ordered collection of heterogeneous objects (C++11 § 20.4.2 “Class template tuple”).
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
class tuple : public _pvt::tuple_tail<0, Ts...> {
private:
   typedef _pvt::tuple_tail<0, Ts...> _timpl;

public:
   //! Default constructor.
   /*constexpr*/ tuple() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple(tuple && src) :
      _timpl(static_cast<_timpl &&>(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple(tuple const & src) :
      _timpl(static_cast<_timpl const &>(src)) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <typename... Us>
   tuple(tuple<Us...> && src) :
      _timpl(static_cast<_pvt::tuple_tail<0, Us...> &&>(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <typename... Us>
   tuple(tuple<Us...> const & src) :
      _timpl(static_cast<_pvt::tuple_tail<0, Us...> const &>(src)) {
   }

   /*! Element-moving constructor.

   @param us
      Source elements.
   */
   template <typename... Us>
   explicit tuple(Us &&... us) :
      _timpl(forward<Us>(us)...) {
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

   @param src
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple && src) {
      _timpl::operator=(static_cast<_timpl &&>(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple const & src) {
      _timpl::operator=(static_cast<_timpl const &>(src));
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple & operator=(tuple<Us...> && src) {
      _timpl::operator=(static_cast<_pvt::tuple_tail<0, Us...> &&>(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <typename... Us>
   tuple & operator=(tuple<Us...> const & src) {
      _timpl::operator=(static_cast<_pvt::tuple_tail<0, Us...> const &>(src));
      return *this;
   }
};

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <
   typename T0 = _pvt::tuple_void, typename T1 = _pvt::tuple_void, typename T2 = _pvt::tuple_void,
   typename T3 = _pvt::tuple_void, typename T4 = _pvt::tuple_void, typename T5 = _pvt::tuple_void,
   typename T6 = _pvt::tuple_void, typename T7 = _pvt::tuple_void, typename T8 = _pvt::tuple_void,
   typename T9 = _pvt::tuple_void
>
class tuple : public _pvt::tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> {
private:
   typedef _pvt::tuple_void _tvoid;
   typedef _pvt::tuple_tail<0, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> _timpl;

public:
   //! Default constructor.
   /*constexpr*/ tuple() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   tuple(tuple && src) :
      _timpl(static_cast<_timpl &&>(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   tuple(tuple const & src) :
      _timpl(static_cast<_timpl const &>(src)) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && src) :
      _timpl(static_cast<_pvt::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> &&>(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   tuple(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & src) :
      _timpl(
         static_cast<_pvt::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const &>(src)
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
         forward<U0>(u0), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1>
   tuple(U0 && u0, U1 && u1) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2>
   tuple(U0 && u0, U1 && u1, U2 && u2) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), _tvoid(), _tvoid(), _tvoid(), _tvoid(),
         _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), _tvoid(), _tvoid(),
         _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         forward<U5>(u5), _tvoid(), _tvoid(), _tvoid(), _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         forward<U5>(u5), forward<U6>(u6), _tvoid(), _tvoid(),
         _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         forward<U5>(u5), forward<U6>(u6), forward<U7>(u7), _tvoid(), _tvoid()
      ) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8
   >
   tuple(U0 && u0, U1 && u1, U2 && u2, U3 && u3, U4 && u4, U5 && u5, U6 && u6, U7 && u7, U8 && u8) :
      _timpl(
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         forward<U5>(u5), forward<U6>(u6), forward<U7>(u7), forward<U8>(u8), _tvoid()
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
         forward<U0>(u0), forward<U1>(u1), forward<U2>(u2), forward<U3>(u3), forward<U4>(u4),
         forward<U5>(u5), forward<U6>(u6), forward<U7>(u7), forward<U8>(u8), forward<U9>(u9)
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
      _timpl(u0, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
   }
   template <typename U0, typename U1>
   tuple(U0 const & u0, U1 const & u1) :
      _timpl(u0, u1, _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid(), _tvoid()) {
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
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5, U6 const & u6
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, _tvoid(), _tvoid(), _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5, U6 const & u6,
      U7 const & u7
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, _tvoid(), _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5, U6 const & u6,
      U7 const & u7, U8 const & u8
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, u8, _tvoid()) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple(
      U0 const & u0, U1 const & u1, U2 const & u2, U3 const & u3, U4 const & u4, U5 const & u5, U6 const & u6,
      U7 const & u7, U8 const & u8, U9 const & u9
   ) :
      _timpl(u0, u1, u2, u3, u4, u5, u6, u7, u8, u9) {
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple const & src) {
      _timpl::operator=(static_cast<_timpl const &>(src));
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   tuple & operator=(tuple && src) {
      _timpl::operator=(static_cast<_timpl &&>(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple & operator=(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const & src) {
      _timpl::operator=(
         static_cast<_pvt::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> const &>(src)
      );
      return *this;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   tuple & operator=(tuple<U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> && src) {
      _timpl::operator=(static_cast<_pvt::tuple_tail<0, U0, U1, U2, U3, U4, U5, U6, U7, U8, U9> &&>(src));
      return *this;
   }
};

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Defines as its member type the type of the Nth element in the tuple (C++11 § 20.4.2.5 “Tuple helper
classes”). */
template <std::size_t i, typename T>
struct tuple_element;

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

// Recursion: remove 1 from the index, and 1 item from the tuple.
template <std::size_t i, typename T0, typename... Ts>
struct tuple_element<i, tuple<T0, Ts...>> : public tuple_element<i - 1, tuple<Ts...>> {
};

// Base recursion step.
template <typename T0, typename... Ts>
struct tuple_element<0, tuple<T0, Ts...>> {
   typedef T0 type;
};

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <std::size_t i, typename T>
struct tuple_element;

#define LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(i) \
   template < \
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, \
      typename T7, typename T8, typename T9 \
   > \
   struct tuple_element<i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> { \
      typedef T ## i type; \
   };
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(0)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(1)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(2)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(3)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(4)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(5)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(6)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(7)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(8)
LOFTY_SPECIALIZE_tuple_element_FOR_INDEX(9)
#undef LOFTY_SPECIALIZE_tuple_element_FOR_INDEX

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef LOFTY_CXX_VARIADIC_TEMPLATES

namespace lofty { namespace _std { namespace _pvt {

/*! Helper for get<>(tuple). Being a class, it can be partially specialized, which is necessary to make it
work. */
template <
   std::size_t i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
struct tuple_get_helper;

#define LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(i) \
   template < \
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, \
      typename T7, typename T8, typename T9 \
   > \
   struct tuple_get_helper<i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> { \
      inline static T ## i & get(tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl) { \
         return static_cast<tuple_head<i, T ## i> &>(tpl).get(); \
      } \
      inline static T ## i const & get(tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl) { \
         return static_cast<tuple_head<i, T ## i> const &>(tpl).get(); \
      } \
   };
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(0)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(1)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(2)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(3)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(4)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(5)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(6)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(7)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(8)
LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX(9)
#undef LOFTY_SPECIALIZE_tuple_get_helper_FOR_INDEX

}}} //namespace lofty::_std::_pvt

#endif //ifndef LOFTY_CXX_VARIADIC_TEMPLATES

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Retrieves an element from a tuple (C++11 § 20.4.2.6 “Element access”).

@param tpl
   Tuple from which to extract an element.
@return
   Reference to the tuple element.
*/
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <std::size_t i, typename... Ts>
inline typename tuple_element<i, tuple<Ts...>>::type & get(tuple<Ts...> & tpl) {
   return static_cast<tuple_head<i, typename tuple_element<i, tuple<Ts...>>::type> &>(tpl).get();
}
template <std::size_t i, typename... Ts>
inline typename tuple_element<i, tuple<Ts...>>::type const & get(tuple<Ts...> const & tpl) {
   return static_cast<tuple_head<i, typename tuple_element<i, tuple<Ts...>>::type> const &>(tpl).get();
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <
   std::size_t i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline typename tuple_element<i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> & tpl
) {
   return _pvt::tuple_get_helper<i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}
template <
   std::size_t i, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline typename tuple_element<i, tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>>::type const & get(
   tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> const & tpl
) {
   return _pvt::tuple_get_helper<i, T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>::get(tpl);
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

//! Defines the member value as the size of the specified type (C++11 § 20.4.2.5 “Tuple helper classes”).
template <class T>
struct tuple_size;

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <class... Ts>
struct tuple_size<tuple<Ts...>> : integral_constant<std::size_t, sizeof...(Ts)> {};

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>> : integral_constant<std::size_t, 10> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, _pvt::tuple_void>> :
   integral_constant<std::size_t, 9> {};
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, T7, _pvt::tuple_void, _pvt::tuple_void>> :
   integral_constant<std::size_t, 8> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
struct tuple_size<tuple<T0, T1, T2, T3, T4, T5, T6, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void>> :
   integral_constant<std::size_t, 7> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, T5, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void
>> : integral_constant<std::size_t, 6> {};
template <typename T0, typename T1, typename T2, typename T3, typename T4>
struct tuple_size<tuple<
   T0, T1, T2, T3, T4, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void
>> : integral_constant<std::size_t, 5> {};
template <typename T0, typename T1, typename T2, typename T3>
struct tuple_size<tuple<
   T0, T1, T2, T3, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void
>> : integral_constant<std::size_t, 4> {};
template <typename T0, typename T1, typename T2>
struct tuple_size<tuple<
   T0, T1, T2, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void, _pvt::tuple_void
>> : integral_constant<std::size_t, 3> {};
template <typename T0, typename T1>
struct tuple_size<tuple<
   T0, T1, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void
>> : integral_constant<std::size_t, 2> {};
template <typename T0>
struct tuple_size<tuple<
   T0, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void
>> : integral_constant<std::size_t, 1> {};
template <>
struct tuple_size<tuple<
   _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void,
   _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void, _pvt::tuple_void
>> : integral_constant<std::size_t, 0> {};

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Creates a tuple from the specified values, inferring their types automatically (C++11 § 20.4.2.4
“Tuple creation functions”).

@param ts
   Variables to store in a tuple.
@return
   Tuple containing each argument.
*/
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline /*constexpr*/ tuple<typename decay<Ts>::type...> make_tuple(Ts &&... ts) {
   return tuple<Ts...>(forward<Ts>(ts)...);
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

inline /*constexpr*/ tuple<> make_tuple() {
   return tuple<>();
}
template <typename T0>
inline /*constexpr*/ tuple<typename decay<T0>::type> make_tuple(T0 && t0) {
   return tuple<typename decay<T0>::type>(forward<T0>(t0));
}
template <typename T0, typename T1>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type
> make_tuple(T0 && t0, T1 && t1) {
   return tuple<typename decay<T0>::type, typename decay<T1>::type>(forward<T0>(t0), forward<T1>(t1));
}
template <typename T0, typename T1, typename T2>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type
   >(forward<T0>(t0), forward<T1>(t1), forward<T2>(t2));
}
template <typename T0, typename T1, typename T2, typename T3>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type
   >(forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3));
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type
   >(forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4));
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type, typename decay<T5>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type, typename decay<T5>::type
   >(forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4), forward<T5>(t5));
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type
   >(
      forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4), forward<T5>(t5),
      forward<T6>(t6)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type
   >(
      forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4), forward<T5>(t5),
      forward<T6>(t6), forward<T7>(t7)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type,
   typename decay<T8>::type
> make_tuple(T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type,
      typename decay<T8>::type
   >(
      forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4), forward<T5>(t5),
      forward<T6>(t6), forward<T7>(t7), forward<T8>(t8)
   );
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline /*constexpr*/ tuple<
   typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
   typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type,
   typename decay<T8>::type, typename decay<T9>::type
> make_tuple(
   T0 && t0, T1 && t1, T2 && t2, T3 && t3, T4 && t4, T5 && t5, T6 && t6, T7 && t7, T8 && t8, T9 && t9
) {
   return tuple<
      typename decay<T0>::type, typename decay<T1>::type, typename decay<T2>::type, typename decay<T3>::type,
      typename decay<T4>::type, typename decay<T5>::type, typename decay<T6>::type, typename decay<T7>::type,
      typename decay<T8>::type, typename decay<T9>::type
   >(
      forward<T0>(t0), forward<T1>(t1), forward<T2>(t2), forward<T3>(t3), forward<T4>(t4), forward<T5>(t5),
      forward<T6>(t6), forward<T7>(t7), forward<T8>(t8), forward<T9>(t9)
   );
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

/*! Internal (implementation-defined) type of ignore. It supports construction and assignment from any type,
and silently discards everything. */
class ignore_t {
public:
   //! Default constructor.
   ignore_t() {
   }

   //! Copy constructor.
   ignore_t(ignore_t const &) {
   }

   //! Constructor from any type.
   template <typename T>
   ignore_t(T const &) {
   }

   /*! Copy-assignment operator.

   @return
      *this.
   */
   ignore_t const & operator=(ignore_t const &) const {
      return *this;
   }

   /*! Assignment operator from any type.

   @return
      *this.
   */
   template <typename T>
   ignore_t const & operator=(T const &) const {
      return *this;
   }
};

}}} //namespace lofty::_std::_pvt

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Used with tie(), it allows to ignore individual values in the tuple being unpacked (C++11 § 20.4
“Tuples”). */
extern _pvt::ignore_t const ignore;

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {
_LOFTY_PUBNS_BEGIN

/*! Supports unpacking a tuple into the specified variables (C++11 § 20.4.2.4 “Tuple creation functions”).

@param ts
   Variables to unpack to.
@return
   Tuple containing references to each argument.
*/
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline /*constexpr*/ tuple<Ts &...> tie(Ts &... ts) {
   return tuple<Ts &...>(ts...);
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

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
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &> tie(T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &>(t0, t1, t2, t3, t4);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &>(t0, t1, t2, t3, t4, t5);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &>(t0, t1, t2, t3, t4, t5, t6);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &>(t0, t1, t2, t3, t4, t5, t6, t7);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &>(t0, t1, t2, t3, t4, t5, t6, t7, t8);
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline /*constexpr*/ tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &> tie(
   T0 & t0, T1 & t1, T2 & t2, T3 & t3, T4 & t4, T5 & t5, T6 & t6, T7 & t7, T8 & t8, T9 & t9
) {
   return tuple<T0 &, T1 &, T2 &, T3 &, T4 &, T5 &, T6 &, T7 &, T8 &, T9 &>(
      t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#else //if LOFTY_HOST_STL_LOFTY || !defined(LOFTY_CXX_VARIADIC_TEMPLATES)
   #include <tuple>

   namespace lofty { namespace _std { namespace _pub {

   using ::std::get;
   using ::std::ignore;
   using ::std::make_tuple;
   using ::std::tie;
   using ::std::tuple;
   using ::std::tuple_element;
   using ::std::tuple_size;

   }}}
#endif //if LOFTY_HOST_STL_LOFTY || !defined(LOFTY_CXX_VARIADIC_TEMPLATES) … else

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_TUPLE_HXX_NOPUB

#ifdef _LOFTY_STD_TUPLE_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace _std {

   using _pub::get;
   using _pub::ignore;
   using _pub::make_tuple;
   using _pub::tie;
   using _pub::tuple;
   using _pub::tuple_element;
   using _pub::tuple_size;

   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_STD_TUPLE_HXX

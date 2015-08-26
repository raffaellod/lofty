﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#ifndef _ABACLADE_STD_TYPE_TRAITS_HXX
#define _ABACLADE_STD_TYPE_TRAITS_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Defines an integral constant (C++11 § 20.9.3 “Helper classes”).
template <typename T, T t_t>
struct integral_constant {
   static T const value = t_t;
};

//! True integral constant (C++11 § 20.9.3 “Helper classes”).
typedef integral_constant<bool, true> true_type;
//! False integral constant (C++11 § 20.9.3 “Helper classes”).
typedef integral_constant<bool, false> false_type;

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/*! Removes const and volatile qualifiers from a type (C++11 § 20.9.7.1 “Const-volatile
modifications”). */
template <typename T>
struct remove_cv {
   typedef T type;
};
template <typename T>
struct remove_cv<T const> {
   typedef T type;
};
template <typename T>
struct remove_cv<T volatile> {
   typedef T type;
};
template <typename T>
struct remove_cv<T const volatile> {
   typedef T type;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/* All the constants that depend on compiler support will default to a safe (if pessimistic)
default, in case no such support is available. */

#if 0
// True if a type has a default constructor.
template <typename T>
is_default_constructible
template <typename T>
is_trivially_default_constructible
template <typename T>
is_nothrow_default_constructible

// True if T::T(T const &) is defined.
template <typename T>
struct is_copy_constructible
template <typename T>
struct is_trivially_copy_constructible
template <typename T>
struct is_nothrow_copy_constructible

// True if a type has a move constructor.
template <typename T>
struct is_move_constructible
template <typename T>
struct is_trivially_move_constructible
template <typename T>
struct is_nothrow_move_constructible

// True if a type has a copy assignment operator.
template <typename T>
struct is_copy_assignable
template <typename T>
struct is_trivially_copy_assignable
template <typename T>
struct is_nothrow_copy_assignable

// True if a type has a move assignment operator.
template <typename T>
struct is_move_assignable
template <typename T>
struct is_trivially_move_assignable
template <typename T>
struct is_nothrow_move_assignable

// True if a type has a non-deleted destructor.
template <typename T>
struct is_destructible
template <typename T>
struct is_trivially_destructible
template <typename T>
struct is_nothrow_destructible

// True if T::~T() is declared as virtual.
template <typename T>
struct has_virtual_destructor
#endif

//! True if T::operator=(T const &) is declared as throw().
template <typename T>
struct has_nothrow_assign : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_nothrow_assign(T)
#endif
> {};

//! True if T::T(T const &) is declared as throw().
template <typename T>
struct has_nothrow_copy_constructor : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_nothrow_copy(T)
#endif
> {};

//! True if T::T() is declared as throw().
template <typename T>
struct has_nothrow_default_constructor : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_nothrow_constructor(T)
#endif
> {};

//! True if T::operator=(T const &) is just a memcpy().
template <typename T>
struct has_trivial_assign : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_trivial_assign(T)
#endif
> {};

//! True if T::T(T const &) is just a memcpy().
template <typename T>
struct has_trivial_copy_constructor : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_trivial_copy(T)
#endif
> {};

//! True if T::T() is a no-op.
template <typename T>
struct has_trivial_default_constructor : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_trivial_constructor(T)
#endif
> {};

//! True if T::~T() is a no-op.
template <typename T>
struct has_trivial_destructor : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __has_trivial_destructor(T)
#endif
> {};

//! True if T has no members or base classes of size > 0 (C++11 20.9.4.3 “Type properties”).
template <typename T>
struct is_empty : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __is_empty(T)
#endif
> {};

//! True if T is an l-value reference type (C++11 § 20.9.4.1 “Primary type categories”).
template <typename T>
struct is_lvalue_reference : public false_type {};
template <typename T>
struct is_lvalue_reference<T &> : public true_type {};

//! True if T is an r-value reference type (C++11 § 20.9.4.1 “Primary type categories”).
template <typename T>
struct is_rvalue_reference : public false_type {};
template <typename T>
struct is_rvalue_reference<T &&> : public true_type {};

//! True if T is a reference type (C++11 § 20.9.4.2 “Composite type traits”).
template <typename T>
struct is_reference : public integral_constant<
   bool, is_lvalue_reference<T>::value || is_rvalue_reference<T>::value
> {};

/*! True if T is a scalar type or a trivially copyable class with a trivial default constructor
(C++11 § 20.9.4.3 “Type properties”).
*/
template <typename T>
struct is_trivial : public integral_constant<bool, false
#if ABC_HOST_CXX_GCC || ABC_HOST_CXX_MSC
   || __is_trivial(T)
#endif
> {};

}} //namespace abc::_std

namespace abc { namespace _std { namespace detail {

//! Helper for abc::_std::is_void.
template <typename T>
struct is_void_helper : public false_type {};
template <>
struct is_void_helper<void> : public true_type {};

}}} //namespace abc::_std::detail

namespace abc { namespace _std {

//! True if T is void (C++11 § 20.9.4.1 “Primary type categories”).
template <typename T>
struct is_void : public detail::is_void_helper<typename remove_cv<T>::type> {};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace detail {

//! Helper for add_lvalue_reference.
template <typename T, bool t_bAddLRef>
struct add_lvalue_reference_helper {
   typedef T & type;
};
template <typename T>
struct add_lvalue_reference_helper<T, false> {
   typedef T type;
};

//! Helper for add_rvalue_reference.
template <typename T, bool t_bAddRRef>
struct add_rvalue_reference_helper {
   typedef T & type;
};
template <typename T>
struct add_rvalue_reference_helper<T, false> {
   typedef T type;
};

}}} //namespace abc::_std::detail

namespace abc { namespace _std {

//! Adds an l-value reference to the type (C++11 § 20.9.7.2 “Reference modifications”).
template <typename T>
struct add_lvalue_reference : public detail::add_lvalue_reference_helper<T, !is_void<T>::value> {};

//! Adds an r-value reference to the type (C++11 § 20.9.7.2 “Reference modifications”).
template <typename T>
struct add_rvalue_reference : public detail::add_rvalue_reference_helper<
   T, !is_void<T>::value && !is_reference<T>::value
> {};


/*! Defines a member named type as T, removing reference qualifiers if any are present (C++11 §
20.9.7.2 “Reference modifications”).
*/
template <typename T>
struct remove_reference {
   typedef T type;
};
template <typename T>
struct remove_reference<T &> {
   typedef T type;
};
template <typename T>
struct remove_reference<T &&> {
   typedef T type;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

/*! Defines a member named type as TTrue if t_bTest == true, else if is defined as TFalse (C++11 §
20.9.7.6 “Other transformations”). */
template <bool t_bTest, typename TTrue, typename TFalse>
struct conditional {
   typedef TFalse type;
};
template <typename TTrue, typename TFalse>
struct conditional<true, TTrue, TFalse> {
   typedef TTrue type;
};

/*! Removes extents, references, cv-qualifiers and more, similarly to the way types decay when used
as arguments in a function call (C++11 § 20.9.7.6 “Other transformations”). */
template <typename T>
struct decay {
    typedef typename remove_reference<T>::type U;
    typedef typename conditional<
        is_array<U>::value,
        typename remove_extent<U>::type *,
        typename conditional<
            is_function<U>::value,
            typename add_pointer<U>::type,
            typename remove_cv<U>::type
        >::type
    >::type type;
};

/*! Defines a member named type as T, if and only if t_bTest == true (C++11 § 20.9.7.6 “Other
transformations”). */
template <bool t_bTest, typename T = void>
struct enable_if {
};
template <typename T>
struct enable_if<true, T> {
   typedef T type;
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_TYPE_TRAITS_HXX

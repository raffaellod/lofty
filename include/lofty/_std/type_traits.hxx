/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_TYPE_TRAITS_HXX
#define _LOFTY_STD_TYPE_TRAITS_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declaration.
namespace lofty {

class noncopyable;

} //namespace lofty

namespace lofty { namespace _std {

#ifndef _LOFTY_STD_TYPE_TRAITS_SELECTIVE

//! Defines an integral constant (C++11 § 20.9.3 “Helper classes”).
template <typename T, T t>
struct integral_constant {
   static T const value = t;
};

//! True integral constant (C++11 § 20.9.3 “Helper classes”).
typedef integral_constant<bool, true> true_type;
//! False integral constant (C++11 § 20.9.3 “Helper classes”).
typedef integral_constant<bool, false> false_type;

//! Defines a member named type as T, if and only if test == true (C++11 § 20.9.7.6 “Other transformations”).
template <bool test, typename T = void>
struct enable_if {
};
template <typename T>
struct enable_if<true, T> {
   typedef T type;
};

/*! Defined as _std::true_type if T is a scalar type or a trivially copyable class with a trivial default
constructor, or _std::false_type otherwise (C++11 § 20.9.4.3 “Type properties”). */
template <typename T>
struct is_trivial : public integral_constant<bool, false
#if LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC
   || __is_trivial(T)
#endif
> {};

#endif //ifndef _LOFTY_STD_TYPE_TRAITS_SELECTIVE

#if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || ( \
   LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40800 \
)
   /*! Defined as _std::true_type if T::T(T const &) is defined, or _std::false_type otherwise. The additional
   template argument is used to partially-specialize this trait for lofty::noncopyable subclasses if C++11’s
   function deletion is not supported. */
   template <
      typename T
   #ifndef LOFTY_CXX_FUNC_DELETE
      , typename = void
   #endif
   >
   struct is_copy_constructible {
   private:
      template <typename U>
      static long test(decltype(U(*reinterpret_cast<U const *>(8192))) *);

      template <typename>
      static short test(...);

   public:
      static bool const value = (sizeof(test<T>(0)) == sizeof(long));
   };
   template <>
   struct is_copy_constructible<void> : public false_type {};
   #ifndef LOFTY_CXX_FUNC_DELETE
      // Partially-specialize to always return true for lofty::noncopyable subclasses.
      template <typename T>
      struct is_copy_constructible<T, typename enable_if<is_base_of<noncopyable, T>::value>::type> :
         public false_type {};
   #endif

   #define _LOFTY_STD_TYPE_TRAITS_IS_COPY_CONSTRUCTIBLE
#endif /*if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || (
            LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40800
         )*/

#if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || ( \
   LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 50000 \
)
   //! Defined as _std::true_type if T::T(T const &) is just a memcpy(), or _std::false_type otherwise.
   template <typename T>
   struct is_trivially_copy_constructible : public integral_constant<bool,
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC
      __has_trivial_copy(T)
   #endif
   > {};

   #define _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_COPY_CONSTRUCTIBLE
#endif /*if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || (
            LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40900
         )*/

#if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || ( \
   LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40800 \
)
   //! Defined as _std::true_type if T::~T() is a no-op, or _std::false_type otherwise.
   template <typename T>
   struct is_trivially_destructible : public integral_constant<bool,
   #if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC
      __has_trivial_destructor(T)
   #endif
   > {};

   #define _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_DESTRUCTIBLE
#endif /*if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || (
            LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40800
         )*/

#if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || ( \
   LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 50000 \
)
   //! Defined as _std::true_type if T::T(T &&) is just a memcpy(), or _std::false_type otherwise.
   template <typename T>
   struct is_trivially_move_constructible : public is_trivial<T> {};
   // TODO: the above is less accurate than it could be, but it’s safe.

   #define _LOFTY_STD_TYPE_TRAITS_IS_TRIVIALLY_MOVE_CONSTRUCTIBLE
#endif /*if !defined(_LOFTY_STD_TYPE_TRAITS_SELECTIVE) || LOFTY_HOST_STL_MSVCRT || (
            LOFTY_HOST_STL_LIBSTDCXX && LOFTY_HOST_STL_LIBSTDCXX < 40900
         )*/

#ifndef _LOFTY_STD_TYPE_TRAITS_SELECTIVE

/*! Defined as _std::true_type if T has no members or base classes of size > 0, or _std::false_type otherwise
(C++11 20.9.4.3 “Type properties”). */
template <typename T>
struct is_empty : public integral_constant<bool,
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC || LOFTY_HOST_CXX_MSC
   __is_empty(T)
#endif
> {};

/*! Defined as _std::true_type if T is an l-value reference type, or _std::false_type otherwise (C++11 §
20.9.4.1 “Primary type categories”). */
template <typename T>
struct is_lvalue_reference : public false_type {};
template <typename T>
struct is_lvalue_reference<T &> : public true_type {};

/*! Defined as _std::true_type if T is an r-value reference type, or _std::false_type otherwise (C++11 §
20.9.4.1 “Primary type categories”). */
template <typename T>
struct is_rvalue_reference : public false_type {};
template <typename T>
struct is_rvalue_reference<T &&> : public true_type {};

/*! Defined as _std::true_type if T is a reference type, or _std::false_type otherwise (C++11 § 20.9.4.2
“Composite type traits”). */
template <typename T>
struct is_reference : public integral_constant<
   bool, is_lvalue_reference<T>::value || is_rvalue_reference<T>::value
> {};

//! Removes const and volatile qualifiers from a type (C++11 § 20.9.7.1 “Const-volatile modifications”).
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

}} //namespace lofty::_std

namespace lofty { namespace _std { namespace _pvt {

//! Helper for lofty::_std::is_void.
template <typename T>
struct is_void_helper : public false_type {};
template <>
struct is_void_helper<void> : public true_type {};

}}} //namespace lofty::_std::_pvt

namespace lofty { namespace _std {

/*! Defined as _std::true_type if T is void, or _std::false_type otherwise (C++11 § 20.9.4.1 “Primary type
categories”). */
template <typename T>
struct is_void : public _pvt::is_void_helper<typename remove_cv<T>::type> {};

}} //namespace lofty::_std

namespace lofty { namespace _std { namespace _pvt {

//! Helper for add_lvalue_reference.
template <typename T, bool add_lref>
struct add_lvalue_reference_helper {
   typedef T & type;
};
template <typename T>
struct add_lvalue_reference_helper<T, false> {
   typedef T type;
};

//! Helper for add_rvalue_reference.
template <typename T, bool add_rref>
struct add_rvalue_reference_helper {
   typedef T & type;
};
template <typename T>
struct add_rvalue_reference_helper<T, false> {
   typedef T type;
};

}}} //namespace lofty::_std::_pvt

namespace lofty { namespace _std {

//! Adds an l-value reference to the type (C++11 § 20.9.7.2 “Reference modifications”).
template <typename T>
struct add_lvalue_reference : public _pvt::add_lvalue_reference_helper<T, !is_void<T>::value> {};

//! Adds an r-value reference to the type (C++11 § 20.9.7.2 “Reference modifications”).
template <typename T>
struct add_rvalue_reference : public _pvt::add_rvalue_reference_helper<
   T, !is_void<T>::value && !is_reference<T>::value
> {};

/*! Defines a member named type as T, removing reference qualifiers if any are present (C++11 § 20.9.7.2
“Reference modifications”). */
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

/*! Defines a member named type as TTrue if test == true, else if is defined as TFalse (C++11 § 20.9.7.6
“Other transformations”). */
template <bool test, typename TTrue, typename TFalse>
struct conditional {
   typedef TFalse type;
};
template <typename TTrue, typename TFalse>
struct conditional<true, TTrue, TFalse> {
   typedef TTrue type;
};

/*! Removes extents, references, cv-qualifiers and more, similarly to the way types decay when used as
arguments in a function call (C++11 § 20.9.7.6 “Other transformations”). */
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

#endif //ifndef _LOFTY_STD_TYPE_TRAITS_SELECTIVE

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_TYPE_TRAITS_HXX

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::collections::type_void_adapter

namespace abc {
namespace collections {

/*! DOC:3395 Move constructors and exceptions

In this section, “move constructor” will strictly refer to class::class(class &&).

All classes must provide move constructors and assignment operators if the copy constructor would
result in execution of exception-prone code (e.g. resource allocation).

Because move constructors are employed widely in container classes that need to provide strong
exception guarantee (fully transacted operation) even in case of moves, move constructors must not
throw exceptions. This requirement is relaxed for moves that involve two different classes, since
these will not be used by container classes.
*/

//! Encapsulates raw constructors, destructors and assignment operators for a type.
struct type_void_adapter {
public:
   /*! Prototype of a function that copies items from one array to another.

   @param pDstBegin
      Pointer to the start of the destination array. The items are supposed to be uninitialized.
   @param pSrcBegin
      Pointer to the first item to copy.
   @param pSrcEnd
      Pointer to beyond the last item to copy.
   */
   typedef void (* copy_fn)(void * pDstBegin, void const * pSrcBegin, void const * pSrcEnd);

   /*! Prototype of a function that destructs a range of items in an array.

   @param pBegin
      Pointer to the first item to destruct.
   @param pEnd
      Pointer to beyond the last item to destruct.
   */
   typedef void (* destr_fn)(void const * pBegin, void const * pEnd);

   /*! Prototype of a function that compares two values for equality.

   @param p1
      Pointer to the first item.
   @param p2
      Pointer to the second item.
   @return
      true if the items are equal, or false otherwise.
   */
   typedef bool (* equal_fn)(void const * p1, void const * p2);

   /*! Prototype of a function that moves items from one array to another.

   @param pDstBegin
      Pointer to the start of the destination array. The items are supposed to be uninitialized.
   @param pSrcBegin
      Pointer to the first item to move.
   @param pSrcEnd
      Pointer to beyond the last item to move.
   */
   typedef void (* move_fn)(void * pDstBegin, void * pSrcBegin, void * pSrcEnd);

public:
   //! Size of a variable of this type, in bytes.
   std::size_t cb;
   //! Function to copy items from one array to another.
   copy_fn copy_constr;
   //! Function to destruct items in an array.
   destr_fn destruct;
   //! Function to compare two items for equality.
   equal_fn equal;
   //! Function to move items from one array to another.
   move_fn move_constr;

public:
   //! Constructor.
   type_void_adapter() :
      cb(0),
      copy_constr(nullptr),
      destruct(nullptr),
      equal(nullptr),
      move_constr(nullptr) {
   }

   //! Initializes this->copy_constr.
   template <typename T>
   void set_copy_fn() {
      copy_constr = reinterpret_cast<copy_fn>(_typed_copy_constr<typename std::remove_cv<T>::type>);
#if ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700
      // Force instantiating the template, even if (obviously) never executed.
      if (!copy_constr) {
         _typed_copy_constr<typename std::remove_cv<T>::type>(nullptr, nullptr, nullptr);
      }
#endif
   }

   //! Initializes this->destruct.
   template <typename T>
   void set_destr_fn() {
      destruct = reinterpret_cast<destr_fn>(_typed_destruct<typename std::remove_cv<T>::type>);
#if ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700
      // Force instantiating the template, even if (obviously) never executed.
      if (!destruct) {
         _typed_destruct<typename std::remove_cv<T>::type>(nullptr, nullptr);
      }
#endif
   }

   //! Initializes this->equal.
   template <typename T>
   void set_equal_fn() {
      equal = reinterpret_cast<equal_fn>(_typed_equal<typename std::remove_cv<T>::type>);
#if ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700
      // Force instantiating the template, even if (obviously) never executed.
      if (!equal) {
         _typed_equal<typename std::remove_cv<T>::type>(nullptr, nullptr);
      }
#endif
   }

   //! Initializes this->move_constr.
   template <typename T>
   void set_move_fn() {
      move_constr = reinterpret_cast<move_fn>(_typed_move_constr<typename std::remove_cv<T>::type>);
#if ABC_HOST_CXX_GCC && ABC_HOST_CXX_GCC < 40700
      // Force instantiating the template, even if (obviously) never executed.
      if (!move_constr) {
         _typed_move_constr<typename std::remove_cv<T>::type>(nullptr, nullptr, nullptr);
      }
#endif
   }

   //! Initializes this->cb.
   template <typename T>
   void set_size() {
      cb = sizeof(T);
   }

private:
   /*! Copies a range of items from one array to another, overwriting any existing contents in the
   destination.

   @param ptDstBegin
      Pointer to the start of the destination array. The items are supposed to be uninitialized.
   @param ptSrcBegin
      Pointer to the first item to copy.
   @param ptSrcEnd
      Pointer to beyond the last item to copy.
   */
#if ABC_HOST_CXX_MSC
   /* MSC applies SFINAE too late, and when asked to get the address of the *one and only* valid
   version of _typed_copy_constr() (see non-MSC code in the #else branch), it will raise an error
   saying it doesn’t know which one to choose. */
   template <typename T>
   static void _typed_copy_constr(T * ptDstBegin, T const * ptSrcBegin, T const * ptSrcEnd) {
      if (std::has_trivial_copy_constructor<T>::value) {
         // No constructor, fastest copy possible.
         memory::copy(ptDstBegin, ptSrcBegin, static_cast<std::size_t>(ptSrcEnd - ptSrcBegin));
      } else {
         // Assume that since it’s not trivial, it can throw exceptions, so perform a transactional
         // copy.
         T * ptDst = ptDstBegin;
         try {
            for (T const * ptSrc = ptSrcBegin; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
               ::new(ptDst) T(*ptSrc);
            }
         } catch (...) {
            // Undo (destruct) all the copies instantiated.
            while (--ptDst >= ptDstBegin) {
               ptDst->~T();
            }
            throw;
         }
      }
   }
#else //if ABC_HOST_CXX_MSC
   // Only enabled if the copy constructor is trivial.
   template <typename T>
   static void _typed_copy_constr(
      typename std::enable_if<
#ifdef ABC_CXX_STL_CXX11_TYPE_TRAITS
         std::is_trivially_copy_constructible<T>::value,
#else
         std::has_trivial_copy_constructor<T>::value,
#endif
      T *>::type ptDstBegin, T const * ptSrcBegin, T const * ptSrcEnd
   ) {
      // No constructor, fastest copy possible.
      memory::copy(ptDstBegin, ptSrcBegin, static_cast<std::size_t>(ptSrcEnd - ptSrcBegin));
   }
   // Only enabled if the copy constructor is not trivial.
   template <typename T>
   static void _typed_copy_constr(
      typename std::enable_if<
#ifdef ABC_CXX_STL_CXX11_TYPE_TRAITS
         !std::is_trivially_copy_constructible<T>::value,
#else
         !std::has_trivial_copy_constructor<T>::value,
#endif
      T * >::type ptDstBegin, T const * ptSrcBegin, T const * ptSrcEnd
   ) {
      // Assume that since it’s not trivial, it can throw exceptions, so perform a transactional
      // copy.
      T * ptDst = ptDstBegin;
      try {
         for (T const * ptSrc = ptSrcBegin; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
            ::new(ptDst) T(*ptSrc);
         }
      } catch (...) {
         // Undo (destruct) all the copies instantiated.
         while (--ptDst >= ptDstBegin) {
            ptDst->~T();
         }
         throw;
      }
   }
#endif //if ABC_HOST_CXX_MSC … else

   /*! Destructs a range of items in an array.

   @param ptBegin
      Pointer to the first item to destruct.
   @param ptEnd
      Pointer to beyond the last item to destruct.
   */
   template <typename T>
   static void _typed_destruct(T const * ptBegin, T const * ptEnd) {
#if defined(ABC_CXX_STL_CXX11_TYPE_TRAITS) || defined(ABC_CXX_STL_CXX11_GLIBCXX_PARTIAL_TYPE_TRAITS)
      if (!std::is_trivially_destructible<T>::value) {
#else
      if (!std::has_trivial_destructor<T>::value) {
#endif
         // The destructor is not a no-op.
         for (T const * pt = ptBegin; pt < ptEnd; ++pt) {
            pt->~T();
         }
      }
   }

   /*! Compares two values for equality.

   @param pt1
      Pointer to the first item.
   @param pt2
      Pointer to the second item.
   @return
      true if the items are equal, or false otherwise.
   */
   template <typename T>
   static bool _typed_equal(T const * pt1, T const * pt2) {
      return *pt1 == *pt2;
   }

   /*! Moves a range of items from one array to another, overwriting any existing contents in the
   destination.

   @param ptDstBegin
      Pointer to the start of the destination array. The items are supposed to be uninitialized.
   @param ptSrcBegin
      Pointer to the first item to copy.
   @param ptSrcEnd
      Pointer to beyond the last item to copy.
   */
   template <typename T>
   static void _typed_move_constr(T * ptDstBegin, T * ptSrcBegin, T * ptSrcEnd) {
      for (T * ptSrc = ptSrcBegin, * ptDst = ptDstBegin; ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
         ::new(ptDst) T(std::move(*ptSrc));
      }
   }
};

} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

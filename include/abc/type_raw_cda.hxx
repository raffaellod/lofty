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

#ifndef ABC_TYPE_RAW_CDA_HXX
#define ABC_TYPE_RAW_CDA_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <type_traits>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::void_cda


namespace abc {

/** Encapsulates raw constructors, destructors and assignment operators for a type. To be
instantiated via type_raw_cda.
*/
struct void_cda {

   /** Prototype of a function that copies items from one array to another.

   pDst
      Pointer to the destination array. The items are supposed to be uninitialized.
   pSrc
      Pointer to the source array.
   ci
      Count of items to copy.
   */
   typedef void (* copy_fn)(void * pDst, void const * pSrc, size_t ci);


   /** Prototype of a function that compares two values for equality.

   p1
      Pointer to the first item.
   p2
      Pointer to the second item.
   return
      true if the items are equal, or false otherwise.
   */
   typedef bool (* equal_fn)(void const * p1, void const * p2);


   /** Prototype of a function that moves items from one array to another.

   pDst
      Pointer to the destination array. The items are supposed to be uninitialized.
   pSrc
      Pointer to the source array.
   ci
      Count of items to move.
   */
   typedef void (* move_fn)(void * pDst, void * pSrc, size_t ci);


   /** Prototype of a function that destructs items in an array.

   p
      Pointer to the array.
   ci
      Count of items to destruct.
   */
   typedef void (* destr_fn)(void * p, size_t ci);


   /** Size of a variable of this type, in bytes. */
   size_t cb;
   /** Function to copy items from one array to another. */
   copy_fn copy_constr;
   /** Function to move items from one array to another. */
   move_fn move_constr;
   /** Function to destruct items in an array. */
   destr_fn destruct;
   /** Function to compare two items for equality. */
   equal_fn equal;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::typed_raw_cda


namespace abc {

/** DOC:3395 Move constructors and exceptions

In this section, “move constructor” will strictly refer to class::class(class &&).

All classes must provide move constructors and assignment operators if the copy constructor would
result in execution of exception-prone code (e.g. resource allocation).

Because move constructors are employed widely in container classes that need to provide strong
exception guarantee (fully transacted operation) even in case of moves, move constructors must not
throw exceptions. This requirement is relaxed for moves that involve two different classes, since
these will not be used by container classes.
*/

/** Defines a generic data type.
*/
template <typename T>
struct typed_raw_cda {

   /** Copies a range of items from one array to another, overwriting any existing contents in the
   destination.

   ptDst
      Pointer to the destination array. The items are supposed to be uninitialized.
   ptSrc
      Pointer to the source array.
   ci
      Count of items to copy.
   */
   static void copy_constr(T * ptDst, T const * ptSrc, size_t ci) {
      if (std::has_trivial_copy_constructor<T>::value) {
         // No constructor, fastest copy possible.
         memory::copy(ptDst, ptSrc, ci);
      } else if (
#if defined(_GCC_VER) && _GCC_VER >= 40700
         std::is_nothrow_copy_constructible<T>::value
#else
         std::has_nothrow_copy_constructor<T>::value
#endif
      ) {
         // Not trivial, but it won’t throw either.
         for (T const * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
            ::new(ptDst) T(*ptSrc);
         }
      } else {
         // Exceptions can occur, so implement an all-or-nothing copy.
         T const * ptDstBegin(ptDst);
         try {
            for (T const * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
               ::new(ptDst) T(*ptSrc);
            }
         } catch (...) {
            // Undo (destruct) all the copies instantiated.
            while (--ptSrc >= ptDstBegin) {
               ptDst->~T();
            }
            throw;
         }
      }
   }


   /** Destructs a range of items in an array.

   pt
      Pointer to the array.
   ci
      Count of items to destruct.
   */
   static void destruct(T * pt, size_t ci) {
      if (!std::has_trivial_destructor<T>::value) {
         // The destructor is not a no-op.
         for (T * ptEnd(pt + ci); pt < ptEnd; ++pt) {
            pt->~T();
         }
      }
   }


   /** Compares two values for equality.

   pt1
      Pointer to the first item.
   pt2
      Pointer to the second item.
   return
      true if the items are equal, or false otherwise.
   */
   static bool equal(void const * pt1, void const * pt2) {
      return *static_cast<T const *>(pt1) == *static_cast<T const *>(pt2);
   }


   /** Moves a range of items from one array to another, overwriting any existing contents in the
   destination.

   ptDst
      Pointer to the destination array. The items are supposed to be uninitialized.
   ptSrc
      Pointer to the source array.
   ci
      Count of items to move.
   */
   static void move_constr(T * ptDst, T * ptSrc, size_t ci) {
      for (T * ptSrcEnd(ptSrc + ci); ptSrc < ptSrcEnd; ++ptSrc, ++ptDst) {
         ::new(ptDst) T(std::move(*ptSrc));
      }
   }
};

// Remove const, but not volatile. Or maybe remove both?
template <typename T>
struct typed_raw_cda<T const> :
   public typed_raw_cda<T> {
};
template <typename T>
struct typed_raw_cda<T const volatile> :
   public typed_raw_cda<T volatile> {
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::type_raw_cda


namespace abc {

/** Returns a void_cda populated with the static methods from a typed_raw_cda.

TODO: comment signature.
*/
template <class T>
/*constexpr*/ void_cda const & type_raw_cda() {
   static void_cda const sc_vrcda = {
      sizeof(T),
      reinterpret_cast<void_cda:: copy_fn>(typed_raw_cda<T>::copy_constr),
      reinterpret_cast<void_cda:: move_fn>(typed_raw_cda<T>::move_constr),
      reinterpret_cast<void_cda::destr_fn>(typed_raw_cda<T>::destruct),
      reinterpret_cast<void_cda::equal_fn>(typed_raw_cda<T>::equal)
   };
   return sc_vrcda;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TYPE_RAW_CDA_HXX


/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_STD_ATOMIC_HXX
#define _ABACLADE_STD_ATOMIC_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std {

//! Memory synchronization orders (C++11 § 29.3 “Order and consistency”).
typedef enum {
   //! No operation orders memory.
   memory_order_relaxed,
   //! Causes a load to perform a consume operation.
   memory_order_consume,
   //! Causes a load to perform an acquire operation.
   memory_order_acquire,
   //! Causes a store to perform a release operation.
   memory_order_release,
   //! Causes a load to perform an acquire operation, and a store to perform a release operation.
   memory_order_acq_rel,
   //! Causes a load to perform an acquire operation, and a store to perform a release operation.
   memory_order_seq_cst
}  memory_order;

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace _std { namespace detail {

//! Implementation of key abc::_std::atomic members, specialized by size in bytes of the argument.
template <unsigned t_cb>
class atomic_impl_base;

#if ABC_HOST_CXX_MSC >= 1600 && ABC_HOST_CXX_MSC < 1700
   /* Base for specializations of atomic_impl_base for MSC by leveraging the MS-specific
   interpretation of volatile. */
   template <typename T>
   class atomic_impl_base_impl {
   protected:
      //! Type of the underlying scalar value.
      typedef T int_t;

   protected:
      /*! Constructor.

      @param i
         Initial value for the variable.
      */
      /*constexpr*/ atomic_impl_base_impl(int_t i) :
         m_i(i) {
      }

      //! See atomic::compare_exchange_strong().
      bool compare_exchange_strong(int_t & iExpected, int_t iDesired, memory_order mo) {
         ABC_UNUSED_ARG(mo);
         if (m_i != iExpected) {
            iExpected = m_i;
            return false;
         }
         m_i = iDesired;
         return true;
      }

      //! See atomic::load().
      int_t load(memory_order mo) const {
         ABC_UNUSED_ARG(mo);
         return m_i;
      }

      //! See atomic::store().
      void store(int_t i, memory_order mo) {
         ABC_UNUSED_ARG(mo);
         m_i = i;
      }

   private:
      //! Underlying scalar value.
      T volatile m_i;
   };

   // Specialization for 1-byte integers.
   template <>
   class atomic_impl_base<1> : public atomic_impl_base_impl<std::uint8_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i) :
         atomic_impl_base_impl<std::uint8_t>(i) {
      }
   };

   // Specialization for 2-byte integers.
   template <>
   class atomic_impl_base<2> : public atomic_impl_base_impl<std::uint16_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i) :
         atomic_impl_base_impl<std::uint16_t>(i) {
      }
   };

   // Specialization for 4-byte integers.
   template <>
   class atomic_impl_base<4> : public atomic_impl_base_impl<std::uint32_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i) :
         atomic_impl_base_impl<std::uint32_t>(i) {
      }
   };

   // Specialization for 8-byte integers.
   template <>
   class atomic_impl_base<8> : public atomic_impl_base_impl<std::uint64_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i) :
         atomic_impl_base_impl<std::uint64_t>(i) {
      }
   };
#else
   #error "TODO: HOST_CXX"
#endif

//! Implementation of abc::_std::atomic.
template <
   typename T,
   bool t_bInt = is_arithmetic<T>::value || is_enum<T>::value,
   bool t_bPtr = is_pointer<T>::value
>
class atomic_impl;

// Specialization for integer types.
template <typename T>
class atomic_impl<T, true, false> : private atomic_impl_base<sizeof(T)> {
private:
   typedef atomic_impl_base<sizeof(T)> impl_base;

public:
   /*! Default constructor.

   @param t
      Initial value for the variable.
   */
   /*constexpr*/ atomic_impl(T t = T()) :
      impl_base(static_cast<impl_base::int_t>(t)) {
   }

   /*! Performs a compare-and-swap operation (C++11 § 29.6.1 “General operations on atomic types”).

   @param tExpected
      Expected value of *this; if *this has a different value, that value will be stored in
      tExpected and this method will return false.
   @param tDesired
      New value to be stored in *this.
   @param mo
      Memory ordering requirement.
   @return
      true if the value was swapped, or false if *this was not equal to tExpected.
   */
   bool compare_exchange_strong(T & tExpected, T tDesired, memory_order mo = memory_order_seq_cst) {
      return impl_base::compare_exchange_strong(
         *reinterpret_cast<int_t *>(&tExpected), static_cast<int_t>(tDesired), mo
      );
   }

   /*! Performs an atomic addition operation (C++11 § 29.6.3 “Arithmetic operations on atomic
   types”).

   @param tAddend
      Value to add to *this.
   @param mo
      Memory ordering requirement.
   @return
      Value of *this before the addition.
   */
   T fetch_add(T tAddend, memory_order mo = memory_order_seq_cst) {
      for (;;) {
         T tPrevValue = load(mo);
         if (compare_exchange_strong(tPrevValue, tPrevValue + tAddend, mo)) {
            return tPrevValue;
         }
      }
   }

   /*! Performs an atomic subtraction operation (C++11 § 29.6.3 “Arithmetic operations on atomic
   types”).

   @param tSubtrahend
      Value to subtract from *this.
   @param mo
      Memory ordering requirement.
   @return
      Value of *this before the subtraction.
   */
   T fetch_sub(T tSubtrahend, memory_order mo = memory_order_seq_cst) {
      for (;;) {
         T tPrevValue = load(mo);
         if (compare_exchange_strong(tPrevValue, tPrevValue - tSubtrahend, mo)) {
            return tPrevValue;
         }
      }
   }

   /*! Reads the current value of the object (C++11 § 29.6.1 “General operations on atomic types”).

   @param mo
      Memory ordering requirement.
   @return
      Current value of *this.
   */
   T load(memory_order mo = memory_order_seq_cst) const {
#if ABC_HOST_CXX_MSC
      #pragma warning(push)
      // “'unsigned char' : forcing value to bool 'true' or 'false' (performance warning)”.
      #pragma warning(suppress: 4800)
#endif
      return static_cast<T>(impl_base::load(mo));
#if ABC_HOST_CXX_MSC
      #pragma warning(pop)
#endif
   }

   /*! Stores a value in the object (C++11 § 29.6.1 “General operations on atomic types”).

   @param t
      New value of *this.
   @param mo
      Memory ordering requirement.
   */
   void store(T t, memory_order mo = memory_order_seq_cst) {
      impl_base::store(static_cast<impl_base::int_t>(t), mo);
   }
};

// Specialization for pointer types.
template <typename T>
class atomic_impl<T, false, true> : private atomic_impl_base<sizeof(T)> {
private:
   typedef atomic_impl_base<sizeof(T)> impl_base;

public:
   /*! Default constructor.

   @param t
      Initial value for the variable.
   */
   /*constexpr*/ atomic_impl(T t = T()) :
      impl_base(static_cast<impl_base::int_t>(t)) {
   }

   /*! Performs a compare-and-swap operation (C++11 § 29.6.1 “General operations on atomic types”).

   @param tExpected
      Expected value of *this; if *this has a different value, that value will be stored in
      tExpected and this method will return false.
   @param tDesired
      New value to be stored in *this.
   @param mo
      Memory ordering requirement.
   @return
      true if the value was swapped, or false if *this was not equal to tExpected.
   */
   bool compare_exchange_strong(T & tExpected, T tDesired, memory_order mo = memory_order_seq_cst) {
      return impl_base::compare_exchange_strong(
         *reinterpret_cast<impl_base::int_t *>(&tExpected),
         reinterpret_cast<impl_base::int_t>(tDesired), mo
      );
   }

   /*! Reads the current value of the object (C++11 § 29.6.1 “General operations on atomic types”).

   @param mo
      Memory ordering requirement.
   @return
      Current value of *this.
   */
   T load(memory_order mo = memory_order_seq_cst) const {
      return reinterpret_cast<T>(impl_base::load(mo));
   }

   /*! Stores a value in the object (C++11 § 29.6.1 “General operations on atomic types”).

   @param t
      New value of *this.
   @param mo
      Memory ordering requirement.
   */
   void store(T t, memory_order mo = memory_order_seq_cst) {
      impl_base::store(reinterpret_cast<impl_base::int_t>(t), mo);
   }
};

}}} //namespace abc::_std::detail

namespace abc { namespace _std {

/*! Type with enforceable atomic access and defined memory access ordering (C++11 § 29.5.1 “Atomic
types”). */
template <typename T>
class atomic : public detail::atomic_impl<T> {
public:
   /*! Default constructor.

   @param t
      Initial value for the variable.
   */
   /*constexpr*/ atomic(T t = T()) :
      detail::atomic_impl<T>(t) {
   }
};

}} //namespace abc::_std

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_STD_ATOMIC_HXX

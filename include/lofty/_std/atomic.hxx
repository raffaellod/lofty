/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_STD_ATOMIC_HXX
#define _LOFTY_STD_ATOMIC_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std {

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

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _std { namespace _pvt {

//! Implementation of key lofty::_std::atomic members, specialized by size in bytes of the argument.
template <unsigned byte_size>
class atomic_impl_base;

#if LOFTY_HOST_CXX_MSC >= 1600 && LOFTY_HOST_CXX_MSC < 1700
   /* Base for specializations of atomic_impl_base for MSC by leveraging the MS-specific interpretation of
   volatile. */
   template <typename T>
   class atomic_impl_base_impl {
   protected:
      //! Type of the underlying scalar value.
      typedef T int_t;

   protected:
      /*! Constructor.

      @param i_
         Initial value for the variable.
      */
      /*constexpr*/ atomic_impl_base_impl(int_t i_) :
         i(i_) {
      }

      //! See atomic::compare_exchange_strong().
      bool compare_exchange_strong(int_t & expected, int_t desired, memory_order mo) {
         LOFTY_UNUSED_ARG(mo);
         if (i != expected) {
            expected = i;
            return false;
         }
         i = desired;
         return true;
      }

      //! See atomic::load().
      int_t load(memory_order mo) const {
         LOFTY_UNUSED_ARG(mo);
         return i;
      }

      //! See atomic::store().
      void store(int_t i_, memory_order mo) {
         LOFTY_UNUSED_ARG(mo);
         i = i_;
      }

   private:
      //! Underlying scalar value.
      T volatile i;
   };

   // Specialization for 1-byte integers.
   template <>
   class atomic_impl_base<1> : public atomic_impl_base_impl<std::uint8_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i_) :
         atomic_impl_base_impl<std::uint8_t>(i_) {
      }
   };

   // Specialization for 2-byte integers.
   template <>
   class atomic_impl_base<2> : public atomic_impl_base_impl<std::uint16_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i_) :
         atomic_impl_base_impl<std::uint16_t>(i_) {
      }
   };

   // Specialization for 4-byte integers.
   template <>
   class atomic_impl_base<4> : public atomic_impl_base_impl<std::uint32_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i_) :
         atomic_impl_base_impl<std::uint32_t>(i_) {
      }
   };

   // Specialization for 8-byte integers.
   template <>
   class atomic_impl_base<8> : public atomic_impl_base_impl<std::uint64_t> {
   protected:
      //! See atomic_impl_base_impl::atomic_impl_base_impl().
      atomic_impl_base(int_t i_) :
         atomic_impl_base_impl<std::uint64_t>(i_) {
      }
   };
#else
   #error "TODO: HOST_CXX"
#endif

//! Implementation of lofty::_std::atomic.
template <
   typename T,
   bool is_int = is_arithmetic<T>::value || is_enum<T>::value,
   bool is_ptr = is_pointer<T>::value
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

   @param expected
      Expected value of *this; if *this has a different value, that value will be stored in expected and this
      method will return false.
   @param desired
      New value to be stored in *this.
   @param mo
      Memory ordering requirement.
   @return
      true if the value was swapped, or false if *this was not equal to expected.
   */
   bool compare_exchange_strong(T & expected, T desired, memory_order mo = memory_order_seq_cst) {
      return impl_base::compare_exchange_strong(
         *reinterpret_cast<int_t *>(&expected), static_cast<int_t>(desired), mo
      );
   }

   /*! Performs an atomic addition operation (C++11 § 29.6.3 “Arithmetic operations on atomic types”).

   @param addend
      Value to add to *this.
   @param mo
      Memory ordering requirement.
   @return
      Value of *this before the addition.
   */
   T fetch_add(T addend, memory_order mo = memory_order_seq_cst) {
      for (;;) {
         T prev_value = load(mo);
         if (compare_exchange_strong(prev_value, prev_value + addend, mo)) {
            return prev_value;
         }
      }
   }

   /*! Performs an atomic subtraction operation (C++11 § 29.6.3 “Arithmetic operations on atomic types”).

   @param subtrahend
      Value to subtract from *this.
   @param mo
      Memory ordering requirement.
   @return
      Value of *this before the subtraction.
   */
   T fetch_sub(T subtrahend, memory_order mo = memory_order_seq_cst) {
      for (;;) {
         T prev_value = load(mo);
         if (compare_exchange_strong(prev_value, prev_value - subtrahend, mo)) {
            return prev_value;
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
#if LOFTY_HOST_CXX_MSC
      // If T is bool, we get:
      #pragma warning(push)
      // “'unsigned char' : forcing value to bool 'true' or 'false' (performance warning)”.
      #pragma warning(suppress: 4800)
#endif
      return static_cast<T>(impl_base::load(mo));
#if LOFTY_HOST_CXX_MSC
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

   @param expected
      Expected value of *this; if *this has a different value, that value will be stored in expected and this
      method will return false.
   @param desired
      New value to be stored in *this.
   @param mo
      Memory ordering requirement.
   @return
      true if the value was swapped, or false if *this was not equal to expected.
   */
   bool compare_exchange_strong(T & expected, T desired, memory_order mo = memory_order_seq_cst) {
      return impl_base::compare_exchange_strong(
         *reinterpret_cast<impl_base::int_t *>(&expected), reinterpret_cast<impl_base::int_t>(desired), mo
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

}}} //namespace lofty::_std::_pvt

namespace lofty { namespace _std {

//! Type with enforceable atomic access and defined memory access ordering (C++11 § 29.5.1 “Atomic types”).
template <typename T>
class atomic : public _pvt::atomic_impl<T> {
public:
   /*! Default constructor.

   @param t
      Initial value for the variable.
   */
   /*constexpr*/ atomic(T t = T()) :
      _pvt::atomic_impl<T>(t) {
   }
};

}} //namespace lofty::_std

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_STD_ATOMIC_HXX

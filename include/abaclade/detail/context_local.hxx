/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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
// abc::detail::context_local_value

namespace abc {
namespace detail {

//! Implementation of abc::thread_local_value and abc::coroutine_local_value.
template <typename T, typename TImpl>
class context_local_value :
   private TImpl,
   public support_explicit_operator_bool<context_local_value<T, TImpl>> {
public:
   //! Constructor.
   context_local_value() :
      TImpl(sizeof(T)) {
   }

   /*! Assignment operator.

   @param t
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(T const & t) {
      get() = t;
      return *this;
   }
   context_local_value & operator=(T && t) {
      get() = std::move(t);
      return *this;
   }

   /*! Implicit cast to T &.

   @return
      Reference to the object’s value.
   */
   operator T &() {
      return get();
   }
   operator T const &() const {
      return get();
   }

   /*! Returns true if the object’s value evaluates to true.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return get() ? true : false;
   }

   /*! Explicit cast to T &.

   @return
      Reference to the object’s value.
   */
   T & get() {
      return *get_ptr();
   }
   T const & get() const {
      return *get_ptr();
   }

private:
   //! See TImpl::construct().
   virtual void construct(void * p) const override {
      new(p) T();
   }

   //! See TImpl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<T *>(p)->~T();
   }

   //! See TImpl::get_ptr().
   T * get_ptr() const {
      return TImpl::template get_ptr<T>();
   }
};

// Specialization for bool, which does not need operator bool().
template <typename TImpl>
class context_local_value<bool, TImpl> : private TImpl {
public:
   //! Constructor.
   context_local_value() :
      TImpl(sizeof(bool)) {
   }

   /*! Assignment operator.

   @param b
      Source value.
   @return
      *this.
   */
   context_local_value & operator=(bool b) {
      get() = b;
      return *this;
   }

   /*! Implicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   operator bool &() {
      return get();
   }
   operator bool const &() const {
      return get();
   }

   /*! Explicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   bool & get() {
      return *get_ptr();
   }
   bool const & get() const {
      return *get_ptr();
   }

private:
   //! See TImpl::construct().
   virtual void construct(void * p) const override {
      new(p) bool();
   }

   //! See TImpl::destruct().
   virtual void destruct(void * p) const override {
      ABC_UNUSED_ARG(p);
   }

   //! See TImpl::get_ptr().
   bool * get_ptr() const {
      return TImpl::template get_ptr<bool>();
   }
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::context_local_ptr

namespace abc {
namespace detail {

//! Implementation of abc::thread_local_ptr and abc::coroutine_local_ptr.
template <typename T, typename TImpl>
class context_local_ptr :
   private TImpl,
   public support_explicit_operator_bool<context_local_ptr<T, TImpl>> {
private:
   //! Contains a T and a bool to track whether the T has been constructed.
   struct value_t {
      T t;
      bool bConstructed;
   };

public:
   //! Constructor.
   context_local_ptr() :
      TImpl(sizeof(value_t)) {
   }

   /*! Dereference operator.

   @return
      Reference to the owned object.
   */
   T & operator*() const {
      return *get();
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the owned object.
   */
   T * operator->() const {
      return get();
   }

   /*! Boolean evaluation operator.

   @return
      true if get() != nullptr, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return get_ptr()->bConstructed;
   }

   /*! Returns the address of the thread-local value this object points to.

   @return
      Internal pointer.
   */
   T * get() const {
      value_t * pValue = get_ptr();
      return pValue->bConstructed ? &pValue->t : nullptr;
   }

   /*! Deletes the object currently pointed to, if any, resetting the pointer to nullptr.

   @param pt
      Pointer to a new object to take ownership of.
   */
   void reset() {
      value_t * pValue = get_ptr();
      if (pValue->bConstructed) {
         pValue->t.~T();
         pValue->bConstructed = false;
      }
   }

   /*! Destructs the object currently pointed to, if any, and constructs a new object.

   @param tSrc
      Source object to move-construct the new object from.
   @return
      Pointer to the new object.
   */
   T * reset_new(T tSrc = T()) {
      reset();
      value_t * pValue = get_ptr();
      // The constructor invoked is T::T(T &&), which should not throw.
      new(&pValue->t) T(std::move(tSrc));
      pValue->bConstructed = true;
      return &pValue->t;
   }

private:
   //! See TImpl::construct().
   virtual void construct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      pValue->bConstructed = false;
      // Note that pValue.t is left uninitialized.
   }

   //! See TImpl::destruct().
   virtual void destruct(void * p) const override {
      value_t * pValue = static_cast<value_t *>(p);
      if (pValue->bConstructed) {
         pValue->t.~T();
      }
   }

   //! See TImpl::get_ptr().
   value_t * get_ptr() const {
      return TImpl::template get_ptr<value_t>();
   }
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

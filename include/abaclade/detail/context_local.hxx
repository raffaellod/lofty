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

namespace abc { namespace detail {

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
      *get_ptr() = t;
      return *this;
   }
   context_local_value & operator=(T && t) {
      *get_ptr() = std::move(t);
      return *this;
   }

   /*! Implicit cast to T &.

   @return
      Reference to the object’s value.
   */
   operator T &() {
      return *get_ptr();
   }
   operator T const &() const {
      return *get_ptr();
   }

   /*! Returns true if the object’s value evaluates to true.

   @return
      Result of the evaluation of the object’s value in a boolean context.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *get_ptr() ? true : false;
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
      *get_ptr() = b;
      return *this;
   }

   /*! Implicit cast to bool &.

   @return
      Reference to the object’s value.
   */
   operator bool &() {
      return *get_ptr();
   }
   operator bool const &() const {
      return *get_ptr();
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

// Specialization for std::shared_ptr, which offers a few additional methods.
template <typename T, typename TImpl>
class context_local_value<std::shared_ptr<T>, TImpl> :
   private TImpl,
   public support_explicit_operator_bool<context_local_value<std::shared_ptr<T>, TImpl>> {
private:
   typedef std::shared_ptr<T> value_t;

public:
   //! Constructor.
   context_local_value() :
      TImpl(sizeof(std::shared_ptr<T>)) {
   }

   /*! Assignment operator.

   @param pt
      Source object.
   @return
      *this.
   */
   context_local_value & operator=(std::shared_ptr<T> const & pt) {
      *get_ptr() = pt;
      return *this;
   }
   context_local_value & operator=(std::shared_ptr<T> && pt) {
      *get_ptr() = std::move(pt);
      return *this;
   }

   /*! Implicit cast to std::shared_ptr<T> &.

   @return
      Reference to the shared pointer.
   */
   operator std::shared_ptr<T> &() {
      return *get_ptr();
   }
   operator std::shared_ptr<T> const &() const {
      return *get_ptr();
   }

   /*! Returns true if the pointer is not nullptr.

   @return
      false if the pointer is nullptr, or true otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *get_ptr() ? true : false;
   }

   /*! Explicit cast to T *.

   @return
      Pointer to the current T instance.
   */
   T * get() {
      return get_ptr()->get();
   }
   T const * get() const {
      return get_ptr()->get();
   }

   //! Releases the pointed-to object.
   void reset() {
      get_ptr()->reset();
   }

   /*! Returns true if no other pointers are referring to the object pointed to.

   @return
      true if *this is the only pointer to the owned object, or false otherwise.
   */
   bool unique() const {
      return get_ptr()->unique();
   }

   /*! Returns the number of references to the object pointed to.

   @return
      Reference count.
   */
   long use_count() const {
      return get_ptr()->use_count();
   }

private:
   //! See TImpl::construct().
   virtual void construct(void * p) const override {
      new(p) std::shared_ptr<T>();
   }

   //! See TImpl::destruct().
   virtual void destruct(void * p) const override {
      static_cast<std::shared_ptr<T> *>(p)->~shared_ptr();
   }

   //! See TImpl::get_ptr().
   std::shared_ptr<T> * get_ptr() const {
      return TImpl::template get_ptr<std::shared_ptr<T>>();
   }
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

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

}} //namespace abc::detail

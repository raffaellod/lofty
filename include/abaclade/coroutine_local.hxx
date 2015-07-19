/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

// Forward declarations.
class coroutine_local_var_impl;

//! Abaclade’s CRLS (TLS for coroutines) slot data manager.
/* TODO: this will need changes to support dynamic loading and unloading of libraries that depend on
Abaclade.

The m_pb byte array should be replaced with a map from library address/name to library-specific
CRLS, and each library would have its own byte array (keyed in the same way).
Loading a new library would add a new element in the maps (and in the CRLS block for each existing
coroutine), and unloading it would remove the library from all maps (and in the CRLS block for each
coroutine). */
class ABACLADE_SYM coroutine_local_storage :
   public collections::static_list<coroutine_local_storage, coroutine_local_var_impl>,
   public noncopyable {
public:
   //! Constructor.
   coroutine_local_storage();

   //! Destructor.
   ~coroutine_local_storage();

   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified coroutine_local_var_impl instance; it also initializes the m_pcrlviNext and
   m_ibStorageOffset members of the latter. This function will be called during initialization of a
   new dynamic library as it’s being loaded, not during normal run-time.

   @param pcrlvi
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   static void add_var(coroutine_local_var_impl * pcrlvi, std::size_t cb);

   /*! Destructs the registered coroutine local variables.

   @return
      true if any variables were destructed, or no constructed ones were found.
   */
   bool destruct_vars();

   /*! Returns a pointer to the specified offset in the storage.

   @return
      Pointer to the data store.
   */
   // Defined in thread_local.hxx.
   static coroutine_local_storage * get();

   /*! Accessor used by coroutine::scheduler to change m_pcrls.

   @param ppcrlsDefault
      Pointer to receive the address of m_crls.
   @param pppcrlsCurrent
      Pointer to receive the address of m_pcrls.
   */
   // Defined in thread_local.hxx.
   static void get_default_and_current_pointers(
      coroutine_local_storage ** ppcrlsDefault, coroutine_local_storage *** pppcrlsCurrent
   );

   /*! Returns a pointer to the specified offset in the coroutine-local data store.

   @param pcrlvi
      Pointer to the variable to retrieve.
   @return
      Corresponding pointer.
   */
   void * get_storage(coroutine_local_var_impl const * pcrlvi);

   ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)

private:
   //! Array of flags indicating whether each storage slot has been constructed.
   std::unique_ptr<bool[]> m_pbConstructed;
   //! Raw byte storage.
   std::unique_ptr<std::int8_t[]> m_pb;

   //! Count of variables registered with calls to add_var().
   static unsigned sm_cVars;
   //! Cumulative storage size registered with calls to add_var().
   static std::size_t sm_cb;
   /*! Tracks the value of sm_cb when coroutine_local_storage was instantiated. Changes occurring
   after that first time are a problem. */
   static std::size_t sm_cbFrozen;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Non-template implementation of abc::coroutine_local_value and abc::coroutine_local_ptr.
class ABACLADE_SYM coroutine_local_var_impl :
   public collections::static_list<coroutine_local_storage, coroutine_local_var_impl>::node,
   public noncopyable {
private:
   friend class coroutine_local_storage;

protected:
   /*! Constructor.

   @param cbObject
      Size of the object pointed to by the coroutine_local_value/coroutine_local_ptr subclass.
   */
   explicit coroutine_local_var_impl(std::size_t cbObject) {
      // Initializes the members of *this.
      coroutine_local_storage::add_var(this, cbObject);
   }

   /*! Constructs the coroutine-local value for a new coroutine. Invoked at most once for each
   coroutine.

   @param p
      Pointer to the memory block where the new value should be constructed.
   */
   virtual void construct(void * p) const = 0;

   /*! Destructs the coroutine-local value for a terminating coroutine. Invoked at most once for
   each coroutine.

   @param p
      Pointer to the value to be destructed.
   */
   virtual void destruct(void * p) const = 0;

   /*! Returns a pointer to the current coroutine’s copy of the variable.

   @return
      Pointer to the coroutine-local value for this object.
   */
   template <typename T>
   T * get_ptr() const {
      return static_cast<T *>(coroutine_local_storage::get()->get_storage(this));
   }

private:
   //! Offset of this variable in the CRLS block.
   std::size_t m_ibStorageOffset;
   //! Index of this variable in the CRLS block.
   unsigned m_iStorageIndex;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Variable with separate per-coroutine values. Variables of this type cannot be non-static class
members. */
template <typename T>
class coroutine_local_value :
   public detail::context_local_value<T, detail::coroutine_local_var_impl> {
private:
   typedef detail::context_local_value<T, detail::coroutine_local_var_impl> context_local;

public:
   //! See detail::context_local_value::operator=().
   coroutine_local_value & operator=(T const & t) {
      context_local::operator=(t);
      return *this;
   }
   coroutine_local_value & operator=(T && t) {
      context_local::operator=(std::move(t));
      return *this;
   }
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Coroutine-local pointer to an object. The memory this points to is permanently allocated for
each coroutine, and an instance of this class lets each coroutine access its own private copy of the
value pointed to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class coroutine_local_ptr : public detail::context_local_ptr<T, detail::coroutine_local_var_impl> {
};

} //namespace abc

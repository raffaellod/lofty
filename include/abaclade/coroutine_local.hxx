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
class coroutine_local_storage;

//! Abaclade’s CRLS (TLS for coroutines) slot data manager.
class ABACLADE_SYM coroutine_local_storage :
   public collections::static_list<
      coroutine_local_storage, context_local_var_impl<coroutine_local_storage>
   >,
   public context_local_storage_impl {
public:
   //! Constructor.
   coroutine_local_storage();

   //! Destructor.
   ~coroutine_local_storage();

   /*! Adds the specified size to the storage and assigns the corresponding offset within to the
   specified context_local_var_impl instance; it also initializes the m_pcrlviNext and
   m_ibStorageOffset members of the latter. This function will be called during initialization of a
   new dynamic library as it’s being loaded, not during normal run-time.

   @param pcrlvi
      Pointer to the new variable to assign storage to.
   @param cb
      Requested storage size.
   */
   static void add_var(context_local_var_impl<coroutine_local_storage> * pcrlvi, std::size_t cb) {
      context_local_storage_impl::add_var(&sm_sm, pcrlvi, cb);
   }

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

   ABC_COLLECTIONS_STATIC_LIST_DECLARE_SUBCLASS_STATIC_MEMBERS(coroutine_local_storage)

private:
   static static_members_t sm_sm;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Variable with separate per-coroutine values. Variables of this type cannot be non-static class
members. */
template <typename T>
class coroutine_local_value :
   public detail::context_local_value<T, detail::coroutine_local_storage> {
private:
   typedef detail::context_local_value<T, detail::coroutine_local_storage> context_local;

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
class coroutine_local_ptr : public detail::context_local_ptr<T, detail::coroutine_local_storage> {
};

} //namespace abc

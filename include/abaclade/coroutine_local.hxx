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

// Forward declaration
class coroutine_local_storage;

//! Abaclade’s CRLS variable registrar.
class ABACLADE_SYM coroutine_local_storage_registrar :
   public context_local_storage_registrar_impl,
   public collections::static_list_impl<
      coroutine_local_storage_registrar, context_local_storage_node<coroutine_local_storage>
   > {
public:
   /*! Returns the one and only instance of this class.

   @return
      *this.
   */
   static coroutine_local_storage_registrar & instance() {
      return static_cast<coroutine_local_storage_registrar &>(sm_dm.slib);
   }

private:
   //! Only instance of this class’ data.
   static data_members sm_dm;
};

}} //namespace abc::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Abaclade’s CRLS (TLS for coroutines) slot data manager.
class ABACLADE_SYM coroutine_local_storage : public context_local_storage_impl {
public:
   //! Registrar that variables will register with at program startup.
   typedef coroutine_local_storage_registrar registrar;

public:
   //! Default constructor.
   coroutine_local_storage();

   //! Destructor.
   ~coroutine_local_storage();

   /*! Returns the coroutine_local_storage instance for the current coroutine or thread.

   @return
      Reference to the data store.
   */
   // Defined in thread_local.hxx.
   static coroutine_local_storage & instance();

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
      context_local::operator=(_std::move(t));
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

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

// Forward declaration
class coroutine_local_storage;

//! Lofty’s CRLS variable registrar.
class LOFTY_SYM coroutine_local_storage_registrar :
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
      return static_cast<coroutine_local_storage_registrar &>(data_members_.list);
   }

private:
   //! Only instance of this class’ data.
   static data_members data_members_;
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Lofty’s CRLS (TLS for coroutines) slot data manager.
class LOFTY_SYM coroutine_local_storage : public context_local_storage_impl {
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

   @param default_crls
      Pointer to receive the address of default_crls.
   @param current_crls
      Pointer to receive the address of m_pcrls.
   */
   // Defined in thread_local.hxx.
   static void get_default_and_current_pointers(
      coroutine_local_storage ** default_crls, coroutine_local_storage *** current_crls
   );
};

}} //namespace lofty::_pvt

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Variable with separate per-coroutine values. Variables of this type cannot be non-static class members.
template <typename T>
class coroutine_local_value : public _pvt::context_local_value<T, _pvt::coroutine_local_storage> {
private:
   typedef _pvt::context_local_value<T, _pvt::coroutine_local_storage> context_local;

public:
   //! See _pvt::context_local_value::operator=().
   coroutine_local_value & operator=(T const & t) {
      context_local::operator=(t);
      return *this;
   }

   //! See _pvt::context_local_value::operator=().
   coroutine_local_value & operator=(T && t) {
      context_local::operator=(_std::move(t));
      return *this;
   }
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Coroutine-local pointer to an object. The memory this points to is permanently allocated for each
coroutine, and an instance of this class lets each coroutine access its own private copy of the value pointed
to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class coroutine_local_ptr : public _pvt::context_local_ptr<T, _pvt::coroutine_local_storage> {
};

} //namespace lofty

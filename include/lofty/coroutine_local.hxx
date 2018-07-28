/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COROUTINE_LOCAL_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_COROUTINE_LOCAL_HXX
#endif

#ifndef _LOFTY_COROUTINE_LOCAL_HXX_NOPUB
#define _LOFTY_COROUTINE_LOCAL_HXX_NOPUB

#include <lofty/_pvt/context_local.hxx>
#include <lofty/_std/utility.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

// Forward declaration
class coroutine_local_storage;

//! Lofty’s CRLS variable registrar.
class LOFTY_SYM coroutine_local_storage_registrar :
   public context_local_storage_registrar_impl,
   public collections::_LOFTY_PUBNS static_list_impl<
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
_LOFTY_PUBNS_BEGIN

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
      context_local::operator=(_std::_pub::move(t));
      return *this;
   }
};

_LOFTY_PUBNS_END
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Coroutine-local pointer to an object. The memory this points to is permanently allocated for each
coroutine, and an instance of this class lets each coroutine access its own private copy of the value pointed
to by it. Variables of this type cannot be non-static class members. */
template <typename T>
class coroutine_local_ptr : public _pvt::context_local_ptr<T, _pvt::coroutine_local_storage> {
};

_LOFTY_PUBNS_END
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COROUTINE_LOCAL_HXX_NOPUB

#ifdef _LOFTY_COROUTINE_LOCAL_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {

   using _pub::coroutine_local_ptr;
   using _pub::coroutine_local_value;

   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_COROUTINE_LOCAL_HXX

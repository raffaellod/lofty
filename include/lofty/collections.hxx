/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_COLLECTIONS_HXX
#define _LOFTY_COLLECTIONS_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Templated container data structures.

Contained classes must provide move constructors and assignment operators (cls::cls(cls &&) and
cls::operator=(cls &&)) if the copy constructor could result in execution of exception-prone code (e.g.
resource allocation).

Because move constructors are employed widely in container classes that need to provide strong exception
guarantee (fully transacted operation) even in case of moves, move constructors must not throw exceptions.
This requirement is relaxed for moves that involve two different classes, since these will not be used by
container classes. */
namespace collections {}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

//! Base for errors due to an invalid key or index being used on a mapping or sequence.
class LOFTY_SYM bad_access : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_access(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_access(bad_access const & src);

   //! Destructor.
   virtual ~bad_access() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_access & operator=(bad_access const & src);
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

//! Mapping (dictionary) key not found in the set of existing keys.
class LOFTY_SYM bad_key : public bad_access {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_key(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_key(bad_key const & src);

   //! Destructor.
   virtual ~bad_key() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_key & operator=(bad_key const & src);
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections {

//! Thrown when an attempt is made to access elements in a container outside its [begin(), end()) range.
class LOFTY_SYM out_of_range : public bad_access {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit out_of_range(errint_t err = 0);

   /*! Constructor.

   @param invalid
      Index that caused the error.
   @param min
      Minimum allowed index value.
   @param max
      Maximum allowed index value.
   @param err
      OS-defined error number associated to the exception.
   */
   out_of_range(std::ptrdiff_t invalid, std::ptrdiff_t min, std::ptrdiff_t max, errint_t err = 0);

   /*! Constructor.

   @param invalid
      Pointer that caused the error.
   @param min
      Minimum allowed pointer value.
   @param max
      Maximum allowed pointer value.
   @param err
      OS-defined error number associated to the exception.
   */
   out_of_range(void const * invalid, void const * min, void const * max, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   out_of_range(out_of_range const & src);

   //! Destructor.
   virtual ~out_of_range() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   out_of_range & operator=(out_of_range const & src);
};

}} //namespace lofty::collections

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_COLLECTIONS_HXX

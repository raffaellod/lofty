/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_COLLECTIONS_HXX
#define _ABACLADE_COLLECTIONS_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Templated container data structures.

Contained classes must provide move constructors and assignment operators (cls::cls(cls &&) and
cls::operator=(cls &&)) if the copy constructor could result in execution of exception-prone code
(e.g. resource allocation).

Because move constructors are employed widely in container classes that need to provide strong
exception guarantee (fully transacted operation) even in case of moves, move constructors must not
throw exceptions. This requirement is relaxed for moves that involve two different classes, since
these will not be used by container classes. */
namespace collections {}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

//! Base for errors due to an invalid key or index being used on a mapping or sequence.
class ABACLADE_SYM bad_access : public generic_error {
public:
   //! Default constructor.
   bad_access();

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_access(bad_access const & x);

   //! Destructor.
   virtual ~bad_access();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_access & operator=(bad_access const & x);
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

//! Mapping (dictionary) key not found in the set of existing keys.
class ABACLADE_SYM bad_key : public bad_access {
public:
   //! Default constructor.
   bad_key();

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_key(bad_key const & x);

   //! Destructor.
   virtual ~bad_key();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_key & operator=(bad_key const & x);

   //! See abc::collections::bad_access::init().
   void init(errint_t err = 0);
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace collections {

/*! Thrown when an attempt is made to access elements in a container outside its [begin(), end())
range. */
class ABACLADE_SYM out_of_range : public bad_access {
public:
#if 0
   // TODO: make abc::detail::exception_aggregator able to construct <stdexcept> classes.

   //! See abc::collections::bad_access::related_std.
   typedef _std::out_of_range related_std;
#endif

public:
   //! Default constructor.
   out_of_range();

   /*! Copy constructor.

   @param x
      Source object.
   */
   out_of_range(out_of_range const & x);

   //! Destructor.
   virtual ~out_of_range();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   out_of_range & operator=(out_of_range const & x);

   //! See abc::collections::bad_access::init().
   void init(errint_t err = 0);

   /*! See abc::collections::bad_access::init().

   @param iInvalid
      Index that caused the error.
   @param iMin
      Minimum allowed index value.
   @param iMax
      Maximum allowed index value.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(std::ptrdiff_t iInvalid, std::ptrdiff_t iMin, std::ptrdiff_t iMax, errint_t err = 0);

   /*! See abc::collections::bad_access::init().

   @param pInvalid
      Pointer that caused the error.
   @param pMin
      Minimum allowed pointer value.
   @param pMax
      Maximum allowed pointer value.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(void const * iInvalid, void const * iMin, void const * iMax, errint_t err = 0);

protected:
   //! See collections::bad_access::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Pointer that caused the error.
   void const * m_pInvalid;
   //! Minimum allowed pointer value.
   void const * m_pMin;
   //! Maximum allowed pointer value.
   void const * m_pMax;
   //! true if m_iMin and m_iMax have been provided.
   bool m_bRangeProvided:1;
   //! true if m_iMin and m_iMax have been provided.
   bool m_bWriteAsInts:1;
};

}} //namespace abc::collections

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_COLLECTIONS_HXX

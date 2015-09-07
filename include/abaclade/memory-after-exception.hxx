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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! An attempt was made to access an invalid memory location.
class ABACLADE_SYM address_error : public virtual generic_error {
public:
   //! Default constructor.
   address_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   address_error(address_error const & x);

   //! Destructor.
   virtual ~address_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   address_error & operator=(address_error const & x);

   /*! Returns the faulty address.

   @return
      Value of the pointer that was dereferenced.
   */
   void const * address() const {
      return m_pInvalid;
   }

   //! See abc::generic_error::init().
   void init(errint_t err = 0) {
      init(smc_szUnknownAddress, err);
   }

   /*! See abc::generic_error::init().

   @param pInvalid
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the error.
   */
   void init(void const * pInvalid, errint_t err = 0);

protected:
   //! See generic_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Address that could not be dereferenced.
   void const * m_pInvalid;
   //! String used as special value for when the address is not available.
   static char_t const smc_szUnknownAddress[];
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! An invalid memory access (e.g. misaligned pointer) was detected.
class ABACLADE_SYM access_error : public virtual address_error {
public:
   //! Default constructor.
   access_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   access_error(access_error const & x);

   //! Destructor.
   virtual ~access_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   access_error & operator=(access_error const & x);

   //! See abc::address_error::init().
   void init(void const * pInvalid, errint_t err = 0);
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! A memory allocation request could not be satisfied.
class ABACLADE_SYM allocation_error : public virtual generic_error {
public:
   //! See abc::generic_error::related_std.
   typedef _std::bad_alloc related_std;

   //! Default constructor.
   allocation_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   allocation_error(allocation_error const & x);

   //! Destructor.
   virtual ~allocation_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   allocation_error & operator=(allocation_error const & x);

   /*! Returns the amount of memory that could not be allocated.

   @return
      Amount of requested memory, in bytes.
   */
   std::size_t allocation_size() const {
      return m_cbFailed;
   }

   /*! See abc::generic_error::init().

   @param cbFailed
      Amount of memory that could not be allocated.
   @param err
      OS-defined error number associated to the error.
   */
   void init(std::size_t cbFailed, errint_t err = 0);

protected:
   //! See generic_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Amount of memory that could not be allocated.
   std::size_t m_cbFailed;
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! An attempt was made to access the memory location 0 (nullptr).
class ABACLADE_SYM null_pointer_error : public virtual address_error {
public:
   //! Default constructor.
   null_pointer_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   null_pointer_error(null_pointer_error const & x);

   //! Destructor.
   virtual ~null_pointer_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   null_pointer_error & operator=(null_pointer_error const & x);

   //! See abc::address_error::init().
   void init(errint_t err = 0);
};

}} //namespace abc::memory

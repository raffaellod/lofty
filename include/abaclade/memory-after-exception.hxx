/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! A memory allocation request could not be satisfied.
class ABACLADE_SYM bad_alloc : public generic_error {
public:
   //! See abc::generic_error::related_std.
   typedef _std::bad_alloc related_std;

   //! Default constructor.
   bad_alloc();

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_alloc(bad_alloc const & x);

   //! Destructor.
   virtual ~bad_alloc();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_alloc & operator=(bad_alloc const & x);

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

//! An attempt was made to access an invalid memory location.
class ABACLADE_SYM bad_pointer : public generic_error {
public:
   //! Default constructor.
   bad_pointer();

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_pointer(bad_pointer const & x);

   //! Destructor.
   virtual ~bad_pointer();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_pointer & operator=(bad_pointer const & x);

   /*! Returns the faulty pointer.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
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
class ABACLADE_SYM bad_pointer_alignment : public generic_error {
public:
   //! Default constructor.
   bad_pointer_alignment();

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_pointer_alignment(bad_pointer_alignment const & x);

   //! Destructor.
   virtual ~bad_pointer_alignment();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_pointer_alignment & operator=(bad_pointer_alignment const & x);

   /*! Returns the faulty pointer.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return m_pInvalid;
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
};

}} //namespace abc::memory

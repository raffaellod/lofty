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
   /*! Constructor.

   @param cbFailed
      Amount of memory that could not be allocated.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_alloc(std::size_t cbFailed, errint_t err = 0);

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_alloc(bad_alloc const & x);

   //! Destructor.
   virtual ~bad_alloc() ABC_STL_NOEXCEPT_TRUE();

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
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(errint_t err = 0);

   /*! Constructor.

   @param pInvalid
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(void const * pInvalid, errint_t err = 0);

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_pointer(bad_pointer const & x);

   //! Destructor.
   virtual ~bad_pointer() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   bad_pointer & operator=(bad_pointer const & x);

   /*! Returns the faulty pointer. If the returned value is 0xbadf00d, the pointer might have not
   been provided in the constructor.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return m_pInvalid;
   }

private:
   //! Address that could not be dereferenced.
   void const * m_pInvalid;
};

}} //namespace abc::memory

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace memory {

//! An invalid memory access (e.g. misaligned pointer) was detected.
class ABACLADE_SYM bad_pointer_alignment : public generic_error {
public:
   /*! Constructor.

   @param pInvalid
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer_alignment(void const * pInvalid, errint_t err = 0);

   /*! Copy constructor.

   @param x
      Source object.
   */
   bad_pointer_alignment(bad_pointer_alignment const & x);

   //! Destructor.
   virtual ~bad_pointer_alignment() ABC_STL_NOEXCEPT_TRUE();

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

private:
   //! Address that could not be dereferenced.
   void const * m_pInvalid;
};

}} //namespace abc::memory

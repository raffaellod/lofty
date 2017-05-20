/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

//! A memory allocation request could not be satisfied.
class LOFTY_SYM bad_alloc : public generic_error {
public:
   /*! Constructor.

   @param allocation_size
      Amount of memory that could not be allocated.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_alloc(std::size_t allocation_size, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_alloc(bad_alloc const & src);

   //! Destructor.
   virtual ~bad_alloc() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_alloc & operator=(bad_alloc const & src);

   /*! Returns the amount of memory that could not be allocated.

   @return
      Amount of requested memory, in bytes.
   */
   std::size_t allocation_size() const {
      return allocation_size_;
   }

private:
   //! Amount of memory that could not be allocated.
   std::size_t allocation_size_;
};

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

//! An attempt was made to access an invalid memory location.
class LOFTY_SYM bad_pointer : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(errint_t err = 0);

   /*! Constructor.

   @param ptr
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer(void const * ptr, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_pointer(bad_pointer const & src);

   //! Destructor.
   virtual ~bad_pointer() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_pointer & operator=(bad_pointer const & src);

   /*! Returns the faulty pointer. If the returned value is 0xbadf00d, the pointer might have not been
   provided in the constructor.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return ptr;
   }

private:
   //! Address that could not be dereferenced.
   void const * ptr;
};

}} //namespace lofty::memory

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace memory {

//! An invalid memory access (e.g. misaligned pointer) was detected.
class LOFTY_SYM bad_pointer_alignment : public generic_error {
public:
   /*! Constructor.

   @param ptr
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the exception.
   */
   explicit bad_pointer_alignment(void const * ptr, errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   bad_pointer_alignment(bad_pointer_alignment const & src);

   //! Destructor.
   virtual ~bad_pointer_alignment() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   bad_pointer_alignment & operator=(bad_pointer_alignment const & src);

   /*! Returns the faulty pointer.

   @return
      Pointer that was dereferenced.
   */
   void const * pointer() const {
      return ptr;
   }

private:
   //! Address that could not be dereferenced.
   void const * ptr;
};

}} //namespace lofty::memory

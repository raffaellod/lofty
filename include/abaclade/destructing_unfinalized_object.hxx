/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015-2016 Raffaello D. Di Napoli

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

#ifndef _ABACLADE_DESTRUCTING_UNFINALIZED_OBJECT_HXX
#define _ABACLADE_DESTRUCTING_UNFINALIZED_OBJECT_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Thrown when an instance of a class with a finalize() method was destructed before finalize() was
called on it. The owner ot the object should be changed to invoke finalize() before letting the
object go out of scope. */
class ABACLADE_SYM destructing_unfinalized_object : public exception {
public:
   /*! Constructor.

   @param ptObj
      Pointer to the object that was not finalized.
   */
   template <typename T>
   destructing_unfinalized_object(T const * ptObj) {
      write_what(ptObj, typeid(*ptObj));
   }

   /*! Copy constructor.

   @param x
      Source object.
   */
   destructing_unfinalized_object(destructing_unfinalized_object const & x);

   //! Destructor.
   virtual ~destructing_unfinalized_object() ABC_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   destructing_unfinalized_object & operator=(destructing_unfinalized_object const & x);

private:
   /*! Uses exception::what_ostream() to generate a what() string.

   @param pObj
      Pointer to the object that was not finalized.
   @param pti
      Type of *pObj.
   */
   void write_what(void const * pObj, _std::type_info const & ti);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_DESTRUCTING_UNFINALIZED_OBJECT_HXX

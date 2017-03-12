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

#ifndef _LOFTY_DESTRUCTING_UNFINALIZED_OBJECT_HXX
#define _LOFTY_DESTRUCTING_UNFINALIZED_OBJECT_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Thrown when an instance of a class with a finalize() method was destructed before finalize() was called on
it. The owner ot the object should be changed to invoke finalize() before letting the object go out of scope.
*/
class LOFTY_SYM destructing_unfinalized_object : public exception {
public:
   /*! Constructor.

   @param o
      Pointer to the object that was not finalized.
   */
   template <typename T>
   destructing_unfinalized_object(T const * o) {
      write_what(o, typeid(*o));
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   destructing_unfinalized_object(destructing_unfinalized_object const & src);

   //! Destructor.
   virtual ~destructing_unfinalized_object() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   destructing_unfinalized_object & operator=(destructing_unfinalized_object const & src);

private:
   /*! Uses exception::what_ostream() to generate a what() string.

   @param o
      Pointer to the object that was not finalized.
   @param type
      Type of *o.
   */
   void write_what(void const * o, _std::type_info const & type);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_DESTRUCTING_UNFINALIZED_OBJECT_HXX

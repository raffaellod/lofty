/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TESTING_UTILITY_HXX
#define _LOFTY_TESTING_UTILITY_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {

//! Utility classes useful for testing.
namespace utility {}

}} //namespace lofty::testing

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing { namespace utility {

// Forward declaration.
template <class T>
class container_data_ptr_tracker;

/*! Allows to declare a container_data_ptr_tracker instance using the auto keyword.

@param pt
   Pointer to the object to track.
@return
   Tracker instance.
*/
template <class T>
container_data_ptr_tracker<T> make_container_data_ptr_tracker(T const * pt);

}}} //namespace lofty::testing::utility

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing { namespace utility {

//! Allows to verify that its move constructor was invoked instead of the raw bytes being copied.
class class_with_internal_pointer {
public:
   //! Default constructor.
   class_with_internal_pointer() :
      p(&i),
      i(0xcafe) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   class_with_internal_pointer(class_with_internal_pointer && src) :
      p(&i),
      i(src.i) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   class_with_internal_pointer(class_with_internal_pointer const & src) :
      p(&i),
      i(src.i) {
   }

   /*! Validates that the object’s internal pointer has the expected value.

   @return
      true if the internal pointer is valid, or false otherwise.
   */
   bool validate() {
      return i == 0xcafe && p == &i;
   }

private:
   //! Pointer to m_i.
   std::uint16_t * p;
   //! Data referenced by p.
   std::uint16_t i;
};

}}} //namespace lofty::testing::utility

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing { namespace utility {

//! Tracks changes in the data() member of a container.
template <class T>
class container_data_ptr_tracker {
public:
   /*! Constructor. Starts tracking changes in the specified object.

   @param t_
      Object to track.
   */
   container_data_ptr_tracker(T const * t_) :
      t(t_),
      t_data(t->data()) {
   }

   /*! Checks if the monitored object’s data pointer has changed.

   @return
      true if the data pointer has changed, or false otherwise.
   */
   bool changed() {
      auto new_t_data = t->data();
      // Check if the data pointer has changed.
      if (new_t_data != t_data) {
         // Update the data pointer for the next call.
         t_data = new_t_data;
         return true;
      } else {
         return false;
      }
   }

private:
   //! Pointer to the T instance to be monitored.
   T const * t;
   //! Pointer to m_t’s data.
   typename T::const_pointer t_data;
};


// Now this can be defined.

template <class T>
inline container_data_ptr_tracker<T> make_container_data_ptr_tracker(T const * t) {
   return container_data_ptr_tracker<T>(t);
}

}}} //namespace lofty::testing::utility

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing { namespace utility {

/*! This class is meant for use in containers to track when items are copied, when they’re moved, and to check
if individual instances have been copied instead of being moved. */
class LOFTY_TESTING_SYM instances_counter {
public:
   //! Default constructor.
   instances_counter() :
      unique_(++next_unique) {
      ++new_;
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   instances_counter(instances_counter && src) :
      unique_(src.unique_) {
      ++moves_;
   }

   /*! Copy constructor. Doesn’t use its argument since the only non-static member (unique_) is always
   generated. */
   instances_counter(instances_counter const &) :
      unique_(++next_unique) {
      ++copies_;
   }

   /*! Move-assigment operator.

   @param src
      Source object.
   */
   instances_counter & operator=(instances_counter && src) {
      unique_ = src.unique_;
      ++moves_;
      return *this;
   }

   /*! Copy-assigment operator. Doesn’t use its argument since the only non-static member (unique_) is always
   generated.

   @return
      *this.
   */
   instances_counter & operator=(instances_counter const &) {
      unique_ = ++next_unique;
      ++copies_;
      return *this;
   }

   /*! Equality relational operator. Should always return false, since no two simultaneously-living instances
   should have the same unique value.

   @param other
      Object to compare to *this.
   @return
      true if *this has the same unique value as other, or false otherwise.
   */
   bool operator==(instances_counter const & other) const {
      return unique_ == other.unique_;
   }

   /*! Inequality relational operator. Should always return true, since no two simultaneously-living instances
   should have the same unique value.

   @param other
      Object to compare to *this.
   @return
      true if *this has a different unique value than other, or false otherwise.
   */
   bool operator!=(instances_counter const & other) const {
      return !operator==(other);
   }

   /*! Returns the count of instances created, excluding moved ones.

   @return
      Count of instances.
   */
   static std::size_t copies() {
      return copies_;
   }

   /*! Returns the count of moved instances.

   @return
      Count of instances.
   */
   static std::size_t moves() {
      return moves_;
   }

   /*! Returns the count of new (not copied, not moved) instances. Useful to track how many instances have not
   been created from a source instance, perhaps only to be copy- or move-assigned later, which would be less
   efficient than just copy- or move-constructing, 

   @return
      Count of instances.
   */
   static std::size_t new_insts() {
      return new_;
   }

   //! Resets the copies/moves/new instance counts.
   static void reset_counts() {
      copies_ = 0;
      moves_ = 0;
      new_ = 0;
   }

   /*! Returns the unique value associated to this object.

   @return
      Unique value.
   */
   int unique() const {
      return unique_;
   }

private:
   //! Unique value associated to this object.
   int unique_;
   //! Count of instances created, excluding moved ones.
   static std::size_t copies_;
   //! Count of moved instances.
   static std::size_t moves_;
   //! Count of new (not copied, not moved) instances.
   static std::size_t new_;
   //! Value of unique_ for the next instance.
   static int next_unique;
};

}}} //namespace lofty::testing::utility

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TESTING_UTILITY_HXX

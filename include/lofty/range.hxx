/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_RANGE_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_RANGE_HXX
#endif

#ifndef _LOFTY_RANGE_HXX_NOPUB
#define _LOFTY_RANGE_HXX_NOPUB

#include <lofty/explicit_operator_bool.hxx>
#include <lofty/_std/iterator.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {
_LOFTY_PUBNS_BEGIN

/*! Represents an iterable interval of values defined by a beginning and an end, inclusive and exclusive
respectively. */
template <typename T>
class range : public support_explicit_operator_bool<range<T>> {
public:
   //! Iterator for range values.
   class iterator : public _std::_LOFTY_PUBNS iterator<_std::_LOFTY_PUBNS bidirectional_iterator_tag, T> {
   public:
      /*! Constructor.

      @param t
         Current value.
      */
      explicit iterator(T t_) :
         t(_std::_pub::move(t_)) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current value.
      */
      T & operator*() {
         return t;
      }

      /*! Dereferencing operator.

      @return
         Const reference to the current value.
      */
      T const & operator*() const {
         return t;
      }

      /*! Dereferencing member access operator.

      @return
         Pointer to the current value.
      */
      T * operator->() {
         return &t;
      }

      /*! Dereferencing member access operator.

      @return
         Const pointer to the current value.
      */
      T const * operator->() const {
         return &t;
      }

      /*! Preincrement operator.

      @return
         *this after it’s moved to the value following the one currently referenced.
      */
      iterator & operator++() {
         ++t;
         return *this;
      }

      /*! Postincrement operator.

      @return
         Iterator to the value following the one referenced by this iterator.
      */
      iterator operator++(int) {
         return iterator(t++);
      }

      /*! Predecrement operator.

      @return
         *this after it’s moved to the value preceding the one currently referenced.
      */
      iterator & operator--() {
         --t;
         return *this;
      }

      /*! Postdecrement operator.

      @return
         Iterator to the value preceding the one referenced by this iterator.
      */
      iterator operator--(int) {
         return iterator(t--);
      }

      /*! Equality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this has the same value as other, or false otherwise.
      */
      bool operator==(iterator const & other) const {
         return t == other.t;
      }

      /*! Inequality relational operator.

      @param other
         Object to compare to *this.
      @return
         true if *this has a different value than other, or false otherwise.
      */
      bool operator!=(iterator const & other) const {
         return !operator==(other);
      }

   private:
      //! Current value.
      T t;
   };

   typedef iterator const_iterator;
   typedef _std::_LOFTY_PUBNS reverse_iterator<iterator> reverse_iterator;
   typedef _std::_LOFTY_PUBNS reverse_iterator<const_iterator> const_reverse_iterator;

public:
   //! Default constructor. Constructs an empty range.
   range() :
      begin_(),
      end_() {
   }

   /*! Constructor.

   @param begin__
      First value in the range.
   @param end__
      Value beyond the last one in the range.
   */
   range(T begin__, T end__) :
      begin_(_std::_pub::move(begin__)),
      end_(_std::_pub::move(end__)) {
   }

   /*! Boolean evaluation operator.

   @return
      true if the range is non-empty, or false if it’s empty.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return begin_ != end_;
   }

   /*! Right-shift-assignment operator.

   @param t
      Amount by which to shift/translate the range towards positive infinity.
   @return
      *this after its begin() and end() have been increased by t.
   */
   range & operator>>=(T const & t) {
      begin_ += t;
      end_ += t;
      return *this;
   }

   /*! Left-shift-assignment operator.

   @param t
      Amount by which to shift/translate the range towards negative infinity.
   @return
      *this after its begin() and end() have been decreased by t.
   */
   range & operator<<=(T const & t) {
      begin_ -= t;
      end_ -= t;
      return *this;
   }

   /*! Right-shift operator.

   @param t
      Amount by which to shift/translate the range towards positive infinity.
   @return
      Range covering *this with begin() and end() increased by t.
   */
   range operator>>(T const & t) const {
      return range(*this) >>= t;
   }

   /*! Left-shift operator.

   @param t
      Amount by which to shift/translate the range towards negative infinity.
   @return
      Range covering *this with begin() and end() decreased by t.
   */
   range operator<<(T const & t) const {
      return range(*this) <<= t;
   }

   /*! Equality relational operator.

   @param r
      Object to compare to *this.
   @return
      true if *this has the same begin and end as r, or false otherwise.
   */
   bool operator==(range const & r) const {
      return begin_ == r.begin_ && end_ == r.end_;
   }

   /*! Inequality relational operator.

   @param r
      Object to compare to *this.
   @return
      true if *this has a different begin and/or end than r, or false otherwise.
   */
   bool operator!=(range const & r) const {
      return !operator==(r);
   }

   /*! Returns an iterator to the start of the range.

   @return
      Iterator to the first value in the range.
   */
   iterator begin() const {
      return iterator(begin_);
   }

   /*! Returns an iterator to the start of the range. Same as begin().

   @return
      Iterator to the first value in the range.
   */
   iterator cbegin() const {
      return begin();
   }

   /*! Returns an iterator to the end of the range. Same as end().

   @return
      Value beyond the last in the range.
   */
   iterator cend() const {
      return end();
   }

   /*! Returns true if the specified value is included in the range.

   @param t
      Value to check for inclusion.
   @return
      true if t is included in [begin(), end()), or false otherwise.
   */
   bool contains(T const & t) const {
      return t >= begin_ && t < end_;
   }

   /*! Returns an iterator to the end of the range.

   @return
      Value beyond the last in the range.
   */
   iterator end() const {
      return iterator(end_);
   }

   /*! Returns the count of values included in the range.

   @return
      Count of values.
   */
   std::size_t size() const {
      return begin_ < end_ ? static_cast<std::size_t>(end_ - begin_) : 0;
   }

private:
   //! First value in the range.
   T begin_;
   //! Value beyond the last one in the range.
   T end_;
};

/*! Creates a lofty::range by inferring the range type from its arguments.

@param begin
   First value in the range.
@param end
   Value beyond the last one in the range.
@return
   Range covering [begin, end).
*/
template <typename T>
range<T> make_range(T begin, T end) {
   return range<T>(_std::_pub::move(begin), _std::_pub::move(end));
}

_LOFTY_PUBNS_END
} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_RANGE_HXX_NOPUB

#ifdef _LOFTY_RANGE_HXX
   #undef _LOFTY_NOPUB

   namespace lofty {
      using _pub::make_range;
      using _pub::range;
   }

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_RANGE_HXX

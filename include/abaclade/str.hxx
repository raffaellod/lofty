/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::str_base


namespace abc {

// Forward declarations.
class istr;
class dmstr;

/** Base class for strings. Unlike C or STL strings, instances do not implcitly have an accessible
trailing NUL character.

See [DOC:4019 abc::*str and abc::*vector design] for implementation details for this and all the
abc::*str classes.
*/
class ABACLADE_SYM str_base :
   protected _raw_trivial_vextr_impl,
   public support_explicit_operator_bool<str_base> {
public:

   typedef char_t value_type;
   typedef char_t * pointer;
   typedef char_t const * const_pointer;
   typedef char_t & reference;
   typedef char_t const & const_reference;
   typedef size_t size_type;
   typedef ptrdiff_t difference_type;
   typedef text::codepoint_iterator<false> iterator;
   typedef text::codepoint_iterator<true> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


public:

   /** Pointer to a C-style, NUL-terminated character array that may or may not share memory with
   an abc::*str instance. */
   typedef std::unique_ptr<
      char_t const [],
      memory::conditional_deleter<char_t const [], memory::freeing_deleter<char_t const []>>
   > c_str_pointer;


public:

   /** Allows automatic cross-class-hierarchy casts.

   return
      Const reference to *this as an immutable string.
   */
   operator istr const &() const;


   /** Character access operator.

   i
      Character index. See abc::str_base::translate_index() for allowed index values.
   return
      Character at index i.
   */
   char32_t operator[](intptr_t i) const {
      // TODO: convert to char32_t for real.
      return *translate_index(i);
   }


   /** Returns true if the length is greater than 0.

   return
      true if the string is not empty, or false otherwise.
   */
   explicit_operator_bool() const {
      // Use int8_t to avoid multiplying by sizeof(char_t) when all we need is a greater-than check.
      return _raw_vextr_impl_base::end<int8_t>() > _raw_vextr_impl_base::begin<int8_t>();
   }


   /** Returns a forward iterator set to the first element.

   return
      Forward iterator to the first element.
   */
   const_iterator begin() const {
      return const_iterator(chars_begin(), this);
   }


   /** Returns a pointer to a NUL-terminated version of the string.

   If the string already includes a NUL terminator, the returned pointer will refer to the same
   character array, and it will not own it; if the string does not include a NUL terminator, the
   returned pointer will own a NUL-terminated copy of *this.

   The returned pointer should be thought of as having a very short lifetime, and it should never be
   stored of manipulated.

   TODO: provide non-immutable version mstr::to_c_str().

   return
      NUL-terminated version of the string.
   */
   c_str_pointer c_str() const;


   /** Returns the maximum number of characters the string buffer can currently hold.

   return
      Size of the string buffer, in characters.
   */
   size_t capacity() const {
      return _raw_vextr_impl_base::capacity<char_t>();
   }


   /** Returns a const forward iterator set to the first element.

   return
      Forward iterator to the first element.
   */
   const_iterator cbegin() const {
      return const_iterator(chars_begin(), this);
   }


   /** Returns a const forward iterator set beyond the last element.

   return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_iterator(chars_end(), this);
   }


   /** See _raw_trivial_vextr_impl::begin().
   */
   char_t * chars_begin() {
      return _raw_trivial_vextr_impl::begin<char_t>();
   }
   char_t const * chars_begin() const {
      return _raw_trivial_vextr_impl::begin<char_t>();
   }


   /** See _raw_trivial_vextr_impl::end().
   */
   char_t * chars_end() {
      return _raw_trivial_vextr_impl::end<char_t>();
   }
   char_t const * chars_end() const {
      return _raw_trivial_vextr_impl::end<char_t>();
   }


   /** Returns a const reverse iterator set to the last element.

   return
      Reverse iterator to the last element.
   */
   const_reverse_iterator crbegin() const {
      return const_reverse_iterator(cend());
   }


   /** Returns a const reverse iterator set to before the first element.

   return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator crend() const {
      return const_reverse_iterator(cbegin());
   }


   /** Returns the string, encoded as requested, into a byte vector.

   enc
      Requested encoding.
   bNulT
      If true, the resulting vector will contain an additional NUL terminator (using as many vector
      elements as the destination encoding’s character size); if false, no NUL terminator will be
      present.
   return
      Resulting byte vector.
   */
   dmvector<uint8_t> encode(text::encoding enc, bool bNulT) const;


   /** Returns a forward iterator set beyond the last element.

   return
      Forward iterator to the first element.
   */
   const_iterator end() const {
      return const_iterator(chars_end(), this);
   }


   /** Returns true if the string ends with a specified suffix.

   s
      String that *this should end with.
   return
      true if *this ends with the specified suffix, or false otherwise.
   */
   bool ends_with(istr const & s) const;


   /** Searches for and returns the first occurrence of the specified character or substring.

   chNeedle
      Character to search for.
   sNeedle
      String to search for.
   itWhence
      Iterator to the first character whence the search should start. When not specified, it
      defaults to cbegin().
   return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find(char32_t chNeedle) const {
      return find(chNeedle, cbegin());
   }
   const_iterator find(char32_t chNeedle, const_iterator itWhence) const;
   const_iterator find(istr const & sNeedle) const {
      return find(sNeedle, cbegin());
   }
   const_iterator find(istr const & sNeedle, const_iterator itWhence) const;


   /** Searches for and returns the last occurrence of the specified character or substring.

   chNeedle
      Character to search for.
   sNeedle
      String to search for.
   itWhence
      Iterator to the last character whence the search should start. When not specified, it
      defaults to cend().
   return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find_last(char32_t chNeedle) const {
      return find_last(chNeedle, cend());
   }
   const_iterator find_last(char32_t chNeedle, const_iterator itWhence) const;
   const_iterator find_last(istr const & sNeedle) const {
      return find_last(sNeedle, cend());
   }
   const_iterator find_last(istr const & sNeedle, const_iterator itWhence) const;


   /** Uses the current content of the string to generate a new one using io::text::writer::print().

   ts
      Replacement values.
   return
      Resulting string.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES

   template <typename ... Ts>
   dmstr format(Ts const & ... ts) const;

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

   dmstr format() const;
   template <typename T0>
   dmstr format(T0 const & t0) const;
   template <typename T0, typename T1>
   dmstr format(T0 const & t0, T1 const & t1) const;
   template <typename T0, typename T1, typename T2>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2) const;
   template <typename T0, typename T1, typename T2, typename T3>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   dmstr format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   dmstr format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   ) const;

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else


   /** Returns a reverse iterator set to the last element.

   return
      Reverse iterator to the last element.
   */
   const_reverse_iterator rbegin() const {
      return const_reverse_iterator(end());
   }


   /** Returns a reverse iterator set to before the first element.

   return
      Reverse iterator to before the first element.
   */
   const_reverse_iterator rend() const {
      return const_reverse_iterator(begin());
   }


   /** Returns size of the string, in bytes.

   return
      Size of the string.
   */
   size_t size_in_bytes() const {
      return _raw_trivial_vextr_impl::size<int8_t>();
   }


   /** Returns size of the string, in characters.

   return
      Size of the string.
   */
   size_t size_in_chars() const {
      return _raw_trivial_vextr_impl::size<char_t>();
   }


   /** Returns size of the string, in code points.

   return
      Size of the string.
   */
   size_t size_in_codepoints() const {
      return text::host_str_traits::size_in_codepoints(chars_begin(), chars_end());
   }


   /** Returns true if the string starts with a specified prefix.

   s
      String that *this should start with.
   return
      true if *this starts with the specified suffix, or false otherwise.
   */
   bool starts_with(istr const & s) const;


   /** Returns a portion of the string.

   ichBegin
      Index of the first character of the substring. See abc::str_base::translate_range() for
      allowed begin index values.
   ichEnd
      Index of the last character of the substring, exclusive. See abc::str_base::translate_range()
      for allowed end index values.
   itBegin
      Iterator to the first character of the substring.
   itEnd
      Iterator to past the end of the substring.
   return
      Substring of *this.
   */
   dmstr substr(intptr_t ichBegin) const;
   dmstr substr(intptr_t ichBegin, intptr_t ichEnd) const;
   dmstr substr(const_iterator itBegin) const;
   dmstr substr(const_iterator itBegin, const_iterator itEnd) const;


protected:

   /** Constructor.

   cbEmbeddedCapacity
      Size of the embedded character array, in bytes, or 0 if no embedded array is present.
   pchConstSrc
      Pointer to a string that will be adopted by the str_base as read-only.
   cchSrc
      Count of characters in the string pointed to by pchConstSrc.
   bNulT
      true if the array pointed to by pchConstSrc is a NUL-terminated string, or false otherwise.
   */
   str_base(size_t cbEmbeddedCapacity) :
      _raw_trivial_vextr_impl(cbEmbeddedCapacity) {
   }
   str_base(char_t const * pchConstSrc, size_t cchSrc, bool bNulT) :
      _raw_trivial_vextr_impl(pchConstSrc, pchConstSrc + cchSrc, bNulT) {
   }


   /** See _raw_trivial_vextr_impl::assign_copy().

   TODO: comment signature.
   */
   void assign_copy(char_t const * pchBegin, char_t const * pchEnd) {
      _raw_trivial_vextr_impl::assign_copy(pchBegin, pchEnd);
   }


   /** TODO: comment.
   */
   void assign_concat(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   ) {
      _raw_trivial_vextr_impl::assign_concat(pch1Begin, pch1End, pch2Begin, pch2End);
   }


   /** See _raw_trivial_vextr_impl::assign_move().

   s
      Source string.
   */
   void assign_move(str_base && s) {
      _raw_trivial_vextr_impl::assign_move(static_cast<_raw_trivial_vextr_impl &&>(s));
   }


   /** See _raw_trivial_vextr_impl::assign_move_dynamic_or_move_items().

   s
      Source string.
   */
   void assign_move_dynamic_or_move_items(str_base && s) {
      _raw_trivial_vextr_impl::assign_move_dynamic_or_move_items(
         static_cast<_raw_trivial_vextr_impl &&>(s)
      );
   }


   /** See _raw_trivial_vextr_impl::assign_share_raw_or_copy_desc().

   s
      Source string.
   */
   void assign_share_raw_or_copy_desc(str_base const & s) {
      _raw_trivial_vextr_impl::assign_share_raw_or_copy_desc(s);
   }


   /** Converts a possibly negative character index into an iterator, throwing an exception if the
   result is out of bounds for the character array.

   ich
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the character array by adding this->size_in_codepoints() to it.
   return
      Iterator to the character.
   */
   const_iterator translate_index(intptr_t ich) const;


   /** Converts a possibly negative character index into an iterator.

   ich
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the character array by adding this->size_in_codepoints() to it.
   return
      A pair containing the resulting iterator and a flag that indicates whether the iterator was
      clipped to end() (for non-negative ich) or begin() (for negative ich).
   */
   std::pair<const_iterator, bool> translate_index_nothrow(intptr_t ich) const;


   /** Converts a left-closed, right-open interval with possibly negative character indices into one
   consisting of two iterators.

   ichBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the character array by adding
      this->size_in_codepoints() to it.
   ichEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the character array by
      adding this->size_in_codepoints() to it.
   return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [end(), end()) if the indices represent an empty interval after being adjusted.
   */
   std::pair<const_iterator, const_iterator> translate_range(
      intptr_t ichBegin, intptr_t ichEnd
   ) const;


protected:

   /** Single NUL terminator. */
   static char_t const smc_chNul;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   inline bool operator op(abc::str_base const & s1, abc::str_base const & s2) { \
      return abc::text::host_str_traits::compare( \
         s1.chars_begin(), s1.chars_end(), s2.chars_begin(), s2.chars_end() \
      ) op 0; \
   } \
   template <size_t t_cch> \
   inline bool operator op(abc::str_base const & s, abc::char_t const (& ach)[t_cch]) { \
      abc::char_t const * pchEnd(ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0')); \
      return abc::text::host_str_traits::compare( \
         s.chars_begin(), s.chars_end(), ach, pchEnd \
      ) op 0; \
   } \
   template <size_t t_cch> \
   inline bool operator op(abc::char_t const (& ach)[t_cch], abc::str_base const & s) { \
      abc::char_t const * pchEnd(ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0')); \
      return abc::text::host_str_traits::compare( \
         ach, pchEnd, s.chars_begin(), s.chars_end() \
      ) op 0; \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


namespace std {

// Specialization of std::hash.
template <>
struct ABACLADE_SYM hash<abc::str_base> {

   size_t operator()(abc::str_base const & s) const;
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::istr


namespace abc {

// Forward declaration.
class mstr;

/** str_base-derived class, to be used as “the” string class in most cases. It cannot be modified
in-place, which means that it shouldn’t be used in code performing intensive string manipulations.
*/
class istr :
   public str_base {
public:

   /** Constructor.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   pchBegin
      Pointer to the beginning of the source stirng.
   pchEnd
      Pointer to beyond the end of the source stirng.
   psz
      Pointer to the source NUL-terminated string literal.
   cch
      Count of characters in the array pointed to be psz.
   */
   istr() :
      str_base(0) {
   }
   istr(istr const & s) :
      str_base(0) {
      assign_share_raw_or_copy_desc(s);
   }
   istr(istr && s) :
      str_base(0) {
      // Non-const, so it can’t be anything but a real istr, so it owns its character array.
      assign_move(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the istr && overload.
   istr(mstr && s);
   istr(dmstr && s);
   template <size_t t_cch>
   istr(char_t const (& ach)[t_cch]) :
      str_base(ach, t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'), ach[t_cch - 1 /*NUL*/] == '\0') {
   }
   istr(char_t const * pchBegin, char_t const * pchEnd) :
      str_base(0) {
      assign_copy(pchBegin, pchEnd);
   }
   istr(unsafe_t, char_t const * psz) :
      str_base(psz, text::size_in_chars(psz), true) {
   }
   istr(unsafe_t, char_t const * psz, size_t cch) :
      str_base(psz, cch, false) {
   }


   /** Assignment operator.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   return
      *this.
   */
   istr & operator=(istr const & s) {
      assign_share_raw_or_copy_desc(s);
      return *this;
   }
   istr & operator=(istr && s) {
      // Non-const, so it can’t be anything but a real istr, so it owns its character array.
      assign_move(std::move(s));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the istr && overload.
   istr & operator=(mstr && s);
   istr & operator=(dmstr && s);
   template <size_t t_cch>
   istr & operator=(char_t const (& ach)[t_cch]) {
      // This order is safe, because the constructor invoked on the next line won’t throw.
      this->~istr();
      ::new(this) istr(ach);
      return *this;
   }
};


// Now these can be implemented.

inline str_base::operator istr const &() const {
   return *static_cast<istr const *>(this);
}

} //namespace abc


namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::istr> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::mstr


namespace abc {

/** str_base-derived class, to be used as argument type for functions that want to modify a string
argument, since unlike istr, it allows in-place alterations to the string. Both smstr and dmstr
are automatically converted to this.
*/
class ABACLADE_SYM mstr :
   public str_base {
public:

   /** Assignment operator.

   s
      Source string.
   return
      *this.
   */
   mstr & operator=(mstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   mstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the mstr && overload.
   mstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   mstr & operator=(dmstr && s);


   /** Concatenation-assignment operator.

   ch
      Character to append.
   s
      String to append.
   return
      *this.
   */
   mstr & operator+=(char32_t ch) {
      char_t ach[text::host_char_traits::max_codepoint_length];
      append(ach, size_t(text::host_char_traits::codepoint_to_chars(ch, ach) - ach));
      return *this;
   }
   mstr & operator+=(istr const & s) {
      append(s.chars_begin(), s.size_in_chars());
      return *this;
   }


   /** Same as operator+=(), but for multi-argument overloads.

   pchAdd
      Pointer to an array of characters to append.
   cchAdd
      Count of characters in the array pointed to by pchAdd.
   */
   void append(char_t const * pchAdd, size_t cchAdd) {
      _raw_trivial_vextr_impl::insert(
         _raw_vextr_impl_base::size<int8_t>(), pchAdd, sizeof(char_t) * cchAdd
      );
   }


   /** See str_base::begin(). Here also available in non-const overload.
   */
   iterator begin() {
      return iterator(chars_begin(), this);
   }
   const_iterator begin() const {
      return str_base::begin();
   }


   /** See str_base::end(). Here also available in non-const overload.
   */
   iterator end() {
      return iterator(chars_end(), this);
   }
   const_iterator end() const {
      return str_base::end();
   }


   /** Grows the character array until the specified callback succeeds in filling it and returns a
   number of needed characters that’s less than the size of the buffer. For example, for cchMax == 3
   (NUL terminator included), it must return <= 2 (NUL excluded).

   This method is not transaction-safe; if an exception is thrown in the callback or elsewhere,
   *this will not be restored to its previous state.

   TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

   Note: this method probably benefits from being inlined in spite of its size, because in most
   cases the callback will be just a lambda wrapper around some API/OS function, so when the
   compiler inlines this method, it will most likely blend its code with the (also inlined) lambda,
   probably resulting in some optimizations that would be otherwise missed.

   fnRead
      Callback that is invoked to fill up the string buffer.
      pch
         Pointer to the beginning of the buffer to be filled up by the callback.
      cchMax
         Size of the buffer pointed to by pch.
      return
         Count of characters written to the buffer pointed to by pch. If less than cchMax, this will
         be the final count of characters of *this; otherwise, fnRead will be called once more with
         a larger cchMax after the string buffer has been enlarged.
   */
   void grow_for(std::function<size_t (char_t * pch, size_t cchMax)> fnRead) {
      typedef _raw_vextr_impl_base rvib;
      // The initial size avoids a few reallocations (* smc_iGrowthRate ** 2).
      // Multiplying by smc_iGrowthRate should guarantee that set_capacity() will allocate exactly
      // the requested number of characters, eliminating the need to query back with capacity().
      size_t cchRet, cchMax(rvib::smc_cbCapacityMin * rvib::smc_iGrowthRate);
      do {
         cchMax *= rvib::smc_iGrowthRate;
         set_capacity(cchMax, false);
         cchRet = fnRead(chars_begin(), cchMax);
      } while (cchRet >= cchMax);
      // Finalize the length.
      set_size_in_chars(cchRet);
   }


   /** See str_base::rbegin(). Here also available in non-const overload.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(chars_end(), this));
   }
   const_reverse_iterator rbegin() const {
      return str_base::rbegin();
   }


   /** See str_base::rend(). Here also available in non-const overload.
   */
   reverse_iterator rend() {
      return reverse_iterator(iterator(chars_begin(), this));
   }
   const_reverse_iterator rend() const {
      return str_base::rend();
   }


   /** See _raw_trivial_vextr_impl::set_capacity().

   cchMin
      Minimum count of characters requested.
   bPreserve
      If true, the previous contents of the string will be preserved even if the reallocation
      causes the string to switch to a different character array.
   */
   void set_capacity(size_t cchMin, bool bPreserve) {
      _raw_trivial_vextr_impl::set_capacity(sizeof(char_t) * cchMin, bPreserve);
   }


   /** Changes the length of the string. If the string needs to be lengthened, the added characters
   will be left uninitialized.

   cch
      New length of the string.
   bClear
      If true, the string will be cleared after being resized; if false, no characters will be
      changed.
   */
   void set_size_in_chars(size_t cch, bool bClear = false) {
      _raw_trivial_vextr_impl::set_size(sizeof(char_t) * cch);
      if (bClear) {
         memory::clear(chars_begin(), cch);
      }
   }


protected:

   /** Constructor. See str_base::str_base().
   */
   mstr(size_t cbEmbeddedCapacity) :
      str_base(cbEmbeddedCapacity) {
   }
};


// Now these can be implemented.

inline istr::istr(mstr && s) :
   str_base(0) {
   assign_move_dynamic_or_move_items(std::move(s));
}


inline istr & istr::operator=(mstr && s) {
   assign_move_dynamic_or_move_items(std::move(s));
   return *this;
}

} //namespace abc


namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::mstr> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dmstr


namespace abc {

/** mstr-derived class, good for clients that need in-place manipulation of strings whose length is
unknown at design time.
*/
class dmstr :
   public mstr {
public:

   /** Constructor.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   pchBegin
      Pointer to the beginning of the source stirng.
   pchEnd
      Pointer to beyond the end of the source stirng.
   pch1Begin
      Pointer to the beginning of the left source stirng to concatenate.
   pch1End
      Pointer to beyond the end of the left source stirng.
   pch2Begin
      Pointer to the beginning of the right source stirng to concatenate.
   pch2End
      Pointer to beyond the end of the right source stirng.
   */
   dmstr() :
      mstr(0) {
   }
   dmstr(dmstr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   dmstr(dmstr && s) :
      mstr(0) {
      assign_move(std::move(s));
   }
   dmstr(istr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(istr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   dmstr(mstr const & s) :
      mstr(0) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(mstr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   template <size_t t_cch>
   dmstr(char_t const (& ach)[t_cch]) :
      mstr(0) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
   }
   dmstr(char_t const * pchBegin, char_t const * pchEnd) :
      mstr(0) {
      assign_copy(pchBegin, pchEnd);
   }
   dmstr(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   ) :
      mstr(0) {
      assign_concat(pch1Begin, pch1End, pch2Begin, pch2End);
   }


   /** Assignment operator.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   return
      *this.
   */
   dmstr & operator=(dmstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   dmstr & operator=(dmstr && s) {
      assign_move(std::move(s));
      return *this;
   }
   dmstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   dmstr & operator=(mstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(mstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   template <size_t t_cch>
   dmstr & operator=(char_t const (& ach)[t_cch]) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
      return *this;
   }


#if ABC_HOST_MSC

   /** MSC16 BUG: re-defined here because MSC16 seems unable to see the definition in str_base. See
	str_base::operator istr const &().
   */
   operator istr const &() const {
      return str_base::operator istr const &();
   }

#endif //if ABC_HOST_MSC
};


// Now these can be implemented.

inline dmstr str_base::substr(intptr_t ichBegin) const {
   return substr(ichBegin, intptr_t(size_in_chars()));
}
inline dmstr str_base::substr(intptr_t ichBegin, intptr_t ichEnd) const {
   auto range(translate_range(ichBegin, ichEnd));
   return dmstr(range.first.base(), range.second.base());
}
inline dmstr str_base::substr(const_iterator itBegin) const {
   validate_pointer(itBegin.base());
   return dmstr(itBegin.base(), chars_end());
}
inline dmstr str_base::substr(const_iterator itBegin, const_iterator itEnd) const {
   validate_pointer(itBegin.base());
   validate_pointer(itEnd.base());
   return dmstr(itBegin.base(), itEnd.base());
}


inline istr::istr(dmstr && s) :
   str_base(0) {
   assign_move(std::move(s));
}


inline istr & istr::operator=(dmstr && s) {
   assign_move(std::move(s));
   return *this;
}


inline mstr & mstr::operator=(dmstr && s) {
   assign_move(std::move(s));
   return *this;
}

} //namespace abc


/** Concatenation operator.

sL
   Left string operand.
sR
   Right string operand.
chL
   Left character operand.
chR
   Right character operand.
return
   Resulting string.
*/
inline abc::dmstr operator+(abc::istr const & sL, abc::istr const & sR) {
   return abc::dmstr(sL.chars_begin(), sL.chars_end(), sR.chars_begin(), sR.chars_end());
}
// Overloads taking a character literal.
inline abc::dmstr operator+(abc::istr const & sL, char32_t chR) {
   abc::char_t achR[abc::text::host_char_traits::max_codepoint_length];
   return abc::dmstr(
      sL.chars_begin(), sL.chars_end(),
      achR, abc::text::host_char_traits::codepoint_to_chars(chR, achR)
   );
}
inline abc::dmstr operator+(char32_t chL, abc::istr const & sR) {
   abc::char_t achL[abc::text::host_char_traits::max_codepoint_length];
   return abc::dmstr(
      achL, abc::text::host_char_traits::codepoint_to_chars(chL, achL),
      sR.chars_begin(), sR.chars_end()
   );
}
// Overloads taking a temporary string as left operand; they can avoid creating an intermediate
// string.
inline abc::dmstr operator+(abc::istr && sL, char32_t chR) {
   abc::dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
inline abc::dmstr operator+(abc::istr && sL, abc::istr const & sR) {
   abc::dmstr dmsL(std::move(sL));
   dmsL += sR;
   return std::move(dmsL);
}
inline abc::dmstr operator+(abc::mstr && sL, char32_t chR) {
   abc::dmstr dmsL(std::move(sL));
   dmsL += chR;
   return std::move(dmsL);
}
inline abc::dmstr operator+(abc::mstr && sL, abc::istr const & sR) {
   abc::dmstr dmsL(std::move(sL));
   dmsL += sR;
   return std::move(dmsL);
}


namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::dmstr> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::smstr


namespace abc {

/** mstr-derived class, good for clients that need in-place manipulation of strings that are most
likely to be shorter than a known small size.
*/
template <size_t t_cchEmbeddedCapacity>
class smstr :
   public mstr,
   private _raw_vextr_prefixed_item_array<char_t, t_cchEmbeddedCapacity> {

   using _raw_vextr_prefixed_item_array<char_t, t_cchEmbeddedCapacity>::smc_cbEmbeddedCapacity;

public:

   /** Constructor.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   */
   smstr() :
      mstr(smc_cbEmbeddedCapacity) {
   }
   smstr(smstr const & s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // If the source is using its embedded character array, it will be copied without allocating a
   // dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr(smstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(istr const & s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(s.chars_begin(), s.chars_end());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr(istr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   // This also covers smstr of different template arguments.
   smstr(mstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(dmstr && s) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_move(std::move(s));
   }
   template <size_t t_cch>
   smstr(char_t const (& ach)[t_cch]) :
      mstr(smc_cbEmbeddedCapacity) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
   }


   /** Assignment operator.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   return
      *this.
   */
   smstr & operator=(smstr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // If the source is using its embedded character array, it will be copied without allocating a
   // dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr & operator=(smstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(istr const & s) {
      assign_copy(s.chars_begin(), s.chars_end());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   // This also covers smstr of different template arguments.
   smstr & operator=(mstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(dmstr && s) {
      assign_move(std::move(s));
      return *this;
   }
   template <size_t t_cch>
   smstr & operator=(char_t const (& ach)[t_cch]) {
      assign_copy(ach, ach + t_cch - (ach[t_cch - 1 /*NUL*/] == '\0'));
      return *this;
   }
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <size_t t_cchEmbeddedCapacity>
struct hash<abc::smstr<t_cchEmbeddedCapacity>> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


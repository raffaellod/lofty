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
   typedef pointer_iterator<str_base, char_t> iterator;
   typedef pointer_iterator<str_base, char_t const> const_iterator;
   typedef std::reverse_iterator<iterator> reverse_iterator;
   typedef std::reverse_iterator<const_iterator> const_reverse_iterator;


public:

   /** Pointer to a C-style, NUL-terminated character array that may or may not share memory with
   an abc::*str instance. */
   typedef std::unique_ptr<
      char_t const [],
      memory::conditional_deleter<char_t const [], memory::freeing_deleter<char_t const []>>
   > c_str_pointer;
   /** String traits. */
   typedef text::utf_traits<char_t> traits;


public:

   /** Allows automatic cross-class-hierarchy casts.

   return
      Const reference to *this as an immutable string.
   */
   operator istr const &() const;


   /** Character access operator.

   i
      Character index. See abc::str_base::translate_index() for allowed index values.
      Character at index i.
   */
   char_t operator[](intptr_t i) const {
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
      return const_iterator(_raw_vextr_impl_base::begin<char_t>());
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
      return const_iterator(_raw_vextr_impl_base::begin<char_t>());
   }


   /** Returns a const forward iterator set beyond the last element.

   return
      Forward iterator to beyond the last element.
   */
   const_iterator cend() const {
      return const_iterator(_raw_vextr_impl_base::end<char_t>());
   }


   /** Support for relational operators.

   s
      String to compare to.
   return
      Standard comparison result integer:
      •  > 0 if *this > argument;
      •    0 if *this == argument;
      •  < 0 if *this < argument.
   */
   int compare_to(istr const & s) const;


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
      return const_iterator(_raw_vextr_impl_base::end<char_t>());
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


   /** Returns the count of characters in the string.

   return
      Count of characters.
   */
   size_t size() const {
      return _raw_trivial_vextr_impl::size<char_t>();
   }


   /** Returns the count of code points in the string.

   return
      Count of code points.
   */
   size_t size_cp() const {
      return traits::cp_len(cbegin().base(), cend().base());
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
   itBegin
      Iterator to the first character of the substring.
   ichEnd
      Index of the last character of the substring, exclusive. See abc::str_base::translate_range()
      for allowed end index values.
   itEnd
      Iterator to past the end of the substring.
   return
      Substring of *this.
   */
   dmstr substr(intptr_t ichBegin) const;
   dmstr substr(intptr_t ichBegin, intptr_t ichEnd) const;
   dmstr substr(intptr_t ichBegin, const_iterator itEnd) const;
   dmstr substr(const_iterator itBegin) const;
   dmstr substr(const_iterator itBegin, intptr_t ichEnd) const;
   dmstr substr(const_iterator itBegin, const_iterator itEnd) const;


protected:

   /** Constructor.

   cbStaticCapacity
      Size of the static character array, in bytes, or 0 if no static character array is present.
   pchConstSrc
      Pointer to a string that will be adopted by the str_base as read-only.
   cchSrc
      Count of characters in the string pointed to by pchConstSrc.
   bNulT
      true if the array pointed to by pchConstSrc is a NUL-terminated string, or false otherwise.
   */
   str_base(size_t cbStaticCapacity) :
      _raw_trivial_vextr_impl(cbStaticCapacity) {
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


   /** See _raw_trivial_vextr_impl::assign_share_ro_or_copy().

   s
      Source string.
   */
   void assign_share_ro_or_copy(str_base const & s) {
      _raw_trivial_vextr_impl::assign_share_ro_or_copy(s);
   }


   /** Converts a possibly negative character index into a pointer into the character array,
   throwing an exception if the result is out of bounds for the character array.

   i
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the character array by adding this->size() to it.
   return
      Pointer to the character.
   */
   char_t const * translate_index(intptr_t ich) const {
      return static_cast<char_t const *>(
         _raw_trivial_vextr_impl::translate_offset(ptrdiff_t(sizeof(char_t)) * ich)
      );
   }


   /** Converts a left-closed, right-open interval with possibly negative character indices into one
   consisting of two pointers into the item array.

   ichBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the character array by adding
      this->size() to it.
   ichEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the character array by
      adding this->size() to it.
   return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [0, 0) if the indices represent an empty interval after being adjusted.
   */
   std::pair<char_t const *, char_t const *> translate_range(
      intptr_t ichBegin, intptr_t ichEnd
   ) const {
      auto range(_raw_trivial_vextr_impl::translate_byte_range(
         ptrdiff_t(sizeof(char_t)) * ichBegin, ptrdiff_t(sizeof(char_t)) * ichEnd
      ));
      return std::make_pair(
         static_cast<char_t const *>(range.first), static_cast<char_t const *>(range.second)
      );
   }


public:

   // Lower-level helpers used internally by several methods.


   /** Returns a pointer to the first occurrence of a character in a string, or pchHaystackEnd if no
   matches are found. For the non-char32_t needle overload, the needle is a pointer because a code
   point can require more than one non-UTF-32 character to be encoded.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   pchNeedle
      Pointer to the encoded code point (UTF character sequence) to search for; its length is
      deduced automatically.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char_t const * _str_chr(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );
   static char_t const * _str_chr(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char_t const * pchNeedle
   );


   /** Returns a pointer to the last occurrence of a character in a string, or pchHaystackBegin if
   no matches are found.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   chNeedle
      Code point to search for.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the code point
      to search for, or nullptr if no matches are found.
   */
   static char_t const * _str_chr_r(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd, char32_t chNeedle
   );


   /** Returns the character index of the first occurrence of a string into another.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the first match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char_t const * _str_str(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
   );


   /** Returns the character index of the last occurrence of a string into another.

   pchHaystackBegin
      Pointer to the first character of the string to be searched.
   pchHaystackEnd
      Pointer to beyond the last character of the string to be searched.
   pchNeedleBegin
      Pointer to the first character of the string to search for.
   pchNeedleEnd
      Pointer to beyond the last character of the string to search for.
   return
      Pointer to the beginning of the last match, in the string to be searched, of the string to
      search for, or nullptr if no matches are found.
   */
   static char_t const * _str_str_r(
      char_t const * pchHaystackBegin, char_t const * pchHaystackEnd,
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd
   );


   /** Builds a failure restart table for searches using the Knuth-Morris-Pratt algorithm. See
   [DOC:1502 KMP substring search] for how this is built and used.

   pchNeedleBegin
      Pointer to the beginning of the search string.
   pchNeedleEnd
      Pointer beyond the end of the search string.
   pvcchFailNext
      Pointer to a vector that will receive the failure restart indices.
   */
   static void _str_str_build_failure_restart_table(
      char_t const * pchNeedleBegin, char_t const * pchNeedleEnd, mvector<size_t> * pvcchFailNext
   );


protected:

   /** Single NUL terminator. */
   static char_t const smc_chNUL;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   inline bool operator op(abc::str_base const & s1, abc::str_base const & s2) { \
      return s1.compare_to(static_cast<abc::istr const &>(s2)) op 0; \
   } \
   template <size_t t_cch> \
   inline bool operator op(abc::str_base const & s, abc::char_t const (& ach)[t_cch]) { \
      return s.compare_to(ach) op 0; \
   } \
   template <size_t t_cch> \
   inline bool operator op(abc::char_t const (& ach)[t_cch], abc::str_base const & s) { \
      return -s.compare_to(ach) op 0; \
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

   TODO: comment signature.
   */
   istr() :
      str_base(0) {
   }
   istr(istr const & s) :
      str_base(0) {
      assign_share_ro_or_copy(s);
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
      str_base(psz, traits::str_len(psz), true) {
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
      assign_share_ro_or_copy(s);
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


inline int str_base::compare_to(istr const & s) const {
   return traits::str_cmp(cbegin().base(), cend().base(), s.cbegin().base(), s.cend().base());
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
      assign_copy(s.cbegin().base(), s.cend().base());
      return *this;
   }
   mstr & operator=(istr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
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
      char_t ach[traits::max_codepoint_length];
      append(ach, traits::from_utf32(ch, ach));
      return *this;
   }
   mstr & operator+=(istr const & s) {
      append(s.cbegin().base(), s.size());
      return *this;
   }


   /** See str_base::operator[]().
   */
   char_t & operator[](intptr_t i) {
      return *const_cast<char_t *>(translate_index(i));
   }
   char_t operator[](intptr_t i) const {
      return str_base::operator[](i);
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
      return iterator(_raw_vextr_impl_base::begin<char_t>());
   }
   const_iterator begin() const {
      return str_base::begin();
   }


   /** See str_base::end(). Here also available in non-const overload.
   */
   iterator end() {
      return iterator(_raw_vextr_impl_base::end<char_t>());
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

   TODO: comment signature.
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
         cchRet = fnRead(begin().base(), cchMax);
      } while (cchRet >= cchMax);
      // Finalize the length.
      set_size(cchRet);
   }


   /** See str_base::rbegin(). Here also available in non-const overload.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(_raw_vextr_impl_base::end<char_t>()));
   }
   const_reverse_iterator rbegin() const {
      return str_base::rbegin();
   }


   /** See str_base::rend(). Here also available in non-const overload.
   */
   reverse_iterator rend() {
      return reverse_iterator(iterator(_raw_vextr_impl_base::begin<char_t>()));
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
   */
   void set_size(size_t cch) {
      _raw_trivial_vextr_impl::set_size(sizeof(char_t) * cch);
   }


protected:

   /** Constructor. See str_base::str_base().
   */
   mstr(size_t cbStaticCapacity) :
      str_base(cbStaticCapacity) {
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

   TODO: comment signature.
   */
   dmstr() :
      mstr(0) {
   }
   dmstr(dmstr const & s) :
      mstr(0) {
      assign_copy(s.cbegin().base(), s.cend().base());
   }
   dmstr(dmstr && s) :
      mstr(0) {
      assign_move(std::move(s));
   }
   dmstr(istr const & s) :
      mstr(0) {
      assign_copy(s.cbegin().base(), s.cend().base());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(istr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   dmstr(mstr const & s) :
      mstr(0) {
      assign_copy(s.cbegin().base(), s.cend().base());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(mstr && s) :
      mstr(0) {
      assign_move_dynamic_or_move_items(std::move(s));
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
   return
      *this.
   */
   dmstr & operator=(dmstr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
      return *this;
   }
   dmstr & operator=(dmstr && s) {
      assign_move(std::move(s));
      return *this;
   }
   dmstr & operator=(istr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(istr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   dmstr & operator=(mstr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(mstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
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
   return substr(ichBegin, intptr_t(size()));
}
inline dmstr str_base::substr(intptr_t ichBegin, intptr_t ichEnd) const {
   auto range(translate_range(ichBegin, ichEnd));
   return dmstr(range.first, range.second);
}
inline dmstr str_base::substr(intptr_t ichBegin, const_iterator itEnd) const {
   auto range(translate_range(ichBegin, itEnd - cbegin()));
   return dmstr(range.first, range.second);
}
inline dmstr str_base::substr(const_iterator itBegin) const {
   return substr(itBegin, cend());
}
inline dmstr str_base::substr(const_iterator itBegin, intptr_t ichEnd) const {
   auto range(translate_range(itBegin - cbegin(), ichEnd));
   return dmstr(range.first, range.second);
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
   return abc::dmstr(sL.cbegin().base(), sL.cend().base(), sR.cbegin().base(), sR.cend().base());
}
// Overloads taking a character literal.
inline abc::dmstr operator+(abc::istr const & sL, char32_t chR) {
   abc::char_t achR[abc::istr::traits::max_codepoint_length];
   return abc::dmstr(
      sL.cbegin().base(), sL.cend().base(), achR, achR + abc::istr::traits::from_utf32(chR, achR)
   );
}
inline abc::dmstr operator+(char32_t chL, abc::istr const & sR) {
   abc::char_t achL[abc::istr::traits::max_codepoint_length];
   return abc::dmstr(
      achL, achL + abc::istr::traits::from_utf32(chL, achL), sR.cbegin().base(), sR.cend().base()
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
template <size_t t_cchStaticCapacity>
class smstr :
   public mstr {
private:

   /** Actual static character array size, in bytes. */
   static size_t const smc_cbStaticCapacity = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_ARRAY_SIZE(
      sizeof(char_t) * t_cchStaticCapacity
   );


public:

   /** Constructor.

   TODO: comment signature.
   */
   smstr() :
      mstr(smc_cbStaticCapacity) {
   }
   smstr(smstr const & s) :
      mstr(smc_cbStaticCapacity) {
      assign_copy(s.cbegin().base(), s.cend().base());
   }
   // If the source is using its static character array, it will be copied without allocating a
   // dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr(smstr && s) :
      mstr(smc_cbStaticCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(istr const & s) :
      mstr(smc_cbStaticCapacity) {
      assign_copy(s.cbegin().base(), s.cend().base());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr(istr && s) :
      mstr(smc_cbStaticCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   // This also covers smstr of different template arguments.
   smstr(mstr && s) :
      mstr(smc_cbStaticCapacity) {
      assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(dmstr && s) :
      mstr(smc_cbStaticCapacity) {
      assign_move(std::move(s));
   }


   /** Assignment operator.

   s
      Source string.
   return
      *this.
   */
   smstr & operator=(smstr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
      return *this;
   }
   // If the source is using its static character array, it will be copied without allocating a
   // dynamic one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr & operator=(smstr && s) {
      assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(istr const & s) {
      assign_copy(s.cbegin().base(), s.cend().base());
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


private:

   // This section must match exactly _raw_vextr_impl_base_with_static_item_array.

   /** See _raw_vextr_impl_base_with_static_item_array::m_cbStaticCapacity. */
   size_t m_cbStaticCapacity;
   /** See _raw_vextr_impl_base_with_static_item_array::m_at. */
   std::max_align_t m_at[ABC_ALIGNED_SIZE(smc_cbStaticCapacity)];
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <size_t t_cchStaticCapacity>
struct hash<abc::smstr<t_cchStaticCapacity>> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


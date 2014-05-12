/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif

#include <abc/_vextr.hxx>
#include <abc/utf_traits.hxx>
#include <functional>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_str


namespace abc {

/** Template-independent methods of str.
*/
class ABCAPI _raw_str :
   public _raw_trivial_vextr_impl {
public:

   /** Pointer to a C-style, NUL-terminated character array that may or may not share memory with
   an abc::*str instance. */
   typedef std::unique_ptr<
      void const,
      memory::conditional_deleter<void const, memory::freeing_deleter<void const>>
   > c_str_pointer;


public:

   /** Returns a pointer to a NUL-terminated version of the string. See abc::str_base::c_str().

   cbItem
      Size of a single array item, in bytes.
   return
      NUL-terminated version of the string.
   */
   c_str_pointer c_str(size_t cbItem) const;


   /** Computes the hash value of the string.

   cbItem
      Size of a single array item, in bytes.
   return
      Hash value of the string.
   */
   size_t hash(size_t cbItem) const;


   /** Changes the length of the string, without changing its capacity.

   cbItem
      Size of a single array item, in bytes.
   cch
      New length of the string.
   */
   void set_size(size_t cbItem, size_t cch);


protected:

   /** Constructor.

   cchStaticMax
      Count of slots in the static character array, or 0 if no static character array is present.
   pchConstSrc
      Pointer to a string that will be adopted by the _raw_str as read-only.
   cchSrc
      Count of characters in the string pointed to by pchConstSrc.
   */
   _raw_str(size_t cchStaticMax) :
      _raw_trivial_vextr_impl(cchStaticMax) {
   }
   _raw_str(void const * pchConstSrc, size_t cchSrc) :
      _raw_trivial_vextr_impl(pchConstSrc, cchSrc, true) {
   }
};

} //namespace abc


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
class str_base :
   protected _raw_str,
   public _iterable_vector<str_base, char_t>,
   public support_explicit_operator_bool<str_base> {
protected:

   /** Shortcut for the base class providing iterator-based types and methods. */
   typedef _iterable_vector<str_base, char_t> itvec;


public:

   /** String traits. */
   typedef text::utf_traits<char_t> traits;
   /** Pointer to a C-style, NUL-terminated character array that may or may not share memory with
   an abc::*str instance. */
   typedef std::unique_ptr<
      char_t const [],
      memory::conditional_deleter<char_t const [], memory::freeing_deleter<char_t const []>>
   > c_str_pointer;
   /** See _iterable_vector::const_iterator. */
   typedef typename itvec::const_iterator const_iterator;


public:

   /** Allows automatic cross-class-hierarchy casts.

   return
      Const reference to *this as an immutable string.
   */
   operator istr const &() const;


   /** Character access operator.

   i
      Character index. See abc::_vextr::adjust_and_validate_index() for allowed index values.
   return
      Character at index i.
   */
   char_t operator[](intptr_t i) const {
      return data()[this->adjust_and_validate_index(i)];
   }


   /** Returns true if the length is greater than 0.

   return
      true if the string is not empty, or false otherwise.
   */
   explicit_operator_bool() const {
      return size() > 0;
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
   c_str_pointer c_str() const {
      auto psz(_raw_str::c_str(sizeof(char_t)));
      // Unpack the std::unique_ptr<void const *, …> into an std::unique_ptr<char_t const *, …>.
      return c_str_pointer(
         static_cast<char_t const *>(psz.release()),
         memory::conditional_deleter<char_t const [], memory::freeing_deleter<char_t const []>>(
            psz.get_deleter().enabled()
         )
      );
   }


   /** Returns the current size of the string buffer, in characters.

   return
      Size of the string buffer, in characters.
   */
   size_t capacity() const {
      return _raw_str::capacity();
   }


   /** Support for relational operators.

   s
      String to compare to.
   ach
      String literal to compare to.
   psz
      Pointer to a NUL-terminated string to compare to.
   return
      Standard comparison result integer:
      •  > 0 if *this > argument;
      •    0 if *this == argument;
      •  < 0 if *this < argument.
   */
   int compare_to(istr const & s) const;
   template <size_t t_cch>
   int compare_to(char_t const (& ach)[t_cch]) const {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      return traits::str_cmp(data(), size(), ach, t_cch - 1 /*NUL*/);
   }
   // This overload needs to be template, or it will take precedence over the one above.
   template <typename>
   int compare_to(char_t const * psz) const {
      return traits::str_cmp(data(), size(), psz, traits::str_len(psz));
   }


   /** Returns a read-only pointer to the character array.

   return
      Pointer to the character array.
   */
   char_t const * data() const {
      return _raw_str::data<char_t>();
   }


   /** Returns true if the string ends with a specified suffix.

   s
      String that *this should end with.
   ach
      String literal that *this should end with.
   psz
      NUL-terminated string that *this should end with.
   return
      true if *this ends with the specified suffix, or false otherwise.
   */
   bool ends_with(istr const & s) const;
   template <size_t t_cch>
   bool ends_with(char_t const (& ach)[t_cch]) const {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      size_t cchEnd(t_cch - 1 /*NUL*/);
      intptr_t cchRest(intptr_t(size()) - intptr_t(cchEnd));
      return cchRest >= 0 && traits::str_cmp(data() + cchRest, cchEnd, ach, cchEnd) == 0;
   }
   // This overload needs to be template, or it will take precedence over the one above.
   template <typename>
   bool ends_with(char_t const * psz) const {
      size_t cchEnd(traits::str_len(psz));
      intptr_t cchRest(intptr_t(size()) - intptr_t(cchEnd));
      return cchRest >= 0 && traits::str_cmp(data() + cchRest, cchEnd, psz, cchEnd) == 0;
   }


   /** Searches for and returns the first occurrence of the specified character or substring.

   chNeedle
      Character to search for.
   sNeedle
      String to search for.
   itWhence
      Iterator to the first character whence the search should start. When not specified, it
      defaults to this->cbegin().
   return
      Iterator to the first occurrence of the character/string, or this->cend() when no matches are
      found.
   */
   const_iterator find(char32_t chNeedle) const {
      return find(chNeedle, itvec::cbegin());
   }
   const_iterator find(char32_t chNeedle, const_iterator itWhence) const {
      auto itEnd(itvec::cend());
      char_t const * pch(traits::str_chr(itWhence.base(), itEnd.base(), chNeedle));
      return pch ? const_iterator(pch) : itEnd;
   }
   const_iterator find(istr const & sNeedle) const {
      return find(sNeedle, itvec::cbegin());
   }
   const_iterator find(istr const & sNeedle, const_iterator itWhence) const;


   /** Searches for and returns the last occurrence of the specified character or substring.

   chNeedle
      Character to search for.
   sNeedle
      String to search for.
   itWhence
      Iterator to the last character whence the search should start. When not specified, it
      defaults to this->cend().
   return
      Iterator to the first occurrence of the character/string, or this->cend() when no matches are
      found.
   */
   const_iterator find_last(char32_t chNeedle) const {
      return find_last(chNeedle, itvec::cend());
   }
   const_iterator find_last(char32_t chNeedle, const_iterator itWhence) const {
      char_t const * pch(traits::str_chr_r(itvec::cbegin().base(), itWhence.base(), chNeedle));
      return pch ? const_iterator(pch) : itvec::cend();
   }
   const_iterator find_last(istr const & sNeedle) const {
      return find_last(sNeedle, itvec::cend());
   }
   const_iterator find_last(istr const & sNeedle, const_iterator itWhence) const;


   /** Uses the current contents of the string to generate a new one using io::str_ostream::print().

   Implemented in str_iostream.hxx due to its dependency on io::str_iostream.

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


   /** Work around the protected inheritance, forcing the raw access to be explicit.

   return
      Reference to *this as a _raw_str instance.
   */
   _raw_str & raw() {
      return *this;
   }
   _raw_str const & raw() const {
      return *this;
   }


   /** Returns the count of characters in the string.

   return
      Count of characters.
   */
   size_t size() const {
      return _raw_str::size();
   }


   /** Returns the count of code points in the string.

   return
      Count of code points.
   */
   size_t size_cp() const {
      char_t const * pchBegin(data());
      return traits::cp_len(pchBegin, pchBegin + size());
   }


   /** Returns true if the string starts with a specified prefix.

   s
      String that *this should start with.
   ach
      String literal that *this should start with.
   psz
      NUL-terminated string that *this should start with.
   return
      true if *this starts with the specified suffix, or false otherwise.
   */
   bool starts_with(istr const & s) const;
   template <size_t t_cch>
   bool starts_with(char_t const (& ach)[t_cch]) const {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      size_t cchStart(t_cch - 1 /*NUL*/);
      return size() >= cchStart && traits::str_cmp(data(), cchStart, ach, cchStart) == 0;
   }
   // This overload needs to be template, or it will take precedence over the one above.
   template <typename>
   bool starts_with(char_t const * psz) const {
      size_t cchStart(traits::str_len(psz));
      return size() >= cchStart && traits::str_cmp(data(), cchStart, psz, cchStart) == 0;
   }


   /** Returns a portion of the string.

   ichBegin
      Index of the first character of the substring. See abc::_vextr::adjust_and_validate_range()
      for allowed begin index values.
   itBegin
      Iterator to the first character of the substring.
   ichEnd
      Index of the last character of the substring, exclusive. See
      abc::_vextr::adjust_and_validate_range() for allowed end index values.
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

   cchStatic
      Count of slots in the static character array, or 0 if no static character array is present.
   pch
      Pointer to a string that will be adopted by the str_base as read-only.
   cch
      Count of characters in the string pointed to by pch.
   */
   str_base(size_t cchStatic) :
      _raw_str(cchStatic) {
   }
   str_base(char_t const * pch, size_t cch) :
      _raw_str(pch, cch) {
   }


   /** See _raw_str::assign_copy().

   TODO: comment signature.
   */
   void assign_copy(char_t const * pch, size_t cch) {
      _raw_str::assign_copy(sizeof(char_t), pch, cch);
   }


   /** TODO: comment.
   */
   void assign_concat(char_t const * pch1, size_t cch1, char_t const * pch2, size_t cch2) {
      _raw_str::assign_concat(sizeof(char_t), pch1, cch1, pch2, cch2);
   }


   /** See _raw_str::assign_move().

   s
      Source string.
   */
   void assign_move(str_base && s) {
      _raw_str::assign_move(static_cast<_raw_str &&>(s));
   }


   /** See _raw_str::assign_move_dynamic_or_move_items().

   s
      Source string.
   */
   void assign_move_dynamic_or_move_items(str_base && s) {
      _raw_str::assign_move_dynamic_or_move_items(sizeof(char_t), static_cast<_raw_str &&>(s));
   }


   /** See _raw_str::assign_share_ro_or_copy().

   s
      Source string.
   */
   void assign_share_ro_or_copy(str_base const & s) {
      _raw_str::assign_share_ro_or_copy(sizeof(char_t), s);
   }
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
struct hash<abc::str_base> {

   size_t operator()(abc::str_base const & s) const {
      return s.raw().hash(sizeof(abc::char_t));
   }
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
      this->assign_share_ro_or_copy(s);
   }
   istr(istr && s) :
      str_base(0) {
      // Non-const, so it can’t be anything but a real istr, so it owns its item array.
      this->assign_move(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the istr && overload.
   istr(mstr && s);
   istr(dmstr && s);
   template <size_t t_cch>
   istr(char_t const (& ach)[t_cch]) :
      str_base(ach, t_cch - 1 /*NUL*/) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
   }
   istr(char_t const * psz, size_t cch) :
      str_base(0) {
      this->assign_copy(psz, cch);
   }
   istr(unsafe_t, char_t const * psz) :
      str_base(psz, traits::str_len(psz)) {
   }
   istr(unsafe_t, char_t const * psz, size_t cch) :
      str_base(psz, cch) {
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
      this->assign_share_ro_or_copy(s);
      return *this;
   }
   istr & operator=(istr && s) {
      // Non-const, so it can’t be anything but a real istr, so it owns its item array.
      this->assign_move(std::move(s));
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


   /** Automatic conversion to char_range.

   return
      Contents of *this.
   */
   operator char_range_<char_t>() const {
      return char_range_<char_t>(str_base::cbegin().base(), str_base::cend().base());
   }
};


// Now these can be implemented.

inline str_base::operator istr const &() const {
   return *static_cast<istr const *>(this);
}


inline int str_base::compare_to(istr const & s) const {
   return traits::str_cmp(data(), size(), s.data(), s.size());
}


inline bool str_base::ends_with(istr const & s) const {
   size_t cchEnd(s.size());
   intptr_t cchRest(intptr_t(size()) - intptr_t(cchEnd));
   return cchRest >= 0 && traits::str_cmp(data() + cchRest, cchEnd, s.data(), cchEnd) == 0;
}


inline str_base::const_iterator str_base::find(
   istr const & sNeedle, const_iterator itWhence
) const {
   auto itEnd(itvec::cend());
   char_t const * pch(traits::str_str(
      itWhence.base(), itEnd.base(), sNeedle.cbegin().base(), sNeedle.cend().base()
   ));
   return pch ? const_iterator(pch) : itEnd;
}


inline str_base::const_iterator str_base::find_last(
   istr const & sNeedle, const_iterator itWhence
) const {
   char_t const * pch(traits::str_str_r(
      itvec::cbegin().base(), itWhence.base(), sNeedle.cbegin().base(), sNeedle.cend().base()
   ));
   return pch ? const_iterator(pch) : itvec::cend();
}


inline bool str_base::starts_with(istr const & s) const {
   size_t cchStart(s.size());
   return size() >= cchStart && traits::str_cmp(data(), cchStart, s.data(), cchStart) == 0;
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
class mstr :
   public str_base {
public:

   /** Assignment operator.

   s
      Source string.
   ach
      Source NUL-terminated string literal.
   return
      *this.
   */
   mstr & operator=(mstr const & s) {
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   mstr & operator=(istr const & s) {
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the mstr && overload.
   mstr & operator=(istr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   mstr & operator=(dmstr && s);
   template <size_t t_cch>
   mstr & operator=(char_t const (& ach)[t_cch]) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      this->assign_copy(ach, t_cch - 1 /*NUL*/);
      return *this;
   }


   /** Concatenation-assignment operator.

   TODO: comment signature.
   */
   mstr & operator+=(char_t ch) {
      append(&ch, 1);
      return *this;
   }
   template <size_t t_cch>
   mstr & operator+=(char_t const (& ach)[t_cch]) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      append(ach, t_cch - 1 /*NUL*/);
      return *this;
   }
   mstr & operator+=(istr const & s) {
      append(s.data(), s.size());
      return *this;
   }


   /** See str_base::operator[]().
   */
   char_t & operator[](intptr_t i) {
      return data()[this->adjust_and_validate_index(i)];
   }
   char_t operator[](intptr_t i) const {
      return str_base::operator[](i);
   }


   /** Same as operator+=(), but for multi-argument overloads.

   TODO: comment signature.
   */
   void append(char_t const * pchAdd, size_t cchAdd) {
      _raw_str::append(sizeof(char_t), pchAdd, cchAdd);
   }


   /** Returns a pointer to the character array.

   TODO: comment signature.
   */
   char_t * data() {
      return _raw_str::data<char_t>();
   }
   char_t const * data() const {
      return _raw_str::data<char_t>();
   }


   /** Grows the item array until the specified callback succeeds in filling it and returns a number
   of needed characters that’s less than the size of the buffer. For example, for cchMax == 3 (NUL
   terminator included), it must return <= 2 (NUL excluded).

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
      size_t cchRet, cchMax(rvib::smc_cMinSlots * rvib::smc_iGrowthRate);
      do {
         cchMax *= rvib::smc_iGrowthRate;
         set_capacity(cchMax, false);
         cchRet = fnRead(data(), cchMax);
      } while (cchRet >= cchMax);
      // Finalize the length.
      set_size(cchRet);
   }


   /** See _raw_str::set_capacity().

   TODO: comment signature.
   */
   void set_capacity(size_t cchMin, bool bPreserve) {
      _raw_str::set_capacity(sizeof(char_t), cchMin, bPreserve);
   }


   /** See _raw_str::set_size().

   TODO: comment signature.
   */
   void set_size(size_t cch) {
      _raw_str::set_size(sizeof(char_t), cch);
   }


protected:

   /** Constructor. See str_base::str_base().
   */
   mstr(size_t cchStatic) :
      str_base(cchStatic) {
   }
   mstr(char_t const * pch, size_t cch) :
      str_base(pch, cch) {
   }
};


// Now these can be implemented.

inline istr::istr(mstr && s) :
   str_base(0) {
   this->assign_move_dynamic_or_move_items(std::move(s));
}


inline istr & istr::operator=(mstr && s) {
   this->assign_move_dynamic_or_move_items(std::move(s));
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
      this->assign_copy(s.data(), s.size());
   }
   dmstr(dmstr && s) :
      mstr(0) {
      this->assign_move(std::move(s));
   }
   dmstr(istr const & s) :
      mstr(0) {
      this->assign_copy(s.data(), s.size());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(istr && s) :
      mstr(0) {
      this->assign_move_dynamic_or_move_items(std::move(s));
   }
   dmstr(mstr const & s) :
      mstr(0) {
      this->assign_copy(s.data(), s.size());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr(mstr && s) :
      mstr(0) {
      this->assign_move_dynamic_or_move_items(std::move(s));
   }
   template <size_t t_cch>
   explicit dmstr(char_t const (& ach)[t_cch]) :
      mstr(0) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      this->assign_copy(ach, t_cch - 1 /*NUL*/);
   }
   dmstr(char_t const * pch, size_t cch) :
      mstr(0) {
      this->assign_copy(pch, cch);
   }
   dmstr(char_t const * pch1, size_t cch1, char_t const * pch2, size_t cch2) :
      mstr(0) {
      this->assign_concat(pch1, cch1, pch2, cch2);
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
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   dmstr & operator=(dmstr && s) {
      this->assign_move(std::move(s));
      return *this;
   }
   dmstr & operator=(istr const & s) {
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(istr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   dmstr & operator=(mstr const & s) {
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the dmstr && overload.
   dmstr & operator=(mstr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   template <size_t t_cch>
   dmstr & operator=(char_t const (& ach)[t_cch]) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      this->assign_copy(ach, t_cch - 1 /*NUL*/);
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
   auto range(this->adjust_and_validate_range(ichBegin, ichEnd));
   return dmstr(data() + range.first, range.second - range.first);
}
inline dmstr str_base::substr(intptr_t ichBegin, const_iterator itEnd) const {
   auto range(this->adjust_and_validate_range(ichBegin, itEnd - itvec::cbegin()));
   return dmstr(data() + range.first, range.second - range.first);
}
inline dmstr str_base::substr(const_iterator itBegin) const {
   return substr(itBegin, itvec::cend());
}
inline dmstr str_base::substr(const_iterator itBegin, intptr_t ichEnd) const {
   auto range(this->adjust_and_validate_range(itBegin - itvec::cbegin(), ichEnd));
   return dmstr(data() + range.first, range.second - range.first);
}
inline dmstr str_base::substr(const_iterator itBegin, const_iterator itEnd) const {
   return dmstr(itBegin.base(), size_t(itEnd - itBegin));
}


inline istr::istr(dmstr && s) :
   str_base(0) {
   this->assign_move(std::move(s));
}


inline istr & istr::operator=(dmstr && s) {
   this->assign_move(std::move(s));
   return *this;
}


inline mstr & mstr::operator=(dmstr && s) {
   this->assign_move(std::move(s));
   return *this;
}

} //namespace abc


/** Concatenation operator.

TODO: comment signature.
*/
inline abc::dmstr operator+(abc::str_base const & s1, abc::str_base const & s2) {
   return abc::dmstr(s1.data(), s1.size(), s2.data(), s2.size());
}
// Overloads taking a character literal.
inline abc::dmstr operator+(abc::str_base const & s, abc::char_t ch) {
   return abc::dmstr(s.data(), s.size(), &ch, 1);
}
inline abc::dmstr operator+(abc::char_t ch, abc::str_base const & s) {
   return abc::dmstr(&ch, 1, s.data(), s.size());
}
// Overloads taking a string literal.
template <size_t t_cch>
inline abc::dmstr operator+(abc::str_base const & s, abc::char_t const (& ach)[t_cch]) {
   ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
   return abc::dmstr(s.data(), s.size(), ach, t_cch - 1 /*NUL*/);
}
template <size_t t_cch>
inline abc::dmstr operator+(abc::char_t const (& ach)[t_cch], abc::str_base const & s) {
   ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
   return abc::dmstr(ach, t_cch - 1 /*NUL*/, s.data(), s.size());
}
// Overloads taking a temporary dmstr as left operand; they can avoid creating an intermediate
// string.
// TODO: verify that compilers actually select these overloads whenever possible.
inline abc::dmstr operator+(abc::dmstr && s, abc::char_t ch) {
   s += ch;
   return std::move(s);
}
template <size_t t_cch>
inline abc::dmstr operator+(abc::dmstr && s, abc::char_t const (& ach)[t_cch]) {
   s += ach;
   return std::move(s);
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
template <size_t t_cchStatic>
class smstr :
   public mstr {
private:

   /** Actual static item array size. */
   static size_t const smc_cchFixed = _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(t_cchStatic);


public:

   /** Constructor.

   TODO: comment signature.
   */
   smstr() :
      mstr(smc_cchFixed) {
   }
   smstr(smstr const & s) :
      mstr(smc_cchFixed) {
      this->assign_copy(s.data(), s.size());
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr(smstr && s) :
      mstr(smc_cchFixed) {
      this->assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(istr const & s) :
      mstr(smc_cchFixed) {
      this->assign_copy(s.data(), s.size());
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr(istr && s) :
      mstr(smc_cchFixed) {
      this->assign_move_dynamic_or_move_items(std::move(s));
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   // This also covers smstr of different template arguments.
   smstr(mstr && s) :
      mstr(smc_cchFixed) {
      this->assign_move_dynamic_or_move_items(std::move(s));
   }
   smstr(dmstr && s) :
      mstr(smc_cchFixed) {
      this->assign_move(std::move(s));
   }
   template <size_t t_cch>
   explicit smstr(char_t const (& ach)[t_cch]) :
      mstr(smc_cchFixed) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      this->assign_copy(ach, t_cch - 1 /*NUL*/);
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
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   // If the source is using its static item array, it will be copied without allocating a dynamic
   // one; if the source is dynamic, it will be moved. Either way, this won’t throw.
   smstr & operator=(smstr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(istr const & s) {
      this->assign_copy(s.data(), s.size());
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   smstr & operator=(istr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   // This can throw exceptions, but it’s allowed to since it’s not the smstr && overload.
   // This also covers smstr of different template arguments.
   smstr & operator=(mstr && s) {
      this->assign_move_dynamic_or_move_items(std::move(s));
      return *this;
   }
   smstr & operator=(dmstr && s) {
      this->assign_move(std::move(s));
      return *this;
   }
   template <size_t t_cch>
   smstr & operator=(char_t const (& ach)[t_cch]) {
      ABC_ASSERT(ach[t_cch - 1 /*NUL*/] == '\0', SL("string literal must be NUL-terminated"));
      this->assign_copy(ach, t_cch - 1 /*NUL*/);
      return *this;
   }


private:

   // This section must match exactly _raw_vextr_impl_base_with_static_item_array.

   /** See _raw_vextr_impl_base_with_static_item_array::m_ciStaticMax. */
   size_t m_ciStaticMax;
   /** See _raw_vextr_impl_base_with_static_item_array::m_at. */
   std::max_align_t m_at[ABC_ALIGNED_SIZE(sizeof(char_t) * smc_cchFixed)];
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <size_t t_cchStatic>
struct hash<abc::smstr<t_cchStatic>> : public hash<abc::str_base> {};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////


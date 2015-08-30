/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace detail {

/*! Pointer to a C-style, NUL-terminated character array that may or may not share memory with an
abc::text::*str instance. */
class c_str_ptr {
private:
   //! Internal conditionally-deleting pointer type.
   typedef _std::unique_ptr<
      char_t const [], memory::conditional_deleter<char_t const [], memory::freeing_deleter>
   > pointer;

public:
   /*! Constructor.

   @param pch
      Pointer to the character array.
   @param bOwn
      If true, the pointer will own the character array; if false, it won’t try to deallocate it.
   */
   c_str_ptr(char_t const * pch, bool bOwn) :
      m_p(pch, pointer::deleter_type(bOwn)) {
   }

   /*! Move constructor.

   @param p
      Source object.
   */
   c_str_ptr(c_str_ptr && p) :
      m_p(_std::move(p.m_p)) {
   }

   /*! Move-assignment operator.

   @param p
      Source object.
   @return
      *this.
   */
   c_str_ptr & operator=(c_str_ptr && p) {
      m_p = _std::move(p.m_p);
      return *this;
   }

   /*! Implicit conversion to char_t const *.

   @return
      Pointer to the character array.
   */
   operator char_t const *() const {
      return m_p.get();
   }

   /*! Enables access to the internal pointer.

   @return
      Reference to the internal pointer.
   */
   pointer const & _get() const {
      return m_p;
   }

private:
   //! Conditionally-deleting pointer.
   pointer m_p;
};

}}} //namespace abc::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! See abc::external_buffer.
struct external_buffer_t {
   //! Constructor. Required to instantiate a const instance.
   external_buffer_t() {
   }
};

/*! Constant similar in use to std::nothrow; when specified as extra argument for abc::text::*str
constructors, it indicates that the string should use an external buffer that is guaranteed by the
caller to have a scope lifetime equal or longer than that of the string. */
extern ABACLADE_SYM external_buffer_t const external_buffer;

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text {

template <>
class ABACLADE_SYM sstr<0> :
   protected collections::detail::raw_trivial_vextr_impl,
   public support_explicit_operator_bool<str> {
private:
   typedef collections::detail::raw_trivial_vextr_impl vextr_impl;

public:
   //! Presents an abc::text::str character(s) as a char32_t const &.
   class const_codepoint_proxy {
   private:
      friend class sstr;

   public:
      /*! Implicit conversion to a code point.

      @return
         Code point that the proxy is currently referencing.
      */
      operator char32_t() const;

   private:
      /*! Constructor.

      @param ps
         Pointer to the containing string.
      @param ich
         Index of the character(s) that this proxy will present as char32_t.
      */
      const_codepoint_proxy(str const * ps, std::size_t ich) :
         mc_ps(ps),
         mc_ich(ich) {
      }

   protected:
      //! Pointer to the containing string.
      str const * const mc_ps;
      //! Index of the proxied character(s).
      std::size_t const mc_ich;
   };

   //! Presents an abc::text::str character(s) as a char32_t &.
   class codepoint_proxy : public const_codepoint_proxy {
   private:
      friend class sstr;

   public:
      /*! Assignment operator.

      @param ch
         Source character.
      @return
         *this.
      */
      codepoint_proxy & operator=(char_t ch);

   #if ABC_HOST_UTF > 8
      /*! Assignment operator that accepts ASCII characters.

      @param ch
         Source ASCII character.
      @return
         *this.
      */
      codepoint_proxy & operator=(char ch) {
         return operator=(host_char(ch));
      }
   #endif

      /*! Assignment operator that accepts a code point.

      @param cp
         Source code point.
      @return
         *this.
      */
      codepoint_proxy & operator=(char32_t cp);

      /*! Copy-assignment operator. This copies the char32_t value, not the internal data members;
      this allows writing expressions like *itDst = *itSrc to copy code points from one iterator to
      another.

      @param cpp
         Source object to copy a code point from.
      @return
         *this.
      */
      codepoint_proxy & operator=(codepoint_proxy const & cpp) {
         return operator=(cpp.operator char32_t());
      }

      /*! Copy-assignment operator. This copies the char32_t value, not the internal data members;
      this allows writing expressions like *itDst = *itSrc to copy code points from one iterator to
      another. This overload supports copying a code point from a const iterator to a non-const
      iterator.

      @param cpp
         Source object to copy a code point from.
      @return
         *this.
      */
      codepoint_proxy & operator=(const_codepoint_proxy const & cpp) {
         return operator=(cpp.operator char32_t());
      }

   private:
      /*! Constructor.

      @param ps
         Pointer to the containing string.
      @param ich
         Index of the character(s) that this proxy will present as char32_t.
      */
      codepoint_proxy(str * ps, std::size_t ich) :
         const_codepoint_proxy(ps, ich) {
      }
   };

   typedef char_t value_type;
   typedef char_t * pointer;
   typedef char_t const * const_pointer;
   typedef char_t & reference;
   typedef char_t const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;

   /*! Code point iterator that hides the underlying encoded representation, presenting a string as
   an array of code points (char32_t). Pointers/references are still char_t. */
   class ABACLADE_SYM const_iterator {
   private:
      friend class sstr;

   public:
      typedef char32_t value_type;
      typedef char_t const * pointer;
      typedef const_codepoint_proxy reference;
      typedef std::ptrdiff_t difference_type;
      typedef _std::random_access_iterator_tag iterator_category;

   public:
      //! Default constructor.
      /*constexpr*/ const_iterator() :
         m_ps(nullptr),
         m_ich(0) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current character.
      */
      const_codepoint_proxy operator*() const {
         return const_codepoint_proxy(m_ps, throw_if_end(m_ich));
      }

      /*! Element access operator.

      @param i
         Index relative to *this. If the resulting index is outside of the string’s [begin, end)
         range, an index_error exception will be thrown.
      @return
         Reference to the specified item.
      */
      const_codepoint_proxy operator[](std::ptrdiff_t i) const {
         return const_codepoint_proxy(m_ps, throw_if_end(advance(i, true)));
      }

      /*! Addition-assignment operator.

      @param i
         Count of positions by which to advance the iterator. If the resulting iterator is outside
         of the string’s [begin, end] range, an iterator_error exception will be thrown.
      @return
         *this after it’s moved forward by i positions.
      */
      const_iterator & operator+=(std::ptrdiff_t i) {
         m_ich = advance(i, false);
         return *this;
      }

      /*! Subtraction-assignment operator.

      @param i
         Count of positions by which to rewind the iterator. If the resulting iterator is outside of
         the string’s [begin, end] range, an iterator_error exception will be thrown.
      @return
         *this after it’s moved backwards by i positions.
      */
      const_iterator & operator-=(std::ptrdiff_t i) {
         m_ich = advance(-i, false);
         return *this;
      }

      /*! Addition operator.

      @param i
         Count of positions by which to advance the iterator. If the resulting iterator is outside
         of the string’s [begin, end] range, an iterator_error exception will be thrown.
      @return
         Iterator that’s i items ahead of *this.
      */
      const_iterator operator+(std::ptrdiff_t i) const {
         return const_iterator(m_ps, advance(i, false));
      }

      /*! Subtraction/difference operator.

      @param i
         Count of positions by which to rewind the iterator. If the resulting iterator is outside of
         the string’s [begin, end] range, an iterator_error exception will be thrown.
      @param it
         Iterator from which to calculate the distance.
      @return
         Iterator that’s i items behind *this (subtraction) or distance between *this and it, in
         code points (difference).
      */
      const_iterator operator-(std::ptrdiff_t i) const {
         return const_iterator(m_ps, advance(-i, false));
      }
      std::ptrdiff_t operator-(const_iterator it) const {
         return distance(it.m_ich);
      }

      /*! Preincrement operator. If the resulting iterator was already at the string’s end, an
      iterator_error exception will be thrown.

      @return
         *this after it’s moved to the value following the one currently pointed to.
      */
      const_iterator & operator++() {
         m_ich = advance(1, false);
         return *this;
      }

      /*! Postincrement operator. If the resulting iterator was already at the string’s end, an
      iterator_error exception will be thrown.

      @return
         Iterator pointing to the value following the one pointed to by this iterator.
      */
      const_iterator operator++(int) {
         std::size_t ich = m_ich;
         m_ich = advance(1, false);
         return const_iterator(m_ps, ich);
      }

      /*! Predecrement operator. If the resulting iterator was already at the string’s beginning, an
      iterator_error exception will be thrown.

      @return
         *this after it’s moved to the value preceding the one currently pointed to.
      */
      const_iterator & operator--() {
         m_ich = advance(-1, false);
         return *this;
      }

      /*! Postdecrement operator. If the resulting iterator was already at the string’s beginning,
      an iterator_error exception will be thrown.

      @return
         Iterator pointing to the value preceding the one pointed to by this iterator.
      */
      const_iterator operator--(int) {
         std::size_t ich = m_ich;
         m_ich = advance(-1, false);
         return const_iterator(m_ps, ich);
      }

   // Relational operators.
   #define ABC_RELOP_IMPL(op) \
      bool operator op(const_iterator const & it) const { \
         return base() op it.base(); \
      }
   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   ABC_RELOP_IMPL(>)
   ABC_RELOP_IMPL(>=)
   ABC_RELOP_IMPL(<)
   ABC_RELOP_IMPL(<=)
   #undef ABC_RELOP_IMPL

      /*! Returns the underlying iterator type.

      @return
         Pointer to the value pointed to by this iterator.
      */
      char_t const * base() const;

   protected:
      //! Invokes m_ps->advance_char_index(). See str::advance_char_index().
      std::size_t advance(std::ptrdiff_t iDelta, bool bIndex) const;

      /*! Computes the distance from another iterator/index.

      @param ich
         Index from which to calculate the distance.
      @return
         Distance between *this and ich, in code points.
      */
      std::ptrdiff_t distance(std::size_t ich) const;

      /*! Throws an iterator_error if the specified index is at or beyond the end of the string.

      @param ich
         Index to validate.
      @return
         ich.
      */
      std::size_t throw_if_end(std::size_t ich) const;

   protected:
      /*! Constructor.

      @param ps
         Pointer to the string that is creating the iterator.
      @param ich
         Character index to set the iterator to.
      */
      const_iterator(str const * ps, std::size_t ich) :
         m_ps(ps),
         m_ich(ich) {
      }

   protected:
      //! Pointer to the source string.
      str const * m_ps;
      //! Index of the current character.
      std::size_t m_ich;
   };

   /*! Character iterator that hides the underlying encoded representation, presenting a string as
   an array of code points (char32_t). Pointers/references are still char_t. */
   class iterator : public const_iterator {
   private:
      friend class sstr;

   public:
      typedef char_t * pointer;
      typedef codepoint_proxy reference;

   public:
      //! Default constructor.
      /*constexpr*/ iterator() {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current character.
      */
      codepoint_proxy operator*() const {
         return codepoint_proxy(const_cast<str *>(m_ps), throw_if_end(m_ich));
      }

      /*! Element access operator.

      @param i
         Index relative to *this. If the resulting index is outside of the string’s [begin, end)
         range, an index_error exception will be thrown.
      @return
         Reference to the specified item.
      */
      codepoint_proxy operator[](std::ptrdiff_t i) const {
         return codepoint_proxy(const_cast<str *>(m_ps), throw_if_end(advance(i, true)));
      }

      /*! Returns the underlying iterator type.

      @return
         Pointer to the value pointed to by this iterator.
      */
      char_t * base() const;

      //! See const_iterator::operator+=().
      iterator & operator+=(std::ptrdiff_t i) {
         return static_cast<iterator &>(const_iterator::operator+=(i));
      }

      //! See const_iterator::operator-=().
      iterator & operator-=(std::ptrdiff_t i) {
         return static_cast<iterator &>(const_iterator::operator-=(i));
      }

      //! See const_iterator::operator+();
      iterator operator+(std::ptrdiff_t i) const {
         return const_iterator::operator+(i);
      }

      //! See const_iterator::operator-();
      using const_iterator::operator-;
      iterator operator-(std::ptrdiff_t i) const {
         return const_iterator::operator-(i);
      }

      //! See const_iterator::operator++().
      iterator & operator++() {
         return static_cast<iterator &>(const_iterator::operator++());
      }

      //! See const_iterator::operator++(int).
      iterator operator++(int) {
         return const_iterator::operator++(0);
      }

      //! See const_iterator::operator--().
      iterator & operator--() {
         return static_cast<iterator &>(const_iterator::operator--());
      }

      //! See const_iterator::operator--(int).
      iterator operator--(int) {
         return const_iterator::operator--(0);
      }

   private:
      /*! Constructor.

      @param ps
         Pointer to the string that is creating the iterator.
      @param ich
         Character index to set the iterator to.
      */
      iterator(str * ps, std::size_t ich) :
         const_iterator(ps, ich) {
      }

      /*! Copy constructor. Allows to convert from non-const to const iterator types.

      @param it
         Source object.
      */
      iterator(const_iterator const & it) :
         const_iterator(it) {
      }
   };

   typedef _std::reverse_iterator<iterator> reverse_iterator;
   typedef _std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
   //! Empty string constant.
   static sstr const & empty;

public:
   //! Default constructor.
   sstr() :
      vextr_impl(0) {
   }

   /*! Move constructor.

   @param s
      Source object.
   */
   sstr(str && s) :
      vextr_impl(0) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(s));
   }

   /*! Copy constructor.

   @param s
      Source object.
   */
   sstr(str const & s) :
      vextr_impl(0) {
      vextr_impl::assign_share_raw_or_copy_desc(s);
   }

   /*! Constructor from string literals.

   @param ach
      Source NUL-terminated string literal.
   */
   template <std::size_t t_cch>
   sstr(char_t const (& ach)[t_cch]) :
      vextr_impl(0, &ach[0], &ach[ABC_SL_SIZE(ach)], ach[t_cch - 1 /*NUL*/] == '\0') {
   }

   /*! Constructor that copies the contents of a character buffer.

   @param pchBegin
      Pointer to the beginning of the source stirng.
   @param pchEnd
      Pointer to the end of the source stirng.
   */
   sstr(char_t const * pchBegin, char_t const * pchEnd) :
      vextr_impl(0) {
      vextr_impl::assign_copy(pchBegin, pchEnd);
   }

   /*! Constructor that creates a new string from two character buffers.

   @param pch1Begin
      Pointer to the beginning of the left source stirng to concatenate.
   @param pch1End
      Pointer to the end of the left source stirng.
   @param pch2Begin
      Pointer to the beginning of the right source stirng to concatenate.
   @param pch2End
      Pointer to the end of the right source stirng.
   */
   sstr(
      char_t const * pch1Begin, char_t const * pch1End,
      char_t const * pch2Begin, char_t const * pch2End
   ) :
      vextr_impl(0) {
      vextr_impl::assign_concat(pch1Begin, pch1End, pch2Begin, pch2End);
   }

   /*! Constructor that makes the string refer to the specified NUL-terminated raw C string.

   @param psz
      Pointer to the source NUL-terminated string literal.
   */
   sstr(external_buffer_t const &, char_t const * psz) :
      vextr_impl(0, psz, psz + text::size_in_chars(psz), true) {
   }

   /*! Constructor that will make the string refer to the specified raw C string.

   @param pch
      Pointer to the source string.
   @param cch
      Count of characters in the array pointed to be psz.
   */
   sstr(external_buffer_t const &, char_t const * pch, std::size_t cch) :
      vextr_impl(0, pch, pch + cch, false) {
   }

   /*! Move-assignment operator.

   @param s
      Source object.
   @return
      *this.
   */
   str & operator=(str && s) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(s));
      return *this;
   }

   /*! Copy-assignment operator.

   @param s
      Source object.
   @return
      *this.
   */
   str & operator=(str const & s) {
      vextr_impl::assign_share_raw_or_copy_desc(s);
      return *this;
   }

   /*! Assignment operator from string literal.

   @param ach
      Source NUL-terminated string literal.
   @return
      *this.
   */
   template <std::size_t t_cch>
   str & operator=(char_t const (& ach)[t_cch]) {
      str s(ach);
      operator=(_std::move(s));
      return *this;
   }

   /*! Character access operator.

   @param i
      Character index. If outside of the [begin, end) range, an  index_error exception will be
      thrown.
   @return
      Character at index i.
   */
   codepoint_proxy operator[](std::ptrdiff_t i) {
      return codepoint_proxy(this, advance_char_index(0, i, true));
   }

   /*! Const haracter access operator.

   @param i
      Character index. If outside of the [begin, end) range, an  index_error exception will be
      thrown.
   @return
      Character at index i.
   */
   const_codepoint_proxy operator[](std::ptrdiff_t i) const {
      return const_codepoint_proxy(this, advance_char_index(0, i, true));
   }

   /*! Boolean evaluation operator.

   @return
      true if the string is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      /* Use std::int8_t to avoid multiplying by sizeof(char_t) when all we need is a greater-than
      check. */
      return collections::detail::raw_vextr_impl_base::end<std::int8_t>() >
         collections::detail::raw_vextr_impl_base::begin<std::int8_t>();
   }

   /*! Concatenation-assignment operator.

   @param ch
      Character to append.
   @return
      *this.
   */
   str & operator+=(char_t ch) {
      append(&ch, 1);
      return *this;
   }

#if ABC_HOST_UTF > 8
   /*! Concatenation-assignment operator.

   @param ch
      ASCII character to append.
   @return
      *this.
   */
   str & operator+=(char ch) {
      return operator+=(host_char(ch));
   }
#endif

   /*! Concatenation-assignment operator.

   @param ch
      Code point to append.
   @return
      *this.
   */
   str & operator+=(char32_t cp) {
      char_t ach[host_char_traits::max_codepoint_length];
      append(ach, static_cast<std::size_t>(host_char_traits::codepoint_to_chars(cp, ach) - ach));
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param s
      String to append.
   @return
      *this.
   */
   str & operator+=(str const & s) {
      append(s.chars_begin(), s.size_in_chars());
      return *this;
   }

   /*! Same as operator+=(), but for multi-argument overloads.

   @param pchAdd
      Pointer to an array of characters to append.
   @param cchAdd
      Count of characters in the array pointed to by pchAdd.
   */
   void append(char_t const * pchAdd, std::size_t cchAdd) {
      vextr_impl::insert_remove(
         collections::detail::raw_vextr_impl_base::size<std::int8_t>(),
         pchAdd, sizeof(char_t) * cchAdd, 0
      );
   }

   /*! Returns an iterator set to the first character.

   @return
      Iterator to the first character.
   */
   iterator begin() {
      return iterator(this, 0);
   }

   /*! Returns a const iterator set to the first character.

   @return
      Const iterator to the first character.
   */
   const_iterator begin() const {
      return const_cast<str *>(this)->begin();
   }

   /*! Returns a pointer to the string, after ensuring that its character array is NUL-terminated.

   If the string does not include a NUL terminator, the character array will be expanded to include
   one. Either way, the returned pointer will refer to the same character array, and it will not own
   it and the pointed-to memory.

   @return
      Pointer to the NUL-terminated string. Only valid as long as *this is, and only until the next
      change to *this.
   */
   detail::c_str_ptr c_str();

   /*! Returns a pointer to a NUL-terminated version of the string.

   If the string already includes a NUL terminator, the returned pointer will refer to the same
   character array, and it will not own it; if the string does not include a NUL terminator, the
   returned pointer will own a NUL-terminated copy of *this.

   @return
      Pointer to the NUL-terminated string. Only valid as long as *this is, and only until the next
      change to *this.
   */
   detail::c_str_ptr c_str() const;

   /*! Returns the maximum number of characters the string buffer can currently hold.

   @return
      Size of the string buffer, in characters.
   */
   std::size_t capacity() const {
      return collections::detail::raw_vextr_impl_base::capacity<char_t>();
   }

   /*! Returns a const iterator set to the first character.

   @return
      Const iterator to the first character.
   */
   const_iterator cbegin() const {
      return const_cast<str *>(this)->begin();
   }

   /*! Returns a const iterator set beyond the last character.

   @return
      Const iterator to beyond the last character.
   */
   const_iterator cend() const {
      return const_cast<str *>(this)->end();
   }

   //! See vextr_impl::begin().
   char_t * chars_begin() {
      return vextr_impl::begin<char_t>();
   }

   //! See vextr_impl::begin().
   char_t const * chars_begin() const {
      return vextr_impl::begin<char_t>();
   }

   //! See vextr_impl::end().
   char_t * chars_end() {
      return vextr_impl::end<char_t>();
   }

   //! See vextr_impl::end().
   char_t const * chars_end() const {
      return vextr_impl::end<char_t>();
   }

   //! Truncates the string to zero length, without deallocating the internal buffer.
   void clear() {
      set_size(0);
   }

   /*! Returns a const reverse iterator set to the last character.

   @return
      Const reverse iterator to the last character.
   */
   const_reverse_iterator crbegin() const {
      return const_cast<str *>(this)->rbegin();
   }

   /*! Returns a const reverse iterator set to before the first character.

   @return
      Const reverse iterator to before the first character.
   */
   const_reverse_iterator crend() const {
      return const_cast<str *>(this)->rend();
   }

   /*! Returns the string, encoded as requested, into a byte vector.

   @param enc
      Requested encoding.
   @param bNulT
      If true, the resulting vector will contain an additional NUL terminator (using as many vector
      elements as the destination encoding’s character size); if false, no NUL terminator will be
      present.
   @return
      Resulting byte vector.
   */
   collections::dmvector<std::uint8_t> encode(encoding enc, bool bNulT) const;

   /*! Returns an iterator set beyond the last character.

   @return
      Iterator to the first character.
   */
   iterator end() {
      return iterator(this, size_in_chars());
   }

   /*! Returns a const iterator set beyond the last character.

   @return
      Const iterator to the first character.
   */
   const_iterator end() const {
      return const_cast<str *>(this)->end();
   }

   /*! Returns true if the string ends with a specified suffix.

   @param s
      String that *this should end with.
   @return
      true if *this ends with the specified suffix, or false otherwise.
   */
   bool ends_with(str const & s) const;

   /*! Searches for and returns the first occurrence of the specified character or substring.

   @param chNeedle
      Character to search for.
   @param sNeedle
      String to search for.
   @param itWhence
      Iterator to the first character whence the search should start. When not specified, it
      defaults to cbegin().
   @return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find(char_t chNeedle) const {
      return find(chNeedle, cbegin());
   }
#if ABC_HOST_UTF > 8
   const_iterator find(char chNeedle) const {
      return find(host_char(chNeedle));
   }
#endif
   const_iterator find(char32_t cpNeedle) const {
      return find(cpNeedle, cbegin());
   }
   const_iterator find(char_t chNeedle, const_iterator itWhence) const;
#if ABC_HOST_UTF > 8
   const_iterator find(char chNeedle, const_iterator itWhence) const {
      return find(host_char(chNeedle), itWhence);
   }
#endif
   const_iterator find(char32_t cpNeedle, const_iterator itWhence) const;
   const_iterator find(str const & sNeedle) const {
      return find(sNeedle, cbegin());
   }
   const_iterator find(str const & sNeedle, const_iterator itWhence) const;

   /*! Searches for and returns the last occurrence of the specified character or substring.

   @param chNeedle
      Character to search for.
   @param sNeedle
      String to search for.
   @param itWhence
      Iterator to the last character whence the search should start. When not specified, it
      defaults to cend().
   @return
      Iterator to the first occurrence of the character/string, or cend() when no matches are found.
   */
   const_iterator find_last(char_t chNeedle) const {
      return find_last(chNeedle, cend());
   }
#if ABC_HOST_UTF > 8
   const_iterator find_last(char chNeedle) const {
      return find_last(host_char(chNeedle));
   }
#endif
   const_iterator find_last(char32_t cpNeedle) const {
      return find_last(cpNeedle, cend());
   }
   const_iterator find_last(char_t chNeedle, const_iterator itWhence) const;
#if ABC_HOST_UTF > 8
   const_iterator find_last(char chNeedle, const_iterator itWhence) const {
      return find_last(host_char(chNeedle), itWhence);
   }
#endif
   const_iterator find_last(char32_t cpNeedle, const_iterator itWhence) const;
   const_iterator find_last(str const & sNeedle) const {
      return find_last(sNeedle, cend());
   }
   const_iterator find_last(str const & sNeedle, const_iterator itWhence) const;

   /*! Uses the current content of the string to generate a new one using io::text::writer::print().

   @param ts
      Replacement values.
   @return
      Resulting string.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   str format(Ts const &... ts) const;
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
   str format() const;
   template <typename T0>
   str format(T0 const & t0) const;
   template <typename T0, typename T1>
   str format(T0 const & t0, T1 const & t1) const;
   template <typename T0, typename T1, typename T2>
   str format(T0 const & t0, T1 const & t1, T2 const & t2) const;
   template <typename T0, typename T1, typename T2, typename T3>
   str format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   str format(T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4) const;
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   str format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   str format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   str format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   str format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8
   ) const;
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   str format(
      T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4, T5 const & t5,
      T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   ) const;
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

   /*! Converts a character index into its corresponding code point index.

   @param ich
      Character index. No validation is performed on it.
   @return
      Code point index. If ich is not a valid character index for the string, the return value is
      undefined.
   */
   std::size_t index_from_char_index(std::size_t ich) const {
      return str_traits::size_in_codepoints(chars_begin(), chars_begin() + ich);
   }

   /*! Inserts a character into the string at a specific character (not code point) offset.

   @param ichOffset
      0-based offset at which to insert the character.
   @param ch
      Character to insert.
   */
   void insert(std::size_t ichOffset, char_t ch) {
      insert(ichOffset, &ch, 1);
   }

#if ABC_HOST_UTF > 8
   /*! Inserts an ASCII character into the string at a specific character (not code point) offset.

   @param ichOffset
      0-based offset at which to insert the character.
   @param ch
      ASCII character to insert.
   */
   void insert(std::size_t ichOffset, char ch) {
      insert(ichOffset, host_char(ch));
   }
#endif

   /*! Inserts a code point into the string at a specific character (not code point) offset.

   @param ichOffset
      0-based offset at which to insert the character.
   @param cp
      Code point to insert.
   */
   void insert(std::size_t ichOffset, char32_t cp) {
      char_t ach[host_char_traits::max_codepoint_length];
      insert(
         ichOffset, ach,
         static_cast<std::size_t>(host_char_traits::codepoint_to_chars(cp, ach) - ach)
      );
   }

   /*! Inserts characters from a string into the string at a specific character (not code point)
   offset.

   @param ichOffset
      0-based offset at which to insert the characters.
   @param s
      String to insert.
   */
   void insert(std::size_t ichOffset, str const & s) {
      insert(ichOffset, s.chars_begin(), s.size_in_chars());
   }

   /*! Inserts characters into the string at a specific character (not code point) offset.

   @param ichOffset
      0-based offset at which to insert the characters.
   @param pchInsert
      Pointer to an array of characters to insert.
   @param cchInsert
      Count of characters in the array pointed to by pchInsert.
   */
   void insert(std::size_t ichOffset, char_t const * pchInsert, std::size_t cchInsert) {
      vextr_impl::insert_remove(
         sizeof(char_t) * ichOffset, pchInsert, sizeof(char_t) * cchInsert, 0
      );
   }

   /*! Returns a reverse iterator set to the last character.

   @return
      Reverse iterator to the last character.
   */
   reverse_iterator rbegin() {
      return reverse_iterator(iterator(this, size_in_chars()));
   }

   /*! Returns a const reverse iterator set to the last character.

   @return
      Const reverse iterator to the last character.
   */
   const_reverse_iterator rbegin() const {
      return const_cast<str *>(this)->rbegin();
   }

   /*! Returns a reverse iterator set to before the first character.

   @return
      Reverse iterator to before the first character.
   */
   reverse_iterator rend() {
      return reverse_iterator(iterator(this, 0));
   }

   /*! Returns a const reverse iterator set to before the first character.

   @return
      Const reverse iterator to before the first character.
   */
   const_reverse_iterator rend() const {
      return const_cast<str *>(this)->rend();
   }

   /*! Replaces all occurrences of a character with another character.

   @param chSearch
      Character to search for.
   @param chReplacement
      Character to replace chSearch with.
   */
   void replace(char_t chSearch, char_t chReplacement);

#if ABC_HOST_UTF > 8
   /*! Replaces all occurrences of an ASCII character with another ASCII character.

   @param chSearch
      Character to search for.
   @param chReplacement
      Character to replace chSearch with.
   */
   void replace(char chSearch, char chReplacement) {
      replace(host_char(chSearch), host_char(chReplacement));
   }
#endif

   /*! Replaces all occurrences of a code point with another code point.

   @param cpSearch
      Code point to search for.
   @param cpReplacement
      Code point to replace chSearch with.
   */
   void replace(char32_t cpSearch, char32_t cpReplacement);

   /*! See vextr_impl::set_capacity().

   @param cchMin
      Minimum count of characters requested.
   @param bPreserve
      If true, the previous contents of the string will be preserved even if the reallocation
      causes the string to switch to a different character array.
   */
   void set_capacity(std::size_t cchMin, bool bPreserve) {
      vextr_impl::set_capacity(sizeof(char_t) * cchMin, bPreserve);
   }

   /*! Expands the character array until the specified callback succeeds in filling it and returns a
   number of needed characters that’s less than the size of the buffer. For example, for cchMax == 3
   (NUL terminator included), it must return <= 2 (NUL excluded).

   This method is not transaction-safe; if an exception is thrown in the callback or elsewhere,
   *this will not be restored to its previous state.

   TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

   @param fnRead
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
   void set_from(_std::function<std::size_t (char_t * pch, std::size_t cchMax)> const & fnRead);

   /*! Changes the length of the string. If the string needs to be lengthened, the added characters
   will be left uninitialized.

   @param cch
      New length of the string.
   @param bClear
      If true, the string will be cleared after being resized; if false, no characters will be
      changed.
   */
   void set_size_in_chars(std::size_t cch, bool bClear = false) {
      vextr_impl::set_size(sizeof(char_t) * cch);
      if (bClear) {
         prepare_for_writing();
         memory::clear(chars_begin(), cch);
      }
   }

   /*! Returns size of the string, in code points.

   @return
      Size of the string.
   */
   std::size_t size() const {
      return str_traits::size_in_codepoints(chars_begin(), chars_end());
   }

   /*! Returns size of the string, in bytes.

   @return
      Size of the string.
   */
   std::size_t size_in_bytes() const {
      return vextr_impl::size<std::int8_t>();
   }

   /*! Returns size of the string, in characters.

   @return
      Size of the string.
   */
   std::size_t size_in_chars() const {
      return vextr_impl::size<char_t>();
   }

   /*! Returns true if the string starts with a specified prefix.

   @param s
      String that *this should start with.
   @return
      true if *this starts with the specified suffix, or false otherwise.
   */
   bool starts_with(str const & s) const;

   /*! Returns a portion of the string from the specified index to the end of the string.

   @param ichBegin
      Index of the first character of the substring. See str::translate_range() for allowed begin
      index values.
   @return
      Substring of *this.
   */
   str substr(std::ptrdiff_t ichBegin) const {
      return substr(ichBegin, static_cast<std::ptrdiff_t>(size_in_chars()));
   }

   /*! Returns a portion of the string.

   @param ichBegin
      Index of the first character of the substring. See str::translate_range() for allowed begin
      index values.
   @param ichEnd
      Index of the last character of the substring, exclusive. See str::translate_range() for
      allowed end index values.
   @return
      Substring of *this.
   */
   str substr(std::ptrdiff_t ichBegin, std::ptrdiff_t ichEnd) const {
      auto range(translate_range(ichBegin, ichEnd));
      return str(_std::get<0>(range).base(), _std::get<1>(range).base());
   }

   /*! Returns a portion of the string from the specified iterator to the end of the string.

   @param itBegin
      Iterator to the first character of the substring.
   @return
      Substring of *this.
   */
   str substr(const_iterator itBegin) const {
      validate_pointer(itBegin.base());
      return str(itBegin.base(), chars_end());
   }

   /*! Returns a portion of the string.

   @param itBegin
      Iterator to the first character of the substring.
   @param itEnd
      Iterator to past the end of the substring.
   @return
      Substring of *this.
   */
   str substr(const_iterator itBegin, const_iterator itEnd) const {
      validate_pointer(itBegin.base());
      validate_pointer(itEnd.base());
      return str(itBegin.base(), itEnd.base());
   }

protected:
   /*! Constructor for subclasses with an embedded character array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   */
   sstr(std::size_t cbEmbeddedCapacity) :
      vextr_impl(cbEmbeddedCapacity) {
   }

   /*! Move constructor for subclasses with an embedded character array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   @param s
      Source object.
   */
   sstr(std::size_t cbEmbeddedCapacity, str && s) :
      vextr_impl(cbEmbeddedCapacity) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(s));
   }

   /*! Copy constructor for subclasses with an embedded character array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   @param s
      Source object.
   */
   sstr(std::size_t cbEmbeddedCapacity, str const & s) :
      vextr_impl(cbEmbeddedCapacity) {
      vextr_impl::assign_share_raw_or_copy_desc(s);
   }

   /*! Constructor from string literals for subclasses with an embedded character array.

   @param cbEmbeddedCapacity
      Size of the embedded character array, in bytes.
   @param ach
      Source NUL-terminated string literal.
   */
   template <std::size_t t_cch>
   sstr(std::size_t cbEmbeddedCapacity, char_t const (& ach)[t_cch]) :
      vextr_impl(
         cbEmbeddedCapacity, &ach[0], &ach[ABC_SL_SIZE(ach)], ach[t_cch - 1 /*NUL*/] == '\0'
      ) {
   }

   /*! Constructor from string literals.

   @param pchConstSrc
      Pointer to a string that will be adopted by the str as read-only.
   @param cchSrc
      Count of characters in the string pointed to by pchConstSrc.
   @param bNulT
      true if the array pointed to by pchConstSrc is a NUL-terminated string, or false otherwise.
   */
   sstr(char_t const * pchConstSrc, std::size_t cchSrc, bool bNulT) :
      vextr_impl(0, pchConstSrc, pchConstSrc + cchSrc, bNulT) {
   }

   /*! Advances or backs up a character index by the specified number of code points, returning the
   resulting pointer. If the index is moved outside of the buffer, an index_error or
   iterator_error exception (depending on bIndex) is thrown.

   @param ich
      Initial index.
   @param iDelta
      Count of code points to move from ich by.
   @param bIndex
      If true, a movement to outside of [begin, end) will cause an index_error to be thrown; if
      false, a movement to outside of [begin, end] will cause an iterator_error to be thrown.
   @return
      Resulting pointer.
   */
   std::size_t advance_char_index(std::size_t ich, std::ptrdiff_t iDelta, bool bIndex) const;

   //! Prepares the character array to be modified.
   void prepare_for_writing();

   /*! Replaces a single character with another character.

   @param ich
      Index of the first character making up the code point to replace.
   @param chNew
      Character that will be written at index ich.
   */
   void replace_codepoint(std::size_t ich, char_t chNew);

#if ABC_HOST_UTF > 8
   /*! Replaces a single ASCII character with another ASCII character.

   @param ich
      Index of the first character making up the code point to replace.
   @param chNew
      Character that will be written at index ich.
   */
   void replace_codepoint(std::size_t ich, char chNew) {
      replace_codepoint(pch, host_char(chNew));
   }
#endif

   /*! Replaces a single code point with another code point.

   @param ich
      Index of the first character making up the code point to replace.
   @param cpNew
      Code point that will be encoded starting at character index ich.
   */
   void replace_codepoint(std::size_t ich, char32_t cpNew);

   /*! Converts a possibly negative character index into an iterator.

   @param ich
      If positive, this is interpreted as a 0-based index; if negative, it’s interpreted as a
      1-based index from the end of the character array by adding this->size() to it.
   @return
      Resulting iterator.
   */
   const_iterator translate_index(std::ptrdiff_t ich) const;

   /*! Converts a left-closed, right-open interval with possibly negative character indices into one
   consisting of two iterators.

   @param ichBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based index;
      if negative, it’s interpreted as a 1-based index from the end of the character array by adding
      this->size() to it.
   @param ichEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based
      index; if negative, it’s interpreted as a 1-based index from the end of the character array by
      adding this->size() to it.
   @return
      Left-closed, right-open interval such that get<0>(return) <= i < get<1>(return), or the empty
      interval [end(), end()) if the indices represent an empty interval after being adjusted.
   */
   _std::tuple<const_iterator, const_iterator> translate_range(
      std::ptrdiff_t ichBegin, std::ptrdiff_t ichEnd
   ) const;
};

template <std::size_t t_cchEmbeddedCapacity>
class sstr :
   private str,
   private collections::detail::raw_vextr_prefixed_item_array<char_t, t_cchEmbeddedCapacity> {
private:
   using collections::detail::raw_vextr_prefixed_item_array<
      char_t, t_cchEmbeddedCapacity
   >::smc_cbEmbeddedCapacity;

public:
   //! Default constructor.
   sstr() :
      text::str(smc_cbEmbeddedCapacity) {
   }

   /*! Move constructor.

   @param s
      Source object.
   */
   sstr(text::str && s) :
      text::str(smc_cbEmbeddedCapacity, _std::move(s)) {
   }

   /*! Copy constructor.

   @param s
      Source object.
   */
   template <std::size_t t_cchEmbeddedCapacity2>
   sstr(sstr<t_cchEmbeddedCapacity2> const & s) :
      text::str(smc_cbEmbeddedCapacity, s.str()) {
   }

   /*! Constructor that will copy the source string literal.

   @param ach
      Source NUL-terminated string literal.
   */
   template <std::size_t t_cch>
   sstr(char_t const (& ach)[t_cch]) :
      text::str(smc_cbEmbeddedCapacity, ach) {
   }

   /*! Move-assignment operator.

   @param s
      Source object.
   @return
      *this.
   */
   sstr & operator=(text::str && s) {
      text::str::operator=(_std::move(s));
      return *this;
   }

   /*! Copy-assignment operator.

   @param s
      Source string.
   @return
      *this.
   */
   template <std::size_t t_cchEmbeddedCapacity2>
   sstr & operator=(sstr<t_cchEmbeddedCapacity2> const & s) {
      text::str::operator=(s.str());
      return *this;
   }

   /*! Assignment operator.

   @param ach
      Source NUL-terminated string literal.
   @return
      *this.
   */
   template <std::size_t t_cch>
   sstr & operator=(char_t const (& ach)[t_cch]) {
      text::str::operator=(ach);
      return *this;
   }

   using text::str::operator[];

   /*! Boolean evaluation operator.

   @return
      true if the string is not empty, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return *static_cast<text::str *>(this) ? true : false;
   }

   /*! Concatenation-assignment operator.

   @param ch
      Character to append.
   @return
      *this.
   */
   sstr & operator+=(char_t ch) {
      text::str::operator+=(ch);
      return *this;
   }

#if ABC_HOST_UTF > 8
   /*! Concatenation-assignment operator.

   @param ch
      ASCII character to append.
   @return
      *this.
   */
   sstr & operator+=(char ch) {
      text::str::operator+=(host_char(ch));
      return *this;
   }
#endif

   /*! Concatenation-assignment operator.

   @param ch
      Code point to append.
   @return
      *this.
   */
   sstr & operator+=(char32_t cp) {
      text::str::operator+=(cp);
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param s
      String to append.
   @return
      *this.
   */
   template <std::size_t t_cchEmbeddedCapacity2>
   sstr & operator+=(sstr<t_cchEmbeddedCapacity2> const & s) {
      text::str::operator=(s);
      return *this;
   }

   using text::str::append;
   using text::str::begin;
   using text::str::c_str;
   using text::str::capacity;
   using text::str::cbegin;
   using text::str::cend;
   using text::str::chars_begin;
   using text::str::chars_end;
   using text::str::clear;
   using text::str::crbegin;
   using text::str::crend;
   using text::str::encode;
   using text::str::end;
   using text::str::ends_with;
   using text::str::find;
   using text::str::find_last;
   using text::str::format;
   using text::str::index_from_char_index;
   using text::str::insert;
   using text::str::rbegin;
   using text::str::rend;
   using text::str::replace;
   using text::str::set_capacity;
   using text::str::set_from;
   using text::str::set_size_in_chars;
   using text::str::size;
   using text::str::size_in_bytes;
   using text::str::size_in_chars;
   using text::str::starts_with;

   /*! Allows using the object as an abc::text::str const instance.

   @return
      *this.
   */
   text::str const & str() const {
      return *this;
   }

   /*! Returns a pointer to the object as an abc::text::str instance.

   @return
      this.
   */
   text::str * str_ptr() {
      return this;
   }

   using text::str::substr;
};

// Now these can be defined.

inline str::const_codepoint_proxy::operator char32_t() const {
   return host_char_traits::chars_to_codepoint(mc_ps->chars_begin() + mc_ich);
}

inline str::codepoint_proxy & str::codepoint_proxy::operator=(char_t ch) {
   const_cast<str *>(mc_ps)->replace_codepoint(mc_ich, ch);
   return *this;
}

inline str::codepoint_proxy & str::codepoint_proxy::operator=(char32_t cp) {
   const_cast<str *>(mc_ps)->replace_codepoint(mc_ich, cp);
   return *this;
}

inline std::size_t str::const_iterator::advance(std::ptrdiff_t iDelta, bool bIndex) const {
   return m_ps->advance_char_index(m_ich, iDelta, bIndex);
}

inline char_t const * str::const_iterator::base() const {
   return m_ps->chars_begin() + m_ich;
}

inline char_t * str::iterator::base() const {
   return const_cast<str *>(m_ps)->chars_begin() + m_ich;
}

// Relational operators for str.
#define ABC_RELOP_IMPL(op) \
   template <std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR> \
   inline bool operator op( \
      sstr<t_cchEmbeddedCapacityL> const & sL, sstr<t_cchEmbeddedCapacityR> const & sR \
   ) { \
      return str_traits::compare( \
         sL.chars_begin(), sL.chars_end(), sR.chars_begin(), sR.chars_end() \
      ) op 0; \
   } \
   template <std::size_t t_cchEmbeddedCapacity, std::size_t t_cch> \
   inline bool operator op(sstr<t_cchEmbeddedCapacity> const & s, char_t const (& ach)[t_cch]) { \
      return str_traits::compare( \
         s.chars_begin(), s.chars_end(), &ach[0], &ach[ABC_SL_SIZE(ach)] \
      ) op 0; \
   } \
   template <std::size_t t_cch, std::size_t t_cchEmbeddedCapacity> \
   inline bool operator op(char_t const (& ach)[t_cch], sstr<t_cchEmbeddedCapacity> const & s) { \
      return str_traits::compare( \
         &ach[0], &ach[ABC_SL_SIZE(ach)], s.chars_begin(), s.chars_end() \
      ) op 0; \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

/* Relational operators for str::const_codepoint_proxy. Provided so that comparisons between
char32_t (from codepoint_proxy) and char{,8,16}_t don’t raise warnings. */
#define ABC_RELOP_IMPL(op) \
   inline bool operator op( \
      str::const_codepoint_proxy const & cppL, str::const_codepoint_proxy const & cppR \
   ) { \
      return cppL.operator char32_t() op cppR.operator char32_t(); \
   } \
   inline bool operator op(str::const_codepoint_proxy const & cpp, char_t ch) { \
      return cpp.operator char32_t() op codepoint(ch); \
   } \
   inline bool operator op(char_t ch, str::const_codepoint_proxy const & cpp) { \
      return codepoint(ch) op cpp.operator char32_t(); \
   } \
   inline bool operator op(str::const_codepoint_proxy const & cpp, char32_t cp) { \
      return cpp.operator char32_t() op cp; \
   } \
   inline bool operator op(char32_t cp, str::const_codepoint_proxy const & cpp) { \
      return cp op cpp.operator char32_t(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

#if ABC_HOST_UTF > 8
   #define ABC_RELOP_IMPL(op) \
      inline bool operator op(str::const_codepoint_proxy const & cpp, char ch) { \
         return operator op(cpp, host_char(ch)); \
      } \
      inline bool operator op(char ch, str::const_codepoint_proxy const & cpp) { \
         return operator op(host_char(ch), cpp); \
      }
   ABC_RELOP_IMPL(==)
   ABC_RELOP_IMPL(!=)
   ABC_RELOP_IMPL(>)
   ABC_RELOP_IMPL(>=)
   ABC_RELOP_IMPL(<)
   ABC_RELOP_IMPL(<=)
   #undef ABC_RELOP_IMPL
#endif

/*! Concatenation operator.

@param sL
   Left string operand.
@param sR
   Right string operand.
@param chL
   Left character operand.
@param chR
   Right character operand.
@return
   Resulting string.
*/
template <std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline str operator+(
   sstr<t_cchEmbeddedCapacityL> const & sL, sstr<t_cchEmbeddedCapacityR> const & sR
) {
   return str(sL.chars_begin(), sL.chars_end(), sR.chars_begin(), sR.chars_end());
}

template <std::size_t t_cchEmbeddedCapacityL, std::size_t t_cchEmbeddedCapacityR>
inline sstr<t_cchEmbeddedCapacityL> operator+(
   sstr<t_cchEmbeddedCapacityL> && sL, sstr<t_cchEmbeddedCapacityR> const & sR
) {
   sL += sR;
   return _std::move(sL);
}

// Overloads taking a string or character literal as right operand.
template <std::size_t t_cchEmbeddedCapacity, std::size_t t_cch>
inline sstr<t_cchEmbeddedCapacity> operator+(
   sstr<t_cchEmbeddedCapacity> && sL, char_t const (& achR)[t_cch]
) {
   sL += achR;
   return _std::move(sL);
}

template <std::size_t t_cchEmbeddedCapacity, std::size_t t_cch>
inline str operator+(sstr<t_cchEmbeddedCapacity> const & sL, char_t const (& achR)[t_cch]) {
   return str(sL.chars_begin(), sL.chars_end(), &achR[0], &achR[ABC_SL_SIZE(achR)]);
}

template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(sstr<t_cchEmbeddedCapacity> && sL, char_t chR) {
   sL += chR;
   return _std::move(sL);
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(sstr<t_cchEmbeddedCapacity> const & sL, char_t chR) {
   return str(sL.chars_begin(), sL.chars_end(), &chR, &chR + 1);
}

#if ABC_HOST_UTF > 8
template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(sstr<t_cchEmbeddedCapacity> && sL, char chR) {
   return operator+(_std::move(sL), host_char(chR));
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(sstr<t_cchEmbeddedCapacity> const & sL, char chR) {
   return operator+(sL, host_char(chR));
}
#endif

template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(sstr<t_cchEmbeddedCapacity> && sL, char32_t cpR) {
   sL += cpR;
   return _std::move(sL);
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(sstr<t_cchEmbeddedCapacity> const & sL, char32_t cpR) {
   char_t achR[host_char_traits::max_codepoint_length];
   return str(
      sL.chars_begin(), sL.chars_end(), achR, host_char_traits::codepoint_to_chars(cpR, achR)
   );
}

// Overloads taking a string or character literal as left operand.
template <std::size_t t_cch, std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(
   char_t const (& achL)[t_cch], sstr<t_cchEmbeddedCapacity> && sR
) {
   sR.insert(0, achL);
   return _std::move(sR);
}

template <std::size_t t_cch, std::size_t t_cchEmbeddedCapacity>
inline str operator+(char_t const (& achL)[t_cch], sstr<t_cchEmbeddedCapacity> const & sR) {
   return str(&achL[0], &achL[ABC_SL_SIZE(achL)], sR.chars_begin(), sR.chars_end());
}

template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(char_t chL, sstr<t_cchEmbeddedCapacity> && sR) {
   sR.insert(0, chL);
   return _std::move(sR);
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(char_t chL, sstr<t_cchEmbeddedCapacity> const & sR) {
   return str(&chL, &chL + 1, sR.chars_begin(), sR.chars_end());
}

#if ABC_HOST_UTF > 8
template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(char chL, sstr<t_cchEmbeddedCapacity> && sR) {
   return operator+(host_char(chL), _std::move(sR));
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(char chL, sstr<t_cchEmbeddedCapacity> const & sR) {
   return operator+(host_char(chL), sR);
}
#endif

template <std::size_t t_cchEmbeddedCapacity>
inline sstr<t_cchEmbeddedCapacity> operator+(char32_t cpL, sstr<t_cchEmbeddedCapacity> && sR) {
   sR.insert(0, cpL);
   return _std::move(sR);
}

template <std::size_t t_cchEmbeddedCapacity>
inline str operator+(char32_t cpL, sstr<t_cchEmbeddedCapacity> const & sR) {
   char_t achL[host_char_traits::max_codepoint_length];
   return str(
      achL, host_char_traits::codepoint_to_chars(cpL, achL), sR.chars_begin(), sR.chars_end()
   );
}

}} //namespace abc::text

namespace std {

template <>
struct ABACLADE_SYM hash<abc::text::str> {
   std::size_t operator()(abc::text::str const & s) const;
};

template <std::size_t t_cchEmbeddedCapacity>
struct hash<abc::text::sstr<t_cchEmbeddedCapacity>> : public hash<abc::text::str> {
};

} //namespace std

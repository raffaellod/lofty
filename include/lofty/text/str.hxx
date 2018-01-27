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

namespace lofty {

//! Type of lofty::external_buffer.
struct external_buffer_t {
   //! Default constructor. Required to instantiate a const instance.
   external_buffer_t() {
   }
};

/*! Constant similar in use to std::nothrow; when specified as extra argument for lofty::text::*str
constructors, it indicates that the string should use an external buffer that is guaranteed by the caller to
have a scope lifetime equal or longer than that of the string. */
extern LOFTY_SYM external_buffer_t const external_buffer;

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text {

// Specialization with no embedded character array: this is the plain lofty::text::str.
template <>
class LOFTY_SYM sstr<0> :
   protected collections::_pvt::trivial_vextr_impl,
   public support_explicit_operator_bool<str> {
private:
   typedef collections::_pvt::trivial_vextr_impl vextr_impl;

public:
   /*! Pointer to a C-style, NUL-terminated character array that may or may not share memory with an
   lofty::text::*str instance. */
   class c_str_ptr {
   private:
      //! Internal conditionally-deleting pointer type.
      typedef _std::unique_ptr<
         char_t const[], memory::conditional_deleter<char_t const[], memory::freeing_deleter>
      > pointer;

   public:
      /*! Constructor.

      @param ptr_
         Pointer to the character array.
      @param own
         If true, the pointer will own the character array; if false, it won’t try to deallocate it.
      */
      c_str_ptr(char_t const * ptr_, bool own) :
         ptr(ptr_, pointer::deleter_type(own)) {
      }

      /*! Move constructor.

      @param src
         Source object.
      */
      c_str_ptr(c_str_ptr && src) :
         ptr(_std::move(src.ptr)) {
      }

      /*! Move-assignment operator.

      @param src
         Source object.
      @return
         *this.
      */
      c_str_ptr & operator=(c_str_ptr && src) {
         ptr = _std::move(src.ptr);
         return *this;
      }

      /*! Implicit conversion to char_t const *.

      @return
         Pointer to the character array.
      */
      operator char_t const *() const {
         return ptr.get();
      }

      /*! Enables access to the internal pointer.

      @return
         Reference to the internal pointer.
      */
      pointer const & _get() const {
         return ptr;
      }

   private:
      //! Conditionally-deleting pointer.
      pointer ptr;
   };

   //! Presents a lofty::text::str character(s) as a char32_t const &.
   class const_codepoint_proxy {
   private:
      friend class sstr;

   public:
      /*! Implicit conversion to a code point.

      @return
         Code point that the proxy is currently referencing.
      */
      operator char32_t() const {
         return host_char_traits::chars_to_codepoint(owner_str->data() + char_index);
      }


   private:
      /*! Constructor.

      @param owner_str_
         Pointer to the containing string.
      @param char_index_
         Index of the character(s) that this proxy will present as char32_t.
      */
      const_codepoint_proxy(str const * owner_str_, std::size_t char_index_) :
         owner_str(owner_str_),
         char_index(char_index_) {
      }

   protected:
      //! Pointer to the containing string.
      str const * const owner_str;
      //! Index of the proxied character(s).
      std::size_t const char_index;
   };

   //! Presents a lofty::text::str character(s) as a char32_t &.
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
      codepoint_proxy & operator=(char_t ch) {
         const_cast<str *>(owner_str)->replace_codepoint(char_index, ch);
         return *this;
      }

   #if LOFTY_HOST_UTF > 8
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
      codepoint_proxy & operator=(char32_t cp) {
         const_cast<str *>(owner_str)->replace_codepoint(char_index, cp);
         return *this;
      }

      /*! Copy-assignment operator. This copies the char32_t value, not the internal data members; this allows
      writing expressions like *dst_itr = *src_itr to copy code points from one iterator to another.

      @param src
         Source object to copy a code point from.
      @return
         *this.
      */
      codepoint_proxy & operator=(codepoint_proxy const & src) {
         return operator=(src.operator char32_t());
      }

      /*! Copy-assignment operator. This copies the char32_t value, not the internal data members; this allows
      writing expressions like *dst_itr = *src_itr to copy code points from one iterator to another. This
      overload supports copying a code point from a const iterator to a non-const iterator.

      @param src
         Source object to copy a code point from.
      @return
         *this.
      */
      codepoint_proxy & operator=(const_codepoint_proxy const & src) {
         return operator=(src.operator char32_t());
      }

   private:
      /*! Constructor.

      @param owner_str_
         Pointer to the containing string.
      @param char_index_
         Index of the character(s) that this proxy will present as char32_t.
      */
      codepoint_proxy(str * owner_str_, std::size_t char_index_) :
         const_codepoint_proxy(owner_str_, char_index_) {
      }
   };

   typedef char_t value_type;
   typedef char_t * pointer;
   typedef char_t const * const_pointer;
   typedef char_t & reference;
   typedef char_t const & const_reference;
   typedef std::size_t size_type;
   typedef std::ptrdiff_t difference_type;

   /*! Code point iterator that hides the underlying encoded representation, presenting a string as an array
   of code points (char32_t). Pointers/references are still char_t. */
   class LOFTY_SYM const_iterator {
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
         owner_str(nullptr),
         char_index_(0) {
      }

      /*! Dereferencing operator.

      @return
         Reference to the current character.
      */
      const_codepoint_proxy operator*() const {
         str::validate_index_to_pointer(owner_str, char_index_, false);
         return const_codepoint_proxy(owner_str, char_index_);
      }

      /*! Element access operator.

      @param i
         Index relative to *this. If the resulting index is outside of the string’s [begin, end) range, a
         collections::out_of_range exception will be thrown.
      @return
         Reference to the specified item.
      */
      const_codepoint_proxy operator[](std::ptrdiff_t i) const {
         return const_codepoint_proxy(
            owner_str, str::advance_index_by_codepoint_delta(owner_str, char_index_, i, false)
         );
      }

      /*! Addition-assignment operator.

      @param i
         Count of positions by which to advance the iterator. If the resulting iterator is outside of the
         string’s [begin, end] range, a collections::out_of_range exception will be thrown.
      @return
         *this after it’s moved forward by i positions.
      */
      const_iterator & operator+=(std::ptrdiff_t i) {
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, i, true);
         return *this;
      }

      /*! Subtraction-assignment operator.

      @param i
         Count of positions by which to rewind the iterator. If the resulting iterator is outside of the
         string’s [begin, end] range, a collections::out_of_range exception will be thrown.
      @return
         *this after it’s moved backwards by i positions.
      */
      const_iterator & operator-=(std::ptrdiff_t i) {
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, -i, true);
         return *this;
      }

      /*! Addition operator.

      @param i
         Count of positions by which to advance the iterator. If the resulting iterator is outside of the
         string’s [begin, end] range, a collections::out_of_range exception will be thrown.
      @return
         Iterator that’s i items ahead of *this.
      */
      const_iterator operator+(std::ptrdiff_t i) const {
         return const_iterator(
            owner_str, str::advance_index_by_codepoint_delta(owner_str, char_index_, i, true)
         );
      }

      /*! Subtraction operator.

      @param i
         Count of positions by which to rewind the iterator. If the resulting iterator is outside of the
         string’s [begin, end] range, a collections::out_of_range exception will be thrown.
      @return
         Iterator that’s i items behind *this.
      */
      const_iterator operator-(std::ptrdiff_t i) const {
         return const_iterator(
            owner_str, str::advance_index_by_codepoint_delta(owner_str, char_index_, -i, true)
         );
      }

      /*! Difference operator.

      @param right
         Iterator from which to calculate the distance.
      @return
         Distance between *this and right, in code points.
      */
      std::ptrdiff_t operator-(const_iterator right) const {
         return distance(right.char_index_);
      }

      /*! Preincrement operator. If the resulting iterator was already at the string’s end, a
      collections::out_of_range exception will be thrown.

      @return
         *this after it’s moved to the value following the one currently pointed to.
      */
      const_iterator & operator++() {
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, 1, true);
         return *this;
      }

      /*! Postincrement operator. If the resulting iterator was already at the string’s end, a
      collections::out_of_range exception will be thrown.

      @return
         Iterator pointing to the value following the one pointed to by this iterator.
      */
      const_iterator operator++(int) {
         std::size_t old_char_index = char_index_;
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, 1, true);
         return const_iterator(owner_str, old_char_index);
      }

      /*! Predecrement operator. If the resulting iterator was already at the string’s beginning, a
      collections::out_of_range exception will be thrown.

      @return
         *this after it’s moved to the value preceding the one currently pointed to.
      */
      const_iterator & operator--() {
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, -1, true);
         return *this;
      }

      /*! Postdecrement operator. If the resulting iterator was already at the string’s beginning, a
      collections::out_of_range exception will be thrown.

      @return
         Iterator pointing to the value preceding the one pointed to by this iterator.
      */
      const_iterator operator--(int) {
         std::size_t old_char_index = char_index_;
         char_index_ = str::advance_index_by_codepoint_delta(owner_str, char_index_, -1, true);
         return const_iterator(owner_str, old_char_index);
      }

   // Relational operators.
   #define LOFTY_RELOP_IMPL(op) \
      bool operator op(const_iterator const & right) const { \
         return char_index_ op right.char_index_; \
      }
   LOFTY_RELOP_IMPL(==)
   LOFTY_RELOP_IMPL(!=)
   LOFTY_RELOP_IMPL(>)
   LOFTY_RELOP_IMPL(>=)
   LOFTY_RELOP_IMPL(<)
   LOFTY_RELOP_IMPL(<=)
   #undef LOFTY_RELOP_IMPL

      /*! Returns the referenced character index. Note that that’s not a code point index, i.e. two
      consecutive iterators may differ in char_index() by more than 1.

      @return
         Pointer to the referenced character index.
      */
      std::size_t char_index() const {
         return char_index_;
      }

      /*! Returns a pointer to the referenced character.

      @return
         Pointer to the referenced character.
      */
      char_t const * ptr() const {
         return str::validate_index_to_pointer(owner_str, char_index_, true);
      }

   protected:
      /*! Computes the distance from another iterator/index.

      @param other_char_index
         Index from which to calculate the distance.
      @return
         Distance between *this and other_char_index, in code points.
      */
      std::ptrdiff_t distance(std::size_t other_char_index) const;

   protected:
      /*! Constructor.

      @param owner_str_
         Pointer to the string that is creating the iterator.
      @param char_index__
         Character index to set the iterator to.
      */
      const_iterator(str const * owner_str_, std::size_t char_index__) :
         owner_str(owner_str_),
         char_index_(char_index__) {
      }

   protected:
      //! Pointer to the source string.
      str const * owner_str;
      //! Index of the current character.
      std::size_t char_index_;
   };

   /*! Character iterator that hides the underlying encoded representation, presenting a string as an array of
   code points (char32_t). Pointers/references are still char_t. */
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
         str::validate_index_to_pointer(owner_str, char_index_, false);
         return codepoint_proxy(const_cast<str *>(owner_str), char_index_);
      }

      /*! Element access operator.

      @param i
         Index relative to *this. If the resulting index is outside of the string’s [begin, end) range, a
         collections::out_of_range exception will be thrown.
      @return
         Reference to the specified item.
      */
      codepoint_proxy operator[](std::ptrdiff_t i) const {
         return codepoint_proxy(
            const_cast<str *>(owner_str),
            str::advance_index_by_codepoint_delta(owner_str, char_index_, i, false)
         );
      }

      /*! Returns a pointer to the referenced character.

      @return
         Pointer to the referenced character.
      */
      char_t * ptr() const {
         return const_cast<str *>(owner_str)->data() + char_index_;
      }

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

      //! See const_iterator::operator-();
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

      @param owner_str_
         Pointer to the string that is creating the iterator.
      @param char_index__
         Character index to set the iterator to.
      */
      iterator(str * owner_str_, std::size_t char_index__) :
         const_iterator(owner_str_, char_index__) {
      }

      /*! Copy constructor. Allows to convert from non-const to const iterator types.

      @param src
         Source object.
      */
      iterator(const_iterator const & src) :
         const_iterator(src) {
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
   sstr(str && src) :
      vextr_impl(0) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(src));
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   sstr(str const & src) :
      vextr_impl(0) {
      vextr_impl::assign_share_raw_or_copy_desc(src);
   }

   /*! Constructor from string literals.

   @param src
      Source NUL-terminated string literal.
   */
   template <std::size_t src_size>
   sstr(char_t const (& src)[src_size]) :
      vextr_impl(0, &src[0], &src[LOFTY_SL_SIZE(src)], src[src_size - 1 /*NUL*/] == '\0') {
   }

   /*! Constructor that copies the contents of a character buffer.

   @param src_begin
      Pointer to the beginning of the source string.
   @param src_end
      Pointer to the end of the source string.
   */
   sstr(char_t const * src_begin, char_t const * src_end) :
      vextr_impl(0) {
      vextr_impl::assign_copy(src_begin, src_end);
   }

   /*! Constructor that creates a new string from two character buffers.

   @param src1_begin
      Pointer to the beginning of the left source string to concatenate.
   @param src1_end
      Pointer to the end of the left source string.
   @param src2_begin
      Pointer to the beginning of the right source string to concatenate.
   @param src2_end
      Pointer to the end of the right source string.
   */
   sstr(
      char_t const * src1_begin, char_t const * src1_end, char_t const * src2_begin, char_t const * src2_end
   ) :
      vextr_impl(0) {
      vextr_impl::assign_concat(src1_begin, src1_end, src2_begin, src2_end);
   }

   /*! Constructor that makes the string refer to the specified NUL-terminated raw C string.

   @param s
      Pointer to the source NUL-terminated string literal.
   */
   sstr(external_buffer_t const &, char_t const * s) :
      vextr_impl(0, s, s + text::size_in_chars(s), true) {
   }

   /*! Constructor that will make the string refer to the specified raw C string.

   @param src
      Pointer to the source string.
   @param src_char_size
      Count of characters in the array pointed to be src.
   */
   sstr(external_buffer_t const &, char_t const * src, std::size_t src_char_size) :
      vextr_impl(0, src, src + src_char_size, false) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   str & operator=(str && src) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   str & operator=(str const & src) {
      vextr_impl::assign_share_raw_or_copy_desc(src);
      return *this;
   }

   /*! Assignment operator from string literal.

   @param src
      Source NUL-terminated string literal.
   @return
      *this.
   */
   template <std::size_t src_size>
   str & operator=(char_t const (& src)[src_size]) {
      str src_str(src);
      operator=(_std::move(src_str));
      return *this;
   }

   /*! Character access operator.

   @param i
      Character index. If outside of the [begin, end) range, a collections::out_of_range exception will be
      thrown.
   @return
      Character at index i.
   */
   codepoint_proxy operator[](std::ptrdiff_t i) {
      return codepoint_proxy(this, advance_index_by_codepoint_delta(0, i, false));
   }

   /*! Const character access operator.

   @param i
      Character index. If outside of the [begin, end) range, a collections::out_of_range exception will be
      thrown.
   @return
      Character at index i.
   */
   const_codepoint_proxy operator[](std::ptrdiff_t i) const {
      return const_codepoint_proxy(this, advance_index_by_codepoint_delta(0, i, false));
   }

   /*! Boolean evaluation operator.

   @return
      true if the string is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      // Use std::int8_t to avoid multiplying by sizeof(char_t) when all we need is a greater-than check.
      return collections::_pvt::vextr_impl_base::end<std::int8_t>() >
         collections::_pvt::vextr_impl_base::begin<std::int8_t>();
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

#if LOFTY_HOST_UTF > 8
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

   @param cp
      Code point to append.
   @return
      *this.
   */
   str & operator+=(char32_t cp) {
      char_t cp_chars[host_char_traits::max_codepoint_length];
      append(
         cp_chars, static_cast<std::size_t>(host_char_traits::codepoint_to_chars(cp, cp_chars) - cp_chars)
      );
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param src
      String to append.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   str & operator+=(sstr<src_embedded_capacity> const & src) {
      append(src.data(), src.size_in_chars());
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param src
      NUL-terminated string literal to append.
   @return
      *this.
   */
   template <std::size_t src_size>
   str & operator+=(char_t const (& src)[src_size]) {
      append(src, LOFTY_SL_SIZE(src));
      return *this;
   }

   /*! Same as operator+=(), but for multi-argument overloads.

   @param src
      Pointer to an array of characters to append.
   @param src_char_size
      Count of characters in the array pointed to by src.
   */
   void append(char_t const * src, std::size_t src_char_size) {
      vextr_impl::insert_remove(
         collections::_pvt::vextr_impl_base::size<std::int8_t>(), src, sizeof(char_t) * src_char_size, 0
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

   If the string does not include a NUL terminator, the character array will be expanded to include one.
   Either way, the returned pointer will refer to the same character array, and it will not own it and the
   pointed-to memory.

   @return
      Pointer to the NUL-terminated string. Only valid as long as *this is, and only until the next change to
      *this.
   */
   c_str_ptr c_str();

   /*! Returns a pointer to a NUL-terminated version of the string.

   If the string already includes a NUL terminator, the returned pointer will refer to the same character
   array, and it will not own it; if the string does not include a NUL terminator, the returned pointer will
   own a NUL-terminated copy of *this.

   @return
      Pointer to the NUL-terminated string. Only valid as long as *this is, and only until the next change to
      *this.
   */
   c_str_ptr c_str() const;

   /*! Returns the maximum number of characters the string buffer can currently hold.

   @return
      Size of the string buffer, in characters.
   */
   std::size_t capacity() const {
      return collections::_pvt::vextr_impl_base::capacity<char_t>();
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

   /*! Returns a pointer to the character array.

   @return
      Pointer to the character array.
   */
   char_t * data() {
      return vextr_impl::begin<char_t>();
   }

   /*! Returns a const pointer to the character array.

   @return
      Const pointer to the character array.
   */
   char_t const * data() const {
      return vextr_impl::begin<char_t>();
   }

   /*! Returns a pointer to the end of the character array.

   @return
      Pointer to the end of the character array.
   */
   char_t * data_end() {
      return vextr_impl::end<char_t>();
   }

   /*! Returns a const pointer to the end of the character array.

   @return
      Const pointer to the end of the character array.
   */
   char_t const * data_end() const {
      return vextr_impl::end<char_t>();
   }

   /*! Returns the string, encoded as requested, into a byte vector.

   @param enc
      Requested encoding.
   @param add_nul_term
      If true, the resulting vector will contain an additional NUL terminator (using as many vector elements
      as the destination encoding’s character size); if false, no NUL terminator will be present.
   @return
      Resulting byte vector.
   */
   collections::vector<std::uint8_t> encode(encoding enc, bool add_nul_term = false) const;

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

   /*! Searches for and returns the first occurrence of the specified character.

   @param ch
      Character to search for.
   @return
      Iterator to the first occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find(char_t ch) const {
      return find(ch, cbegin());
   }

#if LOFTY_HOST_UTF > 8
   /*! Searches for and returns the first occurrence of the specified ASCII character.

   @param ch
      ASCII character to search for.
   @return
      Iterator to the first occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find(char ch) const {
      return find(host_char(ch));
   }
#endif

   /*! Searches for and returns the first occurrence of the specified code point.

   @param cp
      Code point to search for.
   @return
      Iterator to the first occurrence of the code point, or cend() when no matches are found.
   */
   const_iterator find(char32_t cp) const {
      return find(cp, cbegin());
   }

   /*! Searches for and returns the first occurrence after whence of the specified character.

   @param ch
      Character to search for.
   @param whence
      Iterator to the first character whence the search should start.
   @return
      Iterator to the first occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find(char_t ch, const_iterator whence) const;

#if LOFTY_HOST_UTF > 8
   /*! Searches for and returns the first occurrence after whence of the specified ASCII character.

   @param ch
      ASCII character to search for.
   @param whence
      Iterator to the first character whence the search should start.
   @return
      Iterator to the first occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find(char ch, const_iterator whence) const {
      return find(host_char(ch), whence);
   }
#endif

   /*! Searches for and returns the first occurrence after whence of the specified code point.

   @param cp
      Code point to search for.
   @param whence
      Iterator to the first character whence the search should start.
   @return
      Iterator to the first occurrence of the code point, or cend() when no matches are found.
   */
   const_iterator find(char32_t cp, const_iterator whence) const;

   /*! Searches for and returns the first occurrence of the specified substring.

   @param substr_
      Substring to search for.
   @return
      Iterator to the first occurrence of the substring, or cend() when no matches are found.
   */
   const_iterator find(str const & substr_) const {
      return find(substr_, cbegin());
   }

   /*! Searches for and returns the first occurrence after whence of the specified substring.

   @param substr
      Substring to search for.
   @param whence
      Iterator to the first character whence the search should start.
   @return
      Iterator to the first occurrence of the substring, or cend() when no matches are found.
   */
   const_iterator find(str const & substr, const_iterator whence) const;

   /*! Searches for and returns the last occurrence of the specified character.

   @param ch
      Character to search for.
   @return
      Iterator to the last occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find_last(char_t ch) const {
      return find_last(ch, cend());
   }

#if LOFTY_HOST_UTF > 8
   /*! Searches for and returns the last occurrence of the specified ASCII character.

   @param ch
      ASCII character to search for.
   @return
      Iterator to the last occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find_last(char ch) const {
      return find_last(host_char(ch));
   }
#endif

   /*! Searches for and returns the last occurrence of the specified code point.

   @param cp
      Code point to search for.
   @return
      Iterator to the last occurrence of the code point, or cend() when no matches are found.
   */
   const_iterator find_last(char32_t cp) const {
      return find_last(cp, cend());
   }

   /*! Searches for and returns the last occurrence of the specified character.

   @param ch
      Character to search for.
   @param whence
      Iterator to the last character whence the search should start.
   @return
      Iterator to the last occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find_last(char_t ch, const_iterator whence) const;

#if LOFTY_HOST_UTF > 8
   /*! Searches for and returns the last occurrence of the specified ASCII character.

   @param ch
      ASCII character to search for.
   @param whence
      Iterator to the last character whence the search should start.
   @return
      Iterator to the last occurrence of the character, or cend() when no matches are found.
   */
   const_iterator find_last(char ch, const_iterator whence) const {
      return find_last(host_char(ch), whence);
   }
#endif

   /*! Searches for and returns the last occurrence before whence of the specified code point.

   @param cp
      Code point to search for.
   @param whence
      Iterator to the last character whence the search should start.
   @return
      Iterator to the last occurrence of the code point, or cend() when no matches are found.
   */
   const_iterator find_last(char32_t cp, const_iterator whence) const;

   /*! Searches for and returns the last occurrence of the specified substring.

   @param substr_
      Substring to search for.
   @return
      Iterator to the last occurrence of the substring, or cend() when no matches are found.
   */
   const_iterator find_last(str const & substr_) const {
      return find_last(substr_, cend());
   }

   /*! Searches for and returns the last occurrence before whence of the specified substring.

   @param substr
      Substring to search for.
   @param whence
      Iterator to the last character whence the search should start.
   @return
      Iterator to the last occurrence of the substring, or cend() when no matches are found.
   */
   const_iterator find_last(str const & substr, const_iterator whence) const;

   /*! Prints new contents into the string using io::text::ostream::print().

   @param fmt
      Format string.
   @param ts
      Replacement values.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   void format(str const & fmt, Ts const &... ts);
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   void format(str const & fmt);
   template <typename T0>
   void format(str const & fmt, T0 const & t0);
   template <typename T0, typename T1>
   void format(str const & fmt, T0 const & t0, T1 const & t1);
   template <typename T0, typename T1, typename T2>
   void format(str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2);
   template <typename T0, typename T1, typename T2, typename T3>
   void format(str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3);
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   void format(str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4);
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   void format(
      str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
      T5 const & t5
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
   void format(
      str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
      T5 const & t5, T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
   >
   void format(
      str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
      T5 const & t5, T6 const & t6, T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8
   >
   void format(
      str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
      T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9
   >
   void format(
      str const & fmt, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
      T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   );
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Converts a character index into its corresponding code point index.

   @param char_index
      Character index. No validation is performed on it.
   @return
      Code point index. If char_index is not a valid character index for the string, the return value is
      undefined.
   */
   std::size_t index_from_char_index(std::size_t char_index) const {
      return str_traits::size_in_codepoints(data(), data() + char_index);
   }

   /*! Inserts a character into the string at a specific offset.

   @param offset
      Offset at which to insert the character.
   @param ch
      Character to insert.
   */
   void insert(const_iterator offset, char_t ch) {
      insert(offset, &ch, 1);
   }

#if LOFTY_HOST_UTF > 8
   /*! Inserts an ASCII character into the string at a specific offset.

   @param offset
      Offset at which to insert the character.
   @param ch
      ASCII character to insert.
   */
   void insert(const_iterator offset, char ch) {
      insert(offset, host_char(ch));
   }
#endif

   /*! Inserts a code point into the string at a specific offset.

   @param offset
      Offset at which to insert the character.
   @param cp
      Code point to insert.
   */
   void insert(const_iterator offset, char32_t cp) {
      char_t cp_chars[host_char_traits::max_codepoint_length];
      insert(
         offset, cp_chars,
         static_cast<std::size_t>(host_char_traits::codepoint_to_chars(cp, cp_chars) - cp_chars)
      );
   }

   /*! Inserts characters from a string into the string at a specific offset.

   @param offset
      Offset at which to insert the string.
   @param src
      String to insert.
   */
   template <std::size_t src_embedded_capacity>
   void insert(const_iterator offset, sstr<src_embedded_capacity> const & src) {
      insert(offset, src.data(), src.size_in_chars());
   }

   /*! Inserts characters from a static string into the string at a specific offset.

   @param offset
      Offset at which to insert the string.
   @param src
      String to insert.
   */
   template <std::size_t src_size>
   void insert(const_iterator offset, char_t const (& src)[src_size]) {
      insert(offset, src, LOFTY_SL_SIZE(src));
   }

   /*! Inserts characters into the string at a specific character offset.

   @param offset
      Offset at which to insert the character.
   @param src
      Pointer to an array of characters to insert.
   @param src_char_size
      Count of characters in the array pointed to by src.
   */
   void insert(const_iterator offset, char_t const * src, std::size_t src_char_size) {
      vextr_impl::insert_remove(sizeof(char_t) * offset.char_index(), src, sizeof(char_t) * src_char_size, 0);
   }

   /*! Converts a character index into an iterator set to the corresponding code point.

   @param char_index
      Character index. No validation is performed on it.
   @return
      Iterator. If char_index is not a valid character index for the string, the return value is undefined.
   */
   const_iterator iterator_from_char_index(std::size_t char_index) const {
      return const_iterator(this, char_index);
   }

   /*! Converts a character index into an iterator set to the corresponding code point.

   @param char_index
      Character index. No validation is performed on it.
   @return
      Iterator. If char_index is not a valid character index for the string, the return value is undefined.
   */
   iterator iterator_from_char_index(std::size_t char_index) {
      return iterator(this, char_index);
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

   @param search
      Character to search for.
   @param replacement
      Character to replace search with.
   */
   void replace(char_t search, char_t replacement);

#if LOFTY_HOST_UTF > 8
   /*! Replaces all occurrences of an ASCII character with another ASCII character.

   @param search
      Character to search for.
   @param replacement
      Character to replace search with.
   */
   void replace(char search, char replacement) {
      replace(host_char(search), host_char(replacement));
   }
#endif

   /*! Replaces all occurrences of a code point with another code point.

   @param search
      Code point to search for.
   @param replacement
      Code point to replace search with.
   */
   void replace(char32_t search, char32_t replacement);

   /*! See vextr_impl::set_capacity().

   @param new_capacity_min
      Minimum count of characters requested.
   @param preserve
      If true, the previous contents of the string will be preserved even if the reallocation causes the
      string to switch to a different character array.
   */
   void set_capacity(std::size_t new_capacity_min, bool preserve) {
      vextr_impl::set_capacity(sizeof(char_t) * new_capacity_min, preserve);
   }

   /*! Expands the character array until the specified callback succeeds in filling it and returns a number of
   needed characters that’s less than the size of the buffer. For example, for chars_max == 3 (NUL terminator
   included), it must return <= 2 (NUL excluded).

   This method is not transaction-safe; if an exception is thrown in the callback or elsewhere, *this will not
   be restored to its previous state.

   TODO: maybe improve exception resilience? Check typical usage to see if it’s an issue.

   @param read_fn
      Callback that is invoked to fill up the string buffer.
      chars
         Pointer to the beginning of the buffer to be filled up by the callback.
      chars_max
         Size of the buffer pointed to by chars.
      return
         Count of characters written to the buffer pointed to by chars. If less than chars_max, this will be
         the final count of characters of *this; otherwise, read_fn will be called once more with a larger
         chars_max after the string buffer has been enlarged.
   */
   void set_from(_std::function<std::size_t (char_t * chars, std::size_t chars_max)> const & read_fn);

   /*! Changes the length of the string. If the string needs to be lengthened, the added characters will be
   left uninitialized.

   @param new_size
      New length of the string.
   @param clear_
      If true, the string will be cleared after being resized; if false, no characters will be changed.
   */
   void set_size_in_chars(std::size_t new_size, bool clear_ = false) {
      vextr_impl::set_size(sizeof(char_t) * new_size);
      if (clear_) {
         prepare_for_writing();
         memory::clear(data(), new_size);
      }
   }

   /*! Returns size of the string, in code points.

   @return
      Size of the string.
   */
   std::size_t size() const {
      return str_traits::size_in_codepoints(data(), data_end());
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

   /*! Returns a portion of the string from the specified iterator to the end of the string.

   @param substr_begin
      Iterator to the first character of the substring.
   @return
      Substring of *this.
   */
   str substr(const_iterator substr_begin) const {
      char_t const * substr_begin_ptr = validate_index_to_pointer(substr_begin.char_index_, true);
      return str(substr_begin_ptr, data_end());
   }

   /*! Returns a portion of the string.

   @param substr_begin
      Iterator to the first character of the substring.
   @param substr_end
      Iterator to past the end of the substring.
   @return
      Substring of *this.
   */
   str substr(const_iterator substr_begin, const_iterator substr_end) const {
      char_t const * substr_begin_ptr = validate_index_to_pointer(substr_begin.char_index_, true);
      char_t const * substr_end_ptr   = validate_index_to_pointer(substr_end  .char_index_, true);
      return str(substr_begin_ptr, substr_end_ptr);
   }

protected:
   /*! Constructor for subclasses with an embedded character array.

   @param embedded_capacity
      Size of the embedded character array, in bytes.
   */
   sstr(std::size_t embedded_capacity) :
      vextr_impl(embedded_capacity) {
   }

   /*! Move constructor for subclasses with an embedded character array.

   @param embedded_capacity
      Size of the embedded character array, in bytes.
   @param src
      Source object.
   */
   sstr(std::size_t embedded_capacity, str && src) :
      vextr_impl(embedded_capacity) {
      vextr_impl::assign_move_desc_or_move_items(_std::move(src));
   }

   /*! Copy constructor for subclasses with an embedded character array.

   @param embedded_capacity
      Size of the embedded character array, in bytes.
   @param src
      Source object.
   */
   sstr(std::size_t embedded_capacity, str const & src) :
      vextr_impl(embedded_capacity) {
      vextr_impl::assign_share_raw_or_copy_desc(src);
   }

   /*! Constructor from string literals for subclasses with an embedded character array.

   @param embedded_capacity
      Size of the embedded character array, in bytes.
   @param src
      Source NUL-terminated string literal.
   */
   template <std::size_t src_size>
   sstr(std::size_t embedded_capacity, char_t const (& src)[src_size]) :
      vextr_impl(embedded_capacity, &src[0], &src[LOFTY_SL_SIZE(src)], src[src_size - 1 /*NUL*/] == '\0') {
   }

   /*! Advances or backs up a character index by the specified number of code points, returning the resulting
   pointer. If the resulting index is outside the characters array, a collections::out_of_range exception will
   be thrown.

   @param char_index
      Initial index.
   @param delta
      Count of code points to move from char_index by.
   @param allow_end
      If true, end() will be considered a valid result; if false, it won’t.
   @return
      Resulting character index.
   */
   std::size_t advance_index_by_codepoint_delta(
      std::size_t char_index, std::ptrdiff_t delta, bool allow_end
   ) const;

   /*! Advances or backs up a character index by the specified number of code points, returning the resulting
   pointer. If the resulting index is outside the characters array, a collections::out_of_range exception will
   be thrown.

   @param this_str
      this.
   @param char_index
      Initial index.
   @param delta
      Count of code points to move from char_index by.
   @param allow_end
      If true, end() will be considered a valid result; if false, it won’t.
   @return
      Resulting character index.
   */
   static std::size_t advance_index_by_codepoint_delta(
      str const * this_str, std::size_t char_index, std::ptrdiff_t delta, bool allow_end
   );

   //! Prepares the character array to be modified.
   void prepare_for_writing();

   /*! Replaces a single character with another character.

   @param char_index
      Index of the first character making up the code point to replace.
   @param new_ch
      Character that will be written at char_index.
   */
   void replace_codepoint(std::size_t char_index, char_t new_ch);

#if LOFTY_HOST_UTF > 8
   /*! Replaces a single ASCII character with another ASCII character.

   @param char_index
      Index of the first character making up the code point to replace.
   @param new_ch
      Character that will be written at char_index.
   */
   void replace_codepoint(std::size_t char_index, char new_ch) {
      replace_codepoint(char_index, host_char(new_ch));
   }
#endif

   /*! Replaces a single code point with another code point.

   @param char_index
      Index of the first character making up the code point to replace.
   @param new_cp
      Code point that will be encoded starting at char_index.
   */
   void replace_codepoint(std::size_t char_index, char32_t new_cp);

   /*! Throws a collections::out_of_range if a character index is not within bounds.

   @param char_index
      Character index to validate.
   @param allow_end
      If true, data() + char_index == data_end() is allowed; if false, it’s not.
   @return
      Pointer to the character at char_index.
   */
   char_t const * validate_index_to_pointer(std::size_t char_index, bool allow_end) const;

   /*! Throws a collections::out_of_range if a character index is not within bounds of *this_str.

   This overload is static so that it will validate that this_str is not nullptr before dereferencing it.

   @param this_str
      this.
   @param char_index
      Character index to validate.
   @param allow_end
      If true, this_str->data() + char_index == this_str->data_end() is allowed; if false, it’s not.
   @return
      Pointer to the *this_str character at char_index.
   */
   static char_t const * validate_index_to_pointer(
      str const * this_str, std::size_t char_index, bool allow_end
   );
};

// General definition, with embedded character array.
template <std::size_t embedded_char_capacity>
class sstr : private str, private collections::_pvt::vextr_prefixed_array<char_t, embedded_char_capacity> {
private:
   using collections::_pvt::vextr_prefixed_array<char_t, embedded_char_capacity>::embedded_byte_capacity;

public:
   //! Default constructor.
   sstr() :
      text::str(embedded_byte_capacity) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   sstr(text::str && src) :
      text::str(embedded_byte_capacity, _std::move(src)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   template <std::size_t src_embedded_capacity>
   sstr(sstr<src_embedded_capacity> const & src) :
      text::str(embedded_byte_capacity, src.str()) {
   }

   /*! Constructor that will copy the source string literal.

   @param src
      Source NUL-terminated string literal.
   */
   template <std::size_t src_size>
   sstr(char_t const (& src)[src_size]) :
      text::str(embedded_byte_capacity, src) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   sstr & operator=(text::str && src) {
      text::str::operator=(_std::move(src));
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source string.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   sstr & operator=(sstr<src_embedded_capacity> const & src) {
      text::str::operator=(src.str());
      return *this;
   }

   /*! Assignment operator.

   @param src
      Source NUL-terminated string literal.
   @return
      *this.
   */
   template <std::size_t src_size>
   sstr & operator=(char_t const (& src)[src_size]) {
      text::str::operator=(src);
      return *this;
   }

   using text::str::operator[];
#ifdef LOFTY_CXX_EXPLICIT_CONVERSION_OPERATORS
   using text::str::operator bool;
#else
   using text::str::operator lofty::_pvt::explob_helper::bool_type;
#endif

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

#if LOFTY_HOST_UTF > 8
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

   @param cp
      Code point to append.
   @return
      *this.
   */
   sstr & operator+=(char32_t cp) {
      text::str::operator+=(cp);
      return *this;
   }

   /*! Concatenation-assignment operator.

   @param src
      String to append.
   @return
      *this.
   */
   template <std::size_t src_embedded_capacity>
   sstr & operator+=(sstr<src_embedded_capacity> const & src) {
      text::str::operator=(src);
      return *this;
   }

   using text::str::append;
   using text::str::begin;
   using text::str::c_str;
   using text::str::capacity;
   using text::str::cbegin;
   using text::str::cend;
   using text::str::clear;
   using text::str::crbegin;
   using text::str::crend;
   using text::str::data;
   using text::str::data_end;
   using text::str::encode;
   using text::str::end;
   using text::str::ends_with;
   using text::str::find;
   using text::str::find_last;
   using text::str::format;
   using text::str::index_from_char_index;
   using text::str::insert;
   using text::str::iterator_from_char_index;
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

   /*! Allows using the object as a lofty::text::str const instance.

   @return
      *this.
   */
   text::str const & str() const {
      return *this;
   }

   /*! Returns a pointer to the object as a lofty::text::str instance.

   @return
      this.
   */
   text::str * str_ptr() {
      return this;
   }

   using text::str::substr;
};

// Relational operators for str.
#define LOFTY_RELOP_IMPL(op) \
   template <std::size_t left_embedded_capacity, std::size_t right_embedded_capacity> \
   inline bool operator op( \
      sstr<left_embedded_capacity> const & left, sstr<right_embedded_capacity> const & right \
   ) { \
      return str_traits::compare(left.data(), left.data_end(), right.data(), right.data_end()) op 0; \
   } \
   template <std::size_t left_embedded_capacity, std::size_t right_size> \
   inline bool operator op(sstr<left_embedded_capacity> const & left, char_t const (& right)[right_size]) { \
      return str_traits::compare( \
         left.data(), left.data_end(), &right[0], &right[LOFTY_SL_SIZE(right)] \
      ) op 0; \
   } \
   template <std::size_t left_size, std::size_t right_embedded_capacity> \
   inline bool operator op(char_t const (& left)[left_size], sstr<right_embedded_capacity> const & right) { \
      return str_traits::compare(&left[0], &left[LOFTY_SL_SIZE(left)], right.data(), right.data_end()) op 0; \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

/* Relational operators for str::const_codepoint_proxy. Provided so that comparisons between char32_t (from
codepoint_proxy) and char{,8,16}_t don’t raise warnings. */
#define LOFTY_RELOP_IMPL(op) \
   inline bool operator op( \
      str::const_codepoint_proxy const & left, str::const_codepoint_proxy const & right \
   ) { \
      return left.operator char32_t() op right.operator char32_t(); \
   } \
   inline bool operator op(str::const_codepoint_proxy const & left, char_t right) { \
      return left.operator char32_t() op codepoint(right); \
   } \
   inline bool operator op(char_t left, str::const_codepoint_proxy const & right) { \
      return codepoint(left) op right.operator char32_t(); \
   } \
   inline bool operator op(str::const_codepoint_proxy const & left, char32_t right) { \
      return left.operator char32_t() op right; \
   } \
   inline bool operator op(char32_t left, str::const_codepoint_proxy const & right) { \
      return left op right.operator char32_t(); \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

#if LOFTY_HOST_UTF > 8
   #define LOFTY_RELOP_IMPL(op) \
      inline bool operator op(str::const_codepoint_proxy const & left, char right) { \
         return operator op(left, host_char(right)); \
      } \
      inline bool operator op(char left, str::const_codepoint_proxy const & right) { \
         return operator op(host_char(left), right); \
      }
   LOFTY_RELOP_IMPL(==)
   LOFTY_RELOP_IMPL(!=)
   LOFTY_RELOP_IMPL(>)
   LOFTY_RELOP_IMPL(>=)
   LOFTY_RELOP_IMPL(<)
   LOFTY_RELOP_IMPL(<=)
   #undef LOFTY_RELOP_IMPL
#endif

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline str operator+(sstr<left_embedded_capacity> const & left, sstr<right_embedded_capacity> const & right) {
   return str(left.data(), left.data_end(), right.data(), right.data_end());
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity, std::size_t right_embedded_capacity>
inline sstr<left_embedded_capacity> operator+(
   sstr<left_embedded_capacity> && left, sstr<right_embedded_capacity> const & right
) {
   left += right;
   return _std::move(left);
}

// Overloads taking a string or character literal as right operand.

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right NUL-terminated string literal operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity, std::size_t right_size>
inline sstr<left_embedded_capacity> operator+(
   sstr<left_embedded_capacity> && left, char_t const (& right)[right_size]
) {
   left += right;
   return _std::move(left);
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right NUL-terminated string literal operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity, std::size_t right_size>
inline str operator+(sstr<left_embedded_capacity> const & left, char_t const (& right)[right_size]) {
   return str(left.data(), left.data_end(), &right[0], &right[LOFTY_SL_SIZE(right)]);
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right character operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline sstr<left_embedded_capacity> operator+(sstr<left_embedded_capacity> && left, char_t right) {
   left += right;
   return _std::move(left);
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right character operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline str operator+(sstr<left_embedded_capacity> const & left, char_t right) {
   return str(left.data(), left.data_end(), &right, &right + 1);
}

#if LOFTY_HOST_UTF > 8

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right ASCII character operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline sstr<left_embedded_capacity> operator+(sstr<left_embedded_capacity> && left, char right) {
   return operator+(_std::move(left), host_char(right));
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right ASCII character operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline str operator+(sstr<left_embedded_capacity> const & left, char right) {
   return operator+(left, host_char(right));
}

#endif

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right code point operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline sstr<left_embedded_capacity> operator+(sstr<left_embedded_capacity> && left, char32_t right) {
   left += right;
   return _std::move(left);
}

/*! Concatenation operator.

@param left
   Left string operand.
@param right
   Right code point operand.
@return
   Resulting string.
*/
template <std::size_t left_embedded_capacity>
inline str operator+(sstr<left_embedded_capacity> const & left, char32_t right) {
   char_t right_chars[host_char_traits::max_codepoint_length];
   return str(
      left.data(), left.data_end(), right_chars, host_char_traits::codepoint_to_chars(right, right_chars)
   );
}

// Overloads taking a string or character literal as left operand.

/*! Concatenation operator.

@param left
   Left NUL-terminated string literal operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t left_size, std::size_t right_embedded_capacity>
inline sstr<right_embedded_capacity> operator+(
   char_t const (& left)[left_size], sstr<right_embedded_capacity> && right
) {
   right.insert(right.cbegin(), left);
   return _std::move(right);
}

/*! Concatenation operator.

@param left
   Left NUL-terminated string literal operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t left_size, std::size_t right_embedded_capacity>
inline str operator+(char_t const (& left)[left_size], sstr<right_embedded_capacity> const & right) {
   return str(&left[0], &left[LOFTY_SL_SIZE(left)], right.data(), right.data_end());
}

/*! Concatenation operator.

@param left
   Left character operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline sstr<right_embedded_capacity> operator+(char_t left, sstr<right_embedded_capacity> && right) {
   right.insert(right.cbegin(), left);
   return _std::move(right);
}

/*! Concatenation operator.

@param left
   Left character operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline str operator+(char_t left, sstr<right_embedded_capacity> const & right) {
   return str(&left, &left + 1, right.data(), right.data_end());
}

#if LOFTY_HOST_UTF > 8

/*! Concatenation operator.

@param left
   Left ASCII character operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline sstr<right_embedded_capacity> operator+(char left, sstr<right_embedded_capacity> && right) {
   return operator+(host_char(left), _std::move(right));
}

/*! Concatenation operator.

@param left
   Left ASCII character operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline str operator+(char left, sstr<right_embedded_capacity> const & right) {
   return operator+(host_char(left), right);
}

#endif

/*! Concatenation operator.

@param left
   Left code point operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline sstr<right_embedded_capacity> operator+(char32_t left, sstr<right_embedded_capacity> && right) {
   right.insert(right.cbegin(), left);
   return _std::move(right);
}

/*! Concatenation operator.

@param left
   Left code point operand.
@param right
   Right string operand.
@return
   Resulting string.
*/
template <std::size_t right_embedded_capacity>
inline str operator+(char32_t left, sstr<right_embedded_capacity> const & right) {
   char_t left_chars[host_char_traits::max_codepoint_length];
   return str(
      left_chars, host_char_traits::codepoint_to_chars(left, left_chars), right.data(), right.data_end()
   );
}

}} //namespace lofty::text

//! @cond
namespace std {

template <>
struct LOFTY_SYM hash<lofty::text::str> {
   std::size_t operator()(lofty::text::str const & s) const;
};

template <std::size_t embedded_capacity>
struct hash<lofty::text::sstr<embedded_capacity>> : public hash<lofty::text::str> {
};

} //namespace std
//! @endcond

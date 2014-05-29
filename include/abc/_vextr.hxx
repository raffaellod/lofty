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

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> instead of this file
#endif



/** DOC:4019 abc::*str and abc::*vector design

*str and *vector are implemented using the same base set of classes:

•  _raw_vextr_impl_base, core functionality for a vector of items: a little code and all member
   variables; this is then extended by three implementation classes:

   •  _raw_complex_vextr_impl, implementation of a vector of objects of non-trivial class: this is
      fully transactional and therefore exception-proof, but it's of course slower and uses more
      memory even during simpler operations;

   •  _raw_trivial_vextr_impl, implementation of a vector of plain values (instances of trivial
      class or native type): this is a near-optimal solution, still exception-proof but also taking
      advantage of the knowledge that no copy constructors need to be called; this class also
      supports the presence of a last element of value 0, opening up for the implementation of a
      string-like vector:

      •  str_base, implementation of a string: mostly based on _raw_trivial_vector_impl.

A vector/string using a static item array is nearly as fast as the C-style direct manipulation of an
array, only wasting a very small amount of space, and providing the ability to switch to a
dynamically-allocated item array on-the-fly in case the client needs to store in it more items than
are available.

Note: vextr is a silly portmanteau of vector and str(ing), because most of the above classes are
used by both.


Underlying data storage:

   Note: the third field is of type _raw_vextr_packed_data and is represented here as the tuple
   (capacity, array NUL-terminated?, array is dynamically-allocated?, statically array available?).


1. istr() or dmstr()
   ┌───┬───┬─────────┐
   │ p │ 0 │ 0|f|f|f │
   └───┴───┴─────────┘
     │
     ╰──────────────────▶ nullptr            No item array

2. smstr<5>()
   ┌───┬───┬─────────╥───┬───────────┐
   │ p │ 0 │ 0|f|f|t ║ 5 │ - - - - - │       Static (can be stack-allocated) fixed-size buffer
   └───┴───┴─────────╨───┴───────────┘
     │
     └──────────────────▶ nullptr            No item array

3. istr("abc")
   ┌───┬───┬─────────┐
   │ p │ 3 │ 0|t|f|f │
   └───┴───┴─────────┘
     │                   ┌──────────┐
     └──────────────────▶│ a b c \0 │        Read-only memory
                         └──────────┘
4. dmstr("abc")
   ┌───┬───┬─────────┐
   │ p │ 3 │ 8|f|t|f │
   └───┴───┴─────────┘
     │                   ┌─────────────────┐
     └──────────────────▶│ a b c - - - - - │ Dynamically-allocated variable-size buffer
                         └─────────────────┘
5. smstr<3>()
   ┌───┬───┬─────────╥───┬───────┐
   │ p │ 0 │ 3|f|f|t ║ 3 │ - - - │           Static (can be stack-allocated) fixed-size buffer
   └───┴───┴─────────╨───┴───────┘
     │
     └──────────────────▶ nullptr            No item array

5. smstr<3>() += "abc"
   ┌───┬───┬─────────╥───┬───────┐
   │ p │ 3 │ 3|f|f|t ║ 3 │ a b c │           Static (can be stack-allocated) fixed-size buffer
   └───┴───┴─────────╨───┴───────┘
     │                   ▲
     └───────────────────┘

6. smstr<2>() += "abc"
   ┌───┬───┬─────────╥───┬─────┐
   │ p │ 3 │ 8|f|t|t ║ 3 │ - - │             Static (can be stack-allocated) fixed-size buffer
   └───┴───┴─────────╨───┴─────┘
     │                   ┌─────────────────┐
     └──────────────────▶│ a b c - - - - - │ Dynamically-allocated variable-size buffer
                         └─────────────────┘


String types:

   istr (immutable string)
      Item array can be read-only (and shared) or dynamic.
   smstr (statically- or dynamically-allocated mutable string)
      Item array cannot be read-only nor shared, but it can be static or dynamic.
   dmstr (dynamically-allocated mutable string)
      Item array cannot be read-only, nor shared, nor static - always dynamic and writable.


Argument usage scenarios:

   istr           g_is;
   istr const     gc_is;
   dmstr          g_dms;
   dmstr const    gc_dms;
   smstr<n>       g_sms;
   smstr<n> const gc_sms;
   mstr           g_ms;
   mstr const     gc_ms;


•  No need to modify:

   void f1(istr const & isArg) {
      // N/A - const.
      isArg += "abc";

      // Share a read-only item array, or copy it: istr::operator=(istr const &)
      // Use assign_share_ro_or_copy().
      g_is = isArg;

      // TODO: validate these!
      // 1. Copy-construct: istr::istr(istr const &)
      //    Use assign_share_ro_or_copy(): will share a read-only item array, but will copy anything
      //    else. It's a copy - it's expected to have a separate life.
      // 2. Move-assign from the copy: istr::operator=(istr &&) (“nothrow”)
      //    Use assign_move().
      g_is = std::move(isArg);
      // 3. Destruct the now-empty copy: istr::~istr()

      // Copy the item array: mstr::operator=(istr const &)
      // Use assign_copy().
      g_ms = isArg;

      // TODO: validate these!
      // 1. Same as 1. above.
      // 2. Move-assign from the copy: dmstr::operator=(istr &&) (can throw)
      //    Use assign_move_dynamic_or_move_items(): will move a dynamic item array or move its
      //    items.
      g_ms = std::move(isArg);
      // 3. Same as 3. above.

      // Copy the item array: dmstr::operator=(istr const &)
      // Use assign_copy().
      g_dms = isArg;

      // TODO: validate these!
      // 1. Same as 1. above.
      // 2. Move-assign from the copy: dmstr::operator=(istr &&) (can throw)
      //    Use assign_move_dynamic_or_move_items(): will move a dynamic item array or move its
      //    items.
      g_dms = std::move(isArg);
      // 3. Same as 3. above.

      // Copy the item array: smstr<n>::operator=(istr const &)
      // Use assign_copy().
      g_sms = isArg;

      // TODO: validate these!
      // 1. Same as 1. above.
      // 2. Move-assign from the copy: smstr<n>::operator=(istr &&) (can throw)
      //    See considerations for 2. above.
      g_sms = std::move(isArg);
      // 3. Same as 3. above.
   }

   // 1. Construct a temporary object: istr::istr(char (& ach)[t_cch])
   f1("abc");
   // 2. Destruct the temporary object: istr::~istr()

   // Pass by const &.
   f1(g_is);
   f1(gc_is);

   // Invoke mstr::operator istr const &() const. Given that it's a REFERENCE, it's fine if the
   // source goes away and you get a crash: it's like freeing a pointer after passing it around.
   f1(g_ms);
   f1(gc_ms);

   // Invoke dmstr::operator istr const &() const. See considerations above.
   f1(g_dms);
   f1(gc_dms);

   // Invoke smstr<n>::operator istr const &() const. See considerations above.
   f1(g_sms);
   f1(gc_sms);


•  Writable dynamic string:

   void f2(dmstr * pdmsArg) {
      // Modify the buffer, maybe changing it for size reasons.
      *pdmsArg += "abc";

      // Copy the item array: istr::operator=(dmstr const &)
      // Use assign_copy(). Can never share, because dmstr never uses a read-only buffer.
      g_is = *pdmsArg;

      // Move the item array: istr::operator=(dmstr &&) (“nothrow”)
      // Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
      g_is = std::move(*pdmsArg);

      // Copy the item array: mstr::operator=(dmstr const &)
      // Use assign_copy().
      g_ms = *pdmsArg;

      // Move the item array: mstr::operator=(dmstr &&) (“nothrow”)
      // Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
      g_ms = std::move(*pdmsArg);

      // Copy the item array: dmstr::operator=(dmstr const &)
      // Use assign_copy().
      g_dms = *pdmsArg;

      // Move the item array: dmstr::operator=(dmstr &&) (“nothrow”)
      // Use assign_move(). “nothrow” because mdstr cannot be a smstr<n> under covers.
      g_dms = std::move(*pdmsArg);

      // Copy the item array: smstr<n>::operator=(dmstr const &)
      // Use assign_copy().
      g_sms = *pdmsArg;

      // Move the item array: smstr<n>::operator=(dmstr &&) (“nothrow”)
      // Use assign_move(). “nothrow” because dmstr cannot be a smstr<n> under covers.
      g_sms = std::move(*pdmsArg);
   }

   // N/A - no such conversion.
   f2("abc");
   f2(&g_is);
   f2(&gc_is);

   // N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions described
   // above cannot be guaranteed.
   f2(&g_ms);
   f2(&gc_ms);

   // Pass by &.
   f2(&g_dms);

   // N/A - const.
   f2(&gc_dms);

   // N/A - no such conversion. This must be the case, otherwise the “nothrow” conditions described
   // above cannot be guaranteed.
   f2(&g_sms);
   f2(&gc_sms);


•  Writable (static or dynamic) string:

   void f3(mstr * pmsArg) {
      // Modify the buffer, maybe changing it for size reasons.
      *pmsArg += "abc";

      // Copy the item array: istr::operator=(mstr const &)
      // Use assign_copy(): can never share, because mstr never uses a read-only buffer.
      g_is = *pmsArg;

      // Move the item array: istr::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_move_items(). can throw because mstr can be a smstr<n> under the
      // covers!
      g_is = std::move(*pmsArg);

      // Copy the item array: mstr::operator=(mstr const &)
      // Use assign_copy().
      g_ms = *pmsArg;

      // Move the item array: mstr::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_move_items(). See considerations above.
      // WARNING - this class has a throwing move constructor/assignment operator!
      g_ms = std::move(*pmsArg);

      // Copy the item array: dmstr::operator=(mstr const &)
      // Use assign_copy().
      g_dms = *pmsArg;

      // Move the item array: dmstr::operator=(mstr &&) (“nothrow”)
      // Use assign_move(). Can throw because mstr can be a smstr<n> under covers!
      g_dms = std::move(*pmsArg);

      // Copy the item array: smstr<n>::operator=(mstr const &)
      // Use assign_copy().
      g_sms = *pmsArg;

      // Move the item array: smstr<n>::operator=(mstr &&) (can throw)
      // Use assign_move_dynamic_or_move_items(): will move a dynamic item array or move its items.
      // else (like assign_copy()).
      g_sms = std::move(*pmsArg);
   }

   // N/A - no such conversion.
   f3("abc");
   f3(&g_is);
   f3(&gc_is);

   // Pass by &.
   f3(&g_ms);

   // N/A - const.
   f3(&gc_ms);

   // Down-cast to mstr &.
   f3(&g_dms);

   // N/A - const.
   f3(&gc_dms);

   // Down-cast to mstr &.
   f3(&g_sms);

   // N/A - const.
   f3(&gc_sms);


From the above, it emerges that:

•  mstr and smstr<n> cannot publicly derive from istr or dmstr, because that would enable automatic
   down-cast to i/dmstr &, which would then expose to the i/dmstr move constructor/assignment
   operator being invoked to move a static item array, which is wrong, or (if attempting to work
   around the move) would result in the static item array being copied, which would violate the
   “nothrow” requirement for the move constructor/assignment operator.

•  dmstr can publicly derive from mstr, with mstr being a base class for both dmstr and smstr<n>.

•  The only differences between istr and istr const & are:
   1. istr const & can be using a static item array (because it can be a smstr<n>), while any other
      istr will always use a const/read-only item array or a dynamic one;
   2. other string types can only be automatically converted to istr const &.

•  The difference between istr and mstr (and therefore dmstr/smstr<n>) is that the former can be
   constructed from a static string without copying it, but only offers read-only methods and
   operators; the latter offers the whole range of features one would expect, but will create a new
   item array upon construction or assignment (or use the embedded static one, in case of smstr<n>).

•  mstr cannot have a “nothrow” move constructor or assignment operator from itself, because the
   underlying objects might have static item arrays of different sizes. This isn't a huge deal-
   breaker because of the intended limited usage for mstr and smstr<n>.

The resulting class hierarchy is therefore:

   str_base (near-complete implementation of istr)
      istr
      mstr (near-complete implementation of dmstr/smstr<n>)
         dmstr
         smstr<n>

             ┌─────────────────────────────────────────────────────────┐
             │                     Functional need                     │
┌────────────┼──────────────┬─────────────────┬──────────┬─────────────┤
│            │ Local/member │ Method/function │ Writable │  Constant   │
│ Class      │ variable     │ argument        │          │ (read-only) │
├────────────┼──────────────┼─────────────────┼──────────┼─────────────┤
│ istr const │       x      │    x (const &)  │          │      x      │
│ mstr       │              │      x (*)      │     x    │             │
│ dmstr      │       x      │                 │     x    │             │
│ smstr      │       x      │                 │     x    │             │
└────────────┴──────────────┴─────────────────┴──────────┴─────────────┘
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_packed_data

namespace abc {

// Note: getters and setters in this class don’t follow the regular naming convention used
// everywhere else, to underline the fact this is just a group of member variables rather than a
// regular class.
class _raw_vextr_packed_data {
public:

   /** Constructor.

   ciMax
      Count of slots in the item array.
   bDynamic
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   bHasStatic
      true if the parent object is followed by a static item array, or false otherwise.
   */
   _raw_vextr_packed_data(size_t ciMax, bool bNulT, bool bDynamic, bool bHasStatic) :
      m_iPackedData(
         ciMax |
         (bNulT ? smc_bNulTMask : 0) |
         (bDynamic ? smc_bDynamicMask : 0) |
         (bHasStatic ? smc_bHasStaticMask : 0)
      ) {
   }


   /** Assignment operator. Updates all components except bHasStatic.

   rvpd
      Source data.
   return
      *this.
   */
   _raw_vextr_packed_data & operator=(_raw_vextr_packed_data const & rvpd) {
      m_iPackedData = (rvpd.m_iPackedData & ~smc_bHasStaticMask)
                    | (m_iPackedData & smc_bHasStaticMask);
      return *this;
   }


   /** Assigns new values to all components except bHasStatic.

   ciMax
      Count of slots in the item array.
   bNulT
      true if the item array ends in a NUL terminator, or false otherwise.
   bDynamic
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   return
      *this.
   */
   _raw_vextr_packed_data & set(size_t ciMax, bool bNulT, bool bDynamic) {
      m_iPackedData = ciMax
                    | (bNulT ? smc_bNulTMask : 0)
                    | (bDynamic ? smc_bDynamicMask : 0)
                    | (m_iPackedData & smc_bHasStaticMask);
      return *this;
   }


   /** Returns ciMax.

   return
      Count of slots in the item array.
   */
   size_t get_ciMax() const {
      return m_iPackedData & smc_ciMaxMask;
   }


   /** Returns true if the parent object’s m_p points to a dynamically-allocated item array.

   return
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   */
// bool is_item_array_dynamic() const {
   bool get_bDynamic() const {
      return (m_iPackedData & smc_bDynamicMask) != 0;
   }


   /** Returns true if the parent object is followed by a static item array.

   return
      true if the object also has a static item array, or false otherwise.
   */
// bool has_static_item_array() const {
   bool get_bHasStatic() const {
      return (m_iPackedData & smc_bHasStaticMask) != 0;
   }


   /** Returns true if the parent object’s m_p points to a NUL-terminated item array.

   return
      true if the item array ends in a NUL terminator, or false otherwise.
   */
// bool is_item_array_nul_terminated() const {
   bool get_bNulT() const {
      return (m_iPackedData & smc_bNulTMask) != 0;
   }


   /** Assigns a new value to ciMax.

   ciMax
      Count of slots in the item array.
   */
   void set_ciMax(size_t ciMax) {
      m_iPackedData = (m_iPackedData & ~smc_ciMaxMask) | ciMax;
   }


private:

   /** Bit-field composed by the following components:

   bool const bHasStatic
      true if the parent object is followed by a static item array.
   bool bDynamic
      true if the item array is allocated dynamically, or false otherwise (static or read-only).
   bool bNulT
      true if the item array is NUL-terminated.
   size_t ciMax;
      Size of the item array.
   */
   size_t m_iPackedData;

   /** Mask to access bHasStatic from m_iPackedData. */
   static size_t const smc_bHasStaticMask = 0x01;
   /** Mask to access bDynamic from m_iPackedData. */
   static size_t const smc_bDynamicMask = 0x02;
   /** Mask to access bNulT from m_iPackedData. */
   static size_t const smc_bNulTMask = 0x04;


public:

   /** Mask to access ciMax from m_iPackedData. */
   static size_t const smc_ciMaxMask = ~(smc_bNulTMask | smc_bDynamicMask | smc_bHasStaticMask);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_vextr_impl_base


namespace abc {

/** Template-independent members of _raw_*_vextr_impl that are identical for trivial and non-trivial
types.
*/
class ABCAPI _raw_vextr_impl_base {
protected:

   /** Allows to get a temporary item array from a pool of options, then work with it, and upon
   destruction it ensures that the array is either adopted by the associated _raw_vextr_impl_base,
   or properly discarded.

   A transaction will not take care of copying the item array, if switching to a different item
   array.

   For size increases, the reallocation (if any) is performed in the constructor; for decreases,
   it’s performed in commit().
   */
   class ABCAPI transaction :
      public noncopyable {
   public:

      /** Constructor.

      cbItem
         Size of a single array item, in bytes.
      prvib
         Subject of the transaction.
      ciNew
         New item count, or -1 to use ciDelta instead.
      ciDelta
         Item count change; can be positive or negative.
      */
      transaction(
         size_t cbItem, _raw_vextr_impl_base * prvib, ptrdiff_t ciNew, ptrdiff_t ciDelta = 0
      );


      /** Destructor.
      */
      ~transaction() {
         if (m_bFree) {
            memory::_raw_free(m_pBegin);
         }
      }


      /** Commits the transaction; if the item array is to be replaced, the current one will be
      released if necessary; it’s up to the client to destruct any items in it. If this method is
      not called before the transaction is destructed, it’s up to the client to also ensure that any
      and all objects constructed in the work array have been properly destructed.
      */
      void commit();


      /** Returns the work item array.

      return
         Pointer to the working item array.
      */
      template <typename T>
      T * work_array() const {
         return static_cast<T *>(m_pBegin);
      }


      /** Returns true if the contents of the item array need to migrated due to the transaction
      switching item arrays. If the array was/will be only resized, the return value is false,
      because the reallocation did/will take care of moving the item array.

      return
         true if the pointer to the item array will be changed upon destruction, or false otherwise.
      */
      bool will_replace_item_array() const {
         return m_pBegin != m_prvib->m_pBegin;
      }


   private:

      /** See _raw_vextr_impl_base::m_rvpd. */
      _raw_vextr_packed_data m_rvpd;
      /** Subject of the transaction. */
      _raw_vextr_impl_base * m_prvib;
      /** Pointer to the item array to which clients must write. This may or may not be the same as
      m_prvib->m_pBegin, depending on whether we needed a new item array. This pointer will replace
      m_prvib->m_pBegin upon commit(). */
      void * m_pBegin;
      /** Similar to m_pBegin, but for m_prvib->m_pEnd. */
      void * m_pEnd;
      /** true if m_pBegin has been dynamically allocated for the transaction and needs to be freed
      in the destructor, either because the transaction didn’t get committed, or because it did and
      the item array is now owned by m_prvib. */
      bool m_bFree;
   };

   // Allow transactions to access this class’s protected members.
   friend class transaction;


public:

   /** Destructor.
   */
   ~_raw_vextr_impl_base() {
      if (m_rvpd.get_bDynamic()) {
         memory::_raw_free(m_pBegin);
      }
   }


   /** Returns a pointer to the start of the item array.

   return
      Pointer to the first element.
   */
   template <typename T>
   T * begin() {
      return static_cast<T *>(m_pBegin);
   }
   template <typename T>
   T const * begin() const {
      return static_cast<T const *>(m_pBegin);
   }


   /** Returns the count of item slots in the current item array.

   return
      Count of slots in the item array.
   */
   size_t capacity() const {
      return m_rvpd.get_ciMax();
   }


   /** Returns a pointer to the end of the item array.

   return
      Pointer to beyond the last element.
   */
   template <typename T>
   T * end() {
      return static_cast<T *>(m_pEnd);
   }
   template <typename T>
   T const * end() const {
      return static_cast<T const *>(m_pEnd);
   }


   /** Returns the count of items in the item array.

   return
      Size of the item array.
   */
   template <typename T>
   size_t size() const {
      return size_t(end<T>() - begin<T>());
   }


protected:

   /** Constructor. The overload with ciStaticMax constructs the object as empty, setting
   m_pBegin/End to nullptr; the overload with pConstSrcBegin/End constructs the object assigning an
   item array.

   ciStaticMax
      Count of slots in the static item array, or 0 if no static item array is present.
   pConstSrcBegin
      Pointer to the start of an array that will be adopted by the vextr as read-only.
   pConstSrcEnd
      Pointer to the end of the array.
   bNulT
      true if the array pointed to by pConstSrc is a NUL-terminated string, or false otherwise.
   */
   _raw_vextr_impl_base(size_t ciStaticMax);
   _raw_vextr_impl_base(
      void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false
   ) :
      m_pBegin(const_cast<void *>(pConstSrcBegin)),
      m_pEnd(const_cast<void *>(pConstSrcEnd)),
      // ciMax = 0 means that the item array is read-only.
      m_rvpd(0, bNulT, false, false) {
   }


   /** Resets the contents of the object to nullptr.
   */
   void assign_empty() {
      m_pBegin = m_pEnd = nullptr;
      m_rvpd.set(0, false, false);
   }


   /** Returns true if m_p points to a read-only item array.

   return
      true if m_p points to a read-only item array, or false otherwise.
   */
   bool is_item_array_readonly() const {
      // No capacity means read-only item array.
      return m_rvpd.get_ciMax() == 0;
   }


   /** Returns a pointer to the static item array that follows this object, if present, or nullptr
   otherwise.

   return
      Pointer to the static item array, or nullptr if no static item array is present.
   */
   template <typename T>
   T * static_array_ptr();


   /** Returns the size of the array returned by static_array_ptr(), or 0 if no such array is
   present.

   return
      Capacity of the static item array, or 0 if no static item array is present.
   */
   size_t static_capacity() const;


   /** Converts a possibly negative item byte offset into a pointer into the item array, throwing an
   exception if the result is out of bounds for the item array.

   ib
      If positive, this is interpreted as a 0-based byte offset; if negative, it’s interpreted as a
      1-based byte offset from the end of the item array by adding this->size<int8_t>() to it.
   return
      Pointer to the item.
   */
   void const * translate_offset(intptr_t ib) const;


   /** Converts a left-closed, right-open interval with possibly negative byte offsets into one
   consisting of two pointers into the item array.

   ibBegin
      Left endpoint of the interval, inclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<int8_t>() to it.
   ibEnd
      Right endpoint of the interval, exclusive. If positive, this is interpreted as a 0-based byte
      offset; if negative, it’s interpreted as a 1-based byte offset from the end of the item array
      by adding this->size<int8_t>() to it.
   return
      Left-closed, right-open interval such that return.first <= i < return.second, or the empty
      interval [nullptr, nullptr) if the offsets represent an empty interval after being adjusted.
   */
   std::pair<void const *, void const *> translate_byte_range(
      intptr_t ibBegin, intptr_t ibEnd
   ) const;


protected:

   /** Pointer to the start of the item array. */
   void * m_pBegin;
   /** Pointer to the end of the item array. */
   void * m_pEnd;
   /** Size of the item array pointed to by m_pBegin, and other bits. */
   _raw_vextr_packed_data m_rvpd;

   /** No less than this many items. Must be greater than, and not overlap any bits with,
   _raw_vextr_impl_base::smc_ciMaxMask. */
   static size_t const smc_cMinSlots = 8;
   /** Size multiplier. This should take into account that we want to reallocate as rarely as
   possible, so every time we do it it should be for a rather conspicuous growth. */
   static unsigned const smc_iGrowthRate = 2;
};


/** Used to find out what the offset are for an embedded static item array.
*/
class _raw_vextr_impl_base_with_static_item_array :
   public _raw_vextr_impl_base {
public:

   /** Static size. */
   size_t m_ciStaticMax;
   /** First item of the static array. This can’t be a T[], because we don’t want its items to be
   constructed/destructed automatically, and because this class doesn’t know its size. */
   std::max_align_t m_tFirst;
};


/** Rounds up an array size to avoid interfering with the bits outside of
_raw_vextr_packed_data::smc_ciMaxMask. Should be a constexpr function, but for now it’s just a
macro.

ci
   Count of items.
return
   Rounded-up count of items.
*/
#define _ABC__RAW_VEXTR_IMPL_BASE__ADJUST_ITEM_COUNT(ci) \
   ((size_t(ci) + ~_raw_vextr_packed_data::smc_ciMaxMask) & _raw_vextr_packed_data::smc_ciMaxMask)


// Now these can be implemented.

template <typename T>
inline T * _raw_vextr_impl_base::static_array_ptr() {
   if (!m_rvpd.get_bHasStatic()) {
      return nullptr;
   }
   _raw_vextr_impl_base_with_static_item_array * prvibwsia(
      static_cast<_raw_vextr_impl_base_with_static_item_array *>(this)
   );
   return reinterpret_cast<T *>(&prvibwsia->m_tFirst);
}


inline size_t _raw_vextr_impl_base::static_capacity() const {
   if (!m_rvpd.get_bHasStatic()) {
      return 0;
   }
   _raw_vextr_impl_base_with_static_item_array const * prvibwsia(
      static_cast<_raw_vextr_impl_base_with_static_item_array const *>(this)
   );
   return prvibwsia->m_ciStaticMax;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_complex_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for non-trivial contained types.
*/
class ABCAPI _raw_complex_vextr_impl :
   public _raw_vextr_impl_base {
public:

   /** Appends one or more items.

   type
      Adapter for the items’ type.
   p
      Pointer to the first item to add.
   ci
      Count of items in the array pointed to by p.
   bMove
      true to move the items from pAdd to the vextr’s item array, or false to copy them instead.
   */
   void append(type_void_adapter const & type, void const * p, size_t ci, bool bMove) {
      if (ci) {
         _insert(type, size<int8_t>() / type.cb, p, ci, bMove);
      }
   }


   /** Copies or moves the contents of the two sources to *this, according to the source type. If
   bMove{1,2} == true, the source items will be moved by having their const-ness cast away ‒ be
   careful.

   type
      Adapter for the items’ type.
   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   bMove1
      true to move the items from the first source array to the vextr’s item array, or false to
      copy them instead.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   bMove2
      true to move the items from the second source array to the vextr’s item array, or false to
      copy them instead.
   */
   void assign_concat(
      type_void_adapter const & type,
      void const * p1Begin, void const * p1End, bool bMove1,
      void const * p2Begin, void const * p2End, bool bMove2
   );


   /** Copies the contents of the source to *this.

   type
      Adapter for the items’ type.
   pBegin
      Pointer to the start of the source array.
   pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(type_void_adapter const & type, void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      // assign_concat() is fast enough. Pass the source as the second argument pair, because its
      // code path is faster.
      assign_concat(type, nullptr, nullptr, false, pBegin, pEnd, false);
   }


   /** Moves the contents of the source to *this, taking ownership of the whole item array (items
   are not moved nor copied).

   type
      Adapter for the items’ type.
   rcvi
      Source vextr.
   */
   void assign_move(type_void_adapter const & type, _raw_complex_vextr_impl && rcvi);


   /** Moves the source’s item array if dynamically-allocated, else copies it to *this, moving the
   items instead.

   type
      Adapter for the items’ type.
   rcvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(
      type_void_adapter const & type, _raw_complex_vextr_impl && rcvi
   );


   /** Destructs the item array. It does not deallocate the item array.

   type
      Adapter for the items’ type.
   */
   void destruct_items(type_void_adapter const & type) {
      type.destruct(m_pBegin, m_pEnd);
   }


   /** Inserts elements at a specific position in the vextr.

   type
      Adapter for the items’ type.
   iOffset
      Index at which the items should be inserted. See abc::_raw_vextr_impl_base::translate_offset()
      for allowed index values.
   p
      Pointer to the first item to add.
   ci
      Count of items in the array pointed to by p.
   bMove
      true to move the items from pAdd to the vextr’s item array, or false to copy them instead.
   */
   void insert(
      type_void_adapter const & type, intptr_t iOffset, void const * p, size_t ci, bool bMove
   ) {
      if (ci) {
         _insert(type, uintptr_t(static_cast<int8_t const *>(
            translate_offset(ptrdiff_t(type.cb) * iOffset)
         ) - begin<int8_t>()) / type.cb, p, ci, bMove);
      }
   }


   /** Removes a single element from the vextr.

   type
      Adapter for the items’ type.
   i
      Index of the element. See abc::_raw_vextr_impl_base::translate_offset() for allowed index
      values.
   */
   void remove_at(type_void_adapter const & type, intptr_t i) {
      _remove(type, uintptr_t(static_cast<int8_t const *>(
         translate_offset(ptrdiff_t(type.cb) * i)
      ) - begin<int8_t>()) / type.cb, 1);
   }


   /** Removes elements from the vextr.

   type
      Adapter for the items’ type.
   iBegin
      Index of the first element. See abc::_raw_vextr_impl_base::translate_byte_range() for allowed
      begin index values.
   iEnd
      Index of the last element, exclusive. See abc::_raw_vextr_impl_base::translate_byte_range()
      for allowed end index values.
   */
   void remove_range(type_void_adapter const & type, intptr_t iBegin, intptr_t iEnd);


   /** Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   type
      Adapter for the items’ type.
   ciMin
      Minimum count of items requested.
   bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(type_void_adapter const & type, size_t ciMin, bool bPreserve);


   /** Changes the count of items in the vector. If the new item count is greater than the current
   one, the added elements will be left uninitialized; it’s up to the caller to make sure that these
   elements are properly constructed, or problems will arise when the destructor will attempt to
   destruct these elements.

   type
      Adapter for the items’ type.
   ci
      New count of items.
   */
   void set_size(type_void_adapter const & type, size_t ci);


protected:

   /** Constructor. See _raw_vextr_impl_base::_raw_vextr_impl_base().
   */
   _raw_complex_vextr_impl(size_t ciStaticMax) :
      _raw_vextr_impl_base(ciStaticMax) {
   }
   _raw_complex_vextr_impl(void const * pConstSrcBegin, void const * pConstSrcEnd) :
      _raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd) {
   }


private:

   /** Implementation of append() and insert(). Does not validate iOffset or ci.

   type
      Adapter for the items’ type.
   iOffset
      Index at which the items should be inserted.
   p
      Pointer to the first item to add.
   ci
      Count of items in the array pointed to by p.
   bMove
      true to move the items from pAdd to the vextr’s item array, or false to copy them instead.
   */
   void _insert(
      type_void_adapter const & type, uintptr_t iOffset, void const * p, size_t ci, bool bMove
   );


   /** Implementation of remove_at() and remove_range(). Does not validate iOffset or ciRemove.

   type
      Adapter for the items’ type.
   iOffset
      Index at which the items should be removed.
   ciRemove
      Count of items to remove.
   */
   void _remove(type_void_adapter const & type, uintptr_t iOffset, size_t ciRemove);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_raw_trivial_vextr_impl


namespace abc {

/** Template-independent implementation of a vector for trivial contained types. This is the most
derived common base class of both vector and str.
*/
class ABCAPI _raw_trivial_vextr_impl :
   public _raw_vextr_impl_base {
public:

   /** Appends one or more items.

   cbItem
      Size of a single array item, in bytes.
   p
      Pointer to the first item to add.
   ci
      Count of items in the array pointed to by p.
   */
   void append(size_t cbItem, void const * p, size_t ci) {
      if (ci) {
         _insert_or_remove(cbItem, size<int8_t>() / cbItem, p, ci, 0);
      }
   }


   /** Copies the contents of the two sources to *this. This method must never be called with p1 or
   p2 == m_pBegin.

   cbItem
      Size of a single array item, in bytes.
   p1Begin
      Pointer to the start of the first source array.
   p1End
      Pointer to the end of the first source array.
   p2Begin
      Pointer to the start of the second source array.
   p2End
      Pointer to the end of the second source array.
   */
   void assign_concat(
      size_t cbItem,
      void const * p1Begin, void const * p1End, void const * p2Begin, void const * p2End
   );


   /** Copies the contents of the source array to *this.

   cbItem
      Size of a single array item, in bytes.
   pBegin
      Pointer to the start of the source array.
   pEnd
      Pointer to the end of the source array.
   */
   void assign_copy(size_t cbItem, void const * pBegin, void const * pEnd) {
      if (pBegin == m_pBegin) {
         return;
      }
      // assign_concat() is fast enough. Pass the source as the second argument pair, because its
      // code path is faster.
      assign_concat(cbItem, nullptr, nullptr, pBegin, pEnd);
   }


   /** Moves the source’s item array to *this. This must be called with rtvi being in control of a
   read-only or dynamic item array; see [DOC:4019 abc::*str and abc::*vector design] to see how str
   and vector ensure this.

   rtvi
      Source vextr.
   */
   void assign_move(_raw_trivial_vextr_impl && rtvi) {
      if (rtvi.m_pBegin == m_pBegin) {
         return;
      }
      // Share the item array.
      _assign_share(rtvi);
      // And now empty the source.
      rtvi.assign_empty();
   }


   /** Moves the source’s item array if dynamically-allocated, else copies its items (not move –
   items are trivial) to *this.

   cbItem
      Size of a single array item, in bytes.
   rtvi
      Source vextr.
   */
   void assign_move_dynamic_or_move_items(size_t cbItem, _raw_trivial_vextr_impl && rtvi);


   /** Shares the source’s item array if read-only, else copies it to *this.

   cbItem
      Size of a single array item, in bytes.
   rtvi
      Source vextr.
   */
   void assign_share_ro_or_copy(size_t cbItem, _raw_trivial_vextr_impl const & rtvi) {
      if (rtvi.m_pBegin == m_pBegin) {
         return;
      }
      if (rtvi.is_item_array_readonly()) {
         _assign_share(rtvi);
      } else {
         // Non-read-only, cannot share.
         assign_copy(cbItem, rtvi.m_pBegin, rtvi.m_pEnd);
      }
   }


   /** Inserts elements at a specific position in the vextr.

   cbItem
      Size of a single array item, in bytes.
   iOffset
      Index at which the items should be inserted. See abc::_raw_vextr_impl_base::translate_offset()
      for allowed index values.
   p
      Pointer to the first item to add.
   ci
      Count of items in the array pointed to by p.
   */
   void insert(size_t cbItem, intptr_t iOffset, void const * p, size_t ci) {
      if (ci) {
         _insert_or_remove(cbItem, uintptr_t(static_cast<int8_t const *>(
            translate_offset(ptrdiff_t(cbItem) * iOffset)
         ) - begin<int8_t>()) / cbItem, p, ci, 0);
      }
   }


   /** Removes a single element from the vextr.

   cbItem
      Size of a single array item, in bytes.
   i
      Index of the element. See abc::_raw_vextr_impl_base::translate_offset() for allowed index
      values.
   */
   void remove_at(size_t cbItem, intptr_t i) {
      _insert_or_remove(cbItem, uintptr_t(static_cast<int8_t const *>(
         translate_offset(ptrdiff_t(cbItem) * i)
      ) - begin<int8_t>()) / cbItem, nullptr, 0, 1);
   }


   /** Removes elements from the vextr.

   cbItem
      Size of a single array item, in bytes.
   iBegin
      Index of the first element. See abc::_raw_vextr_impl_base::translate_byte_range() for allowed
      begin index values.
   iEnd
      Index of the last element, exclusive. See abc::_raw_vextr_impl_base::translate_byte_range()
      for allowed end index values.
   */
   void remove_range(size_t cbItem, intptr_t iBegin, intptr_t iEnd);


   /** Ensures that the item array has at least ciMin of actual item space. If this causes *this to
   switch to using a different item array, any data in the current one will be lost unless bPreserve
   == true.

   cbItem
      Size of a single array item, in bytes.
   ciMin
      Minimum count of items requested.
   bPreserve
      If true, the previous contents of the item array will be preserved even if the reallocation
      causes the vextr to switch to a different item array.
   */
   void set_capacity(size_t cbItem, size_t ciMin, bool bPreserve);


   /** Changes the count of elements in the vector. If the item array needs to be lengthened, the
   added elements will be left uninitialized.

   cbItem
      Size of a single array item, in bytes.
   ci
      New count of items.
   */
   void set_size(size_t cbItem, size_t ci);


protected:

   /** Constructor. See _raw_vextr_impl_base::_raw_vextr_impl_base().
   */
   _raw_trivial_vextr_impl(size_t ciStaticMax) :
      _raw_vextr_impl_base(ciStaticMax) {
   }
   _raw_trivial_vextr_impl(
      void const * pConstSrcBegin, void const * pConstSrcEnd, bool bNulT = false
   ) :
      _raw_vextr_impl_base(pConstSrcBegin, pConstSrcEnd, bNulT) {
   }


private:

   /** Shares the source’s item array. It only allows sharing read-only or dynamically-allocated
   item arrays (the latter only as part of moving them).

   rtvi
      Source vextr.
   */
   void _assign_share(_raw_trivial_vextr_impl const & rtvi);


   /** Implementation of append(), insert(), remove_at() and remove_range().

   cbItem
      Size of a single array item, in bytes.
   iOffset
      Index at which the items should be inserted.
   pAdd
      Pointer to the first item to add.
   ciAdd
      Count of items to add.
   ciRemove
      Count of items to remove.
   */
   void _insert_or_remove(
      size_t cbItem, uintptr_t iOffset, void const * pAdd, size_t ciAdd, size_t ciRemove
   );
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


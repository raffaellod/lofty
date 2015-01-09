/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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
// abc::detail::xor_list_iterator_impl

namespace abc {
namespace detail {

//! Iterator for XOR doubly-linked list node classes.
template <typename TIterator, typename TNode, typename TValue>
class xor_list_iterator_impl : public std::iterator<std::bidirectional_iterator_tag, TValue> {
public:
   /*! Constructor.

   @param pnPrev
      Pointer to the node preceding *pnCurr.
   @param pnCurr
      Pointer to the current node.
   @param pnNext
      Pointer to the node following *pnCurr.
   */
   xor_list_iterator_impl(
      xor_list_node_impl * pnPrev, xor_list_node_impl * pnCurr, xor_list_node_impl * pnNext
   ) :
      m_pnPrev(static_cast<TNode *>(pnPrev)),
      m_pnCurr(static_cast<TNode *>(pnCurr)),
      m_pnNext(static_cast<TNode *>(pnNext)) {
   }

   /*! Dereferencing operator.

   @return
      Reference to the current node.
   */
   TValue & operator*() const {
      return *m_pnCurr->value_ptr();
   }

   /*! Dereferencing member access operator.

   @return
      Pointer to the current node.
   */
   TValue * operator->() const {
      return m_pnCurr->value_ptr();
   }

   /*! Preincrement operator.

   @return
      *this after it’s moved to the node following the one currently pointed to by.
   */
   TIterator & operator++() {
      m_pnPrev = m_pnCurr;
      m_pnCurr = m_pnNext;
      m_pnNext = m_pnCurr ? static_cast<TNode *>(m_pnCurr->get_next(m_pnPrev)) : nullptr;
      return *static_cast<TIterator *>(this);
   }

   /*! Postincrement operator.

   @return
      Iterator pointing to the node following the one pointed to by this iterator.
   */
   TIterator operator++(int) {
      TNode * pnPrevPrev = m_pnPrev;
      operator++();
      return TIterator(pnPrevPrev, m_pnPrev, m_pnCurr);
   }

   /*! Predecrement operator.

   @return
      *this after it’s moved to the node preceding the one currently pointed to by.
   */
   TIterator & operator--() {
      m_pnNext = m_pnCurr;
      m_pnCurr = m_pnPrev;
      m_pnPrev = m_pnCurr ? static_cast<TNode *>(m_pnCurr->get_prev(m_pnNext)) : nullptr;
      return *static_cast<TIterator *>(this);
   }

   /*! Postdecrement operator.

   @return
      Iterator pointing to the node preceding the one pointed to by this iterator.
   */
   TIterator operator--(int) {
      TNode * pnNextNext = m_pnNext;
      operator--();
      return TIterator(m_pnCurr, m_pnNext, pnNextNext);
   }

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   template <typename TIterator2> \
   bool operator op( \
      xor_list_iterator_impl<TIterator2, TNode, typename std::add_const<TValue>::type> const & it \
   ) const { \
      return base() op it.base(); \
   } \
   template <typename TIterator2> \
   bool operator op( \
      xor_list_iterator_impl<TIterator2, TNode, \
      typename std::remove_const<TValue>::type> const & it \
   ) const { \
      return base() op it.base(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL

   /*! Returns the underlying pointer to the node.

   @return
      Pointer to the current node.
   */
   TNode const * base() const {
      return m_pnCurr;
   }

protected:
   //! Pointer to the previous node.
   TNode * m_pnPrev;
   //! Pointer to the current node.
   TNode * m_pnCurr;
   //! Pointer to the next node.
   TNode * m_pnNext;
};

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

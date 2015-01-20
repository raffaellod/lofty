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
// abc::collections::detail::xor_list_node_impl

namespace abc {
namespace collections {
namespace detail {

//! Node for XOR doubly-linked list classes.
class xor_list_node_impl {
public:
   //! Constructor.
   xor_list_node_impl() {
   }
   xor_list_node_impl(xor_list_node_impl const &) {
      // Skip copying the source’s links.
   }

   /*! Assignment operator.

   @return
      *this.
   */
   xor_list_node_impl & operator=(xor_list_node_impl const &) {
      // Skip copying the source’s links.
      return *this;
   }

   /*! Returns a pointer to the next node.

   @param pnPrev
      Pointer to the previous node.
   */
   xor_list_node_impl * get_next(xor_list_node_impl * pnPrev) {
      return reinterpret_cast<xor_list_node_impl *>(
         m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnPrev)
      );
   }

   /*! Returns a pointer to the previous node.

   @param pnNext
      Pointer to the next node.
   */
   xor_list_node_impl * get_prev(xor_list_node_impl * pnNext) {
      return reinterpret_cast<xor_list_node_impl *>(
         m_iPrevXorNext ^ reinterpret_cast<std::uintptr_t>(pnNext)
      );
   }

   /*! Updates the previous/next pointer.

   @param pnPrev
      Pointer to the previous node.
   @param pnNext
      Pointer to the next node.
   */
   void set_prev_next(xor_list_node_impl * pnPrev, xor_list_node_impl * pnNext) {
      m_iPrevXorNext = reinterpret_cast<std::uintptr_t>(pnPrev) ^
                       reinterpret_cast<std::uintptr_t>(pnNext);
   }

private:
   //! Pointer to the previous node XOR pointer to the next node.
   std::uintptr_t m_iPrevXorNext;
};

} //namespace detail
} //namespace collections
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

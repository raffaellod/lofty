/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#include <abaclade.hxx>
#include <abaclade/map.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::detail::map_impl

namespace abc {
namespace detail {

map_impl::map_impl() :
   m_cBuckets(0),
   m_cUsedBuckets(0) {
}
map_impl::map_impl(map_impl && m) :
   m_piHashes(std::move(m.m_piHashes)),
   m_pKeys(std::move(m.m_pKeys)),
   m_pValues(std::move(m.m_pValues)),
   m_cBuckets(m.m_cBuckets),
   m_cUsedBuckets(m.m_cUsedBuckets) {
   m.m_cBuckets = 0;
   m.m_cUsedBuckets = 0;
}

map_impl::~map_impl() {
}

map_impl & map_impl::operator=(map_impl && m) {
   m_piHashes = std::move(m.m_piHashes);
   m_pKeys = std::move(m.m_pKeys);
   m_pValues = std::move(m.m_pValues);
   m_cBuckets = m.m_cBuckets;
   m.m_cBuckets = 0;
   m_cUsedBuckets = m.m_cUsedBuckets;
   m.m_cUsedBuckets = 0;
   return *this;
}

std::size_t map_impl::find_bucket_movable_to_empty(std::size_t iEmptyBucket) const {
   std::size_t cNeighborhoodBuckets = neighborhood_size();
   std::size_t cBucketsRightOfEmpty = cNeighborhoodBuckets - 1;
   // Ensure that iEmptyBucket will be on the right of any of the buckets we’re going to check.
   if (iEmptyBucket < cBucketsRightOfEmpty) {
      iEmptyBucket += m_cBuckets;
   }
   // Calculate the bucket index range of the neighborhood that ends with iEmptyBucket.
   std::size_t const * piEmptyHash = m_piHashes.get() + (iEmptyBucket & (m_cBuckets - 1)),
                     * piHash      = piEmptyHash - cBucketsRightOfEmpty,
                     * piHashesEnd = m_piHashes.get() + m_cBuckets;
   /* The neighborhood may wrap, so we can only test for inequality and rely on the wrap-around
   logic at the end of the loop body. */
   while (piHash != piEmptyHash) {
      /* Get the end of the original neighborhood for the key in this bucket; if the empty bucket is
      within that index, the contents of this bucket can be moved to the empty one. */
      std::size_t iCurrNhEnd = hash_neighborhood_index(*piHash) + cNeighborhoodBuckets;
      /* Both indices are allowed to be >m_cBuckets (see earlier if), so this comparison is always
      valid. */
      if (iEmptyBucket < iCurrNhEnd) {
         return static_cast<std::size_t>(piHash - m_piHashes.get());
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++piHash == piHashesEnd) {
         piHash = m_piHashes.get();
      }
   }
   // No luck, the hash table needs to be resized.
   return smc_iNullIndex;
}

std::tuple<std::size_t, std::size_t> map_impl::hash_neighborhood_range(std::size_t iHash) const {
   std::size_t iNhBegin = hash_neighborhood_index(iHash);
   std::size_t iNhEnd = iNhBegin + neighborhood_size();
   // Wrap the end index back in the table.
   iNhEnd &= m_cBuckets - 1;
   return std::make_tuple(iNhBegin, iNhEnd);
}

std::size_t map_impl::neighborhood_size() const {
   // Can’t have a neighborhood larger than the total count of buckets.
   return m_cBuckets < smc_cNeighborhoodBuckets ? m_cBuckets : smc_cNeighborhoodBuckets;
}

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

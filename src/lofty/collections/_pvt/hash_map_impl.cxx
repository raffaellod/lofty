/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/_pvt/hash_map_impl.hxx>
#include <lofty/type_void_adapter.hxx>

#include <climits> // CHAR_BIT


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace collections { namespace _pvt {

std::size_t const hash_map_impl::ideal_neighborhood_size = sizeof(std::size_t) * CHAR_BIT / 8;

hash_map_impl::hash_map_impl() :
   total_buckets(0),
   used_buckets(0),
   neighborhood_size_(0),
   rev(0) {
}
hash_map_impl::hash_map_impl(hash_map_impl && src) :
   hashes(_std::move(src.hashes)),
   keys(_std::move(src.keys)),
   values(_std::move(src.values)),
   total_buckets(src.total_buckets),
   used_buckets(src.used_buckets),
   neighborhood_size_(src.neighborhood_size_),
   rev(0) {
   LOFTY_TRACE_FUNC(this);

   src.total_buckets = 0;
   src.used_buckets = 0;
   src.neighborhood_size_ = 0;
   // Invalidate all iterators for src.
   ++src.rev;
}

hash_map_impl::~hash_map_impl() {
}

hash_map_impl & hash_map_impl::operator=(hash_map_impl && src) {
   LOFTY_TRACE_FUNC(this);

   hashes = _std::move(src.hashes);
   keys = _std::move(src.keys);
   values = _std::move(src.values);
   total_buckets = src.total_buckets;
   src.total_buckets = 0;
   used_buckets = src.used_buckets;
   src.used_buckets = 0;
   neighborhood_size_ = src.neighborhood_size_;
   src.neighborhood_size_ = 0;
   // Invalidate all iterators for *this and for src.
   ++rev;
   ++src.rev;
   return *this;
}

_std::tuple<std::size_t, bool> hash_map_impl::add_or_assign(
   type_void_adapter const & key_type, type_void_adapter const & value_type, keys_equal_fn_type keys_equal_fn,
   void * key, std::size_t key_hash, void * value, unsigned move
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, keys_equal_fn, key, key_hash, value, move);

   if (total_buckets == 0) {
      grow_table(key_type, value_type);
   }
   /* Repeatedly resize the table until we’re able to find a bucket for the key. This should typically loop at
   most once, but need_larger_neighborhoods may need more. */
   std::size_t bucket;
   while ((bucket = get_existing_or_empty_bucket_for_key(
      key_type, value_type, keys_equal_fn, key, key_hash
   )) >= first_special_index) {
      if (bucket == need_larger_neighborhoods) {
         grow_neighborhoods();
      } else {
         grow_table(key_type, value_type);
      }
   }

   std::size_t * hash_ptr = &hashes[bucket];
   bool add = (*hash_ptr == empty_bucket_hash);
   if (add) {
      // The bucket is currently empty, so initialize it with hash/key/value.
      set_bucket_key_value(key_type, value_type, bucket, key, value, move);
      *hash_ptr = key_hash;
   } else {
      // The bucket already has a value, so overwrite that with the value argument.
      set_bucket_key_value(key_type, value_type, bucket, nullptr, value, move);
   }
   ++used_buckets;
   ++rev;
   return _std::make_tuple(bucket, add);
}

void hash_map_impl::clear(type_void_adapter const & key_type, type_void_adapter const & value_type) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/);

   auto hash_ptr  = hashes.get(), hashes_end = hash_ptr + total_buckets;
   auto key_ptr   = static_cast<std::int8_t *>(keys  .get());
   auto value_ptr = static_cast<std::int8_t *>(values.get());
   for (; hash_ptr < hashes_end; ++hash_ptr, key_ptr += key_type.size(), value_ptr += value_type.size()) {
      if (*hash_ptr != empty_bucket_hash) {
         *hash_ptr = empty_bucket_hash;
         key_type  .destruct(key_ptr);
         value_type.destruct(value_ptr);
      }
   }
   used_buckets = 0;
   ++rev;
}

void hash_map_impl::empty_bucket(
   type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t bucket
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, bucket);

   hashes[bucket] = empty_bucket_hash;
   key_type  .destruct(static_cast<std::int8_t *>(keys  .get()) + key_type  .size() * bucket);
   value_type.destruct(static_cast<std::int8_t *>(values.get()) + value_type.size() * bucket);
   --used_buckets;
   /* We could avoid incrementing rev and invalidating every iterator, since nothing other bucket was
   affected, but that would mean that an iterator to the removed pair could still be dereferenced. */
   ++rev;
}

std::size_t hash_map_impl::find_bucket_movable_to_empty(std::size_t empty_bucket_) const {
   LOFTY_TRACE_FUNC(this, empty_bucket_);

   std::size_t const * empty_hash_ptr = hashes.get() + empty_bucket_;
   /* Minimum number of buckets on the right of empty_bucket_ that we need in order to have a full
   neighborhood to scan. */
   std::size_t buckets_right_of_empty = neighborhood_size_ - 1;
   /* Ensure that the heighborhood ending with empty_bucket_ doesn’t wrap. Always having empty_bucket_ on the
   right of any of the buckets we’re going to check simplifies the calculation of hash_ptr and the range
   checks in the loop. */
   if (empty_bucket_ < buckets_right_of_empty) {
      empty_bucket_ += total_buckets;
   }
   // Calculate the bucket index range of the neighborhood that ends with empty_bucket_.
   auto hash_ptr   = hashes.get() + empty_bucket_ - buckets_right_of_empty;
   auto hashes_end = hashes.get() + total_buckets;
   // Prepare to track the count of collisions (identical hashes) in the selected neighborhood.
   std::size_t sample_hash = *hash_ptr, collisions = 0;
   /* The neighborhood may wrap, so we can only test for inequality and rely on the wrap-around logic at the
   end of the loop body. */
   while (hash_ptr != empty_hash_ptr) {
      /* Get the end of the original neighborhood for the key in this bucket; if the empty bucket is within
      that index, the contents of this bucket can be moved to the empty one. */
      std::size_t curr_nh_end = hash_neighborhood_index(*hash_ptr) + neighborhood_size_;
      // Both indices are allowed to be >total_buckets (see above if), so this comparison is always valid.
      if (empty_bucket_ < curr_nh_end) {
         return static_cast<std::size_t>(hash_ptr - hashes.get());
      }

      if (sample_hash == *hash_ptr) {
         ++collisions;
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++hash_ptr == hashes_end) {
         hash_ptr = hashes.get();
      }
   }
   // No luck.
   if (collisions < buckets_right_of_empty) {
      /* Resizing the hash table will redistribute the hashes in the scanned neighborhood into multiple
      neighborhoods, so repeating this algorithm will find a movable bucket. */
      return need_larger_table;
   } else {
      return need_larger_neighborhoods;
   }
}

std::size_t hash_map_impl::find_empty_bucket(std::size_t nh_begin, std::size_t nh_end) const {
   LOFTY_TRACE_FUNC(this, nh_begin, nh_end);

   auto hash_ptr      = hashes.get() + nh_begin;
   auto hashes_nh_end = hashes.get() + nh_end;
   auto hashes_end    = hashes.get() + total_buckets;
   /* nh_begin - nh_end may be a wrapping range, so we can only test for inequality and rely on the
   wrap-around logic at the end of the loop body. Also, we need to iterate at least once, otherwise we won’t
   enter the loop at all if the start condition is the same as the end condition, which is the case for
   neighborhood_size_ == total_buckets. */
   do {
      if (*hash_ptr == empty_bucket_hash) {
         return static_cast<std::size_t>(hash_ptr - hashes.get());
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++hash_ptr == hashes_end) {
         hash_ptr = hashes.get();
      }
   } while (hash_ptr != hashes_nh_end);
   return null_index;
}

std::size_t hash_map_impl::find_empty_bucket_outside_neighborhood(
   type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t nh_begin,
   std::size_t nh_end
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, nh_begin, nh_end);

   // Find an empty bucket, scanning every bucket outside the neighborhood.
   std::size_t empty_bucket_ = find_empty_bucket(nh_end, nh_begin);
   if (empty_bucket_ == null_index) {
      // No luck, the hash table needs to be resized.
      return null_index;
   }
   /* This loop will enter (and maybe repeat) if we have an empty bucket, but it’s not in the key’s
   neighborhood, so we have to try and move it in the neighborhood. The not-in-neighborhood check is made more
   complicated by the fact the range may wrap. */
   while (nh_begin < nh_end
      ? empty_bucket_ >= nh_end || empty_bucket_ < nh_begin // Non-wrapping: |---[begin end)---|
      : empty_bucket_ >= nh_end && empty_bucket_ < nh_begin // Wrapping:     | end)-----[begin |
   ) {
      /* The empty bucket is out of the neighborhood. Find the first non-empty bucket that’s part of the left-
      most neighborhood containing empty_bucket_, but excluding buckets occupied by keys belonging to other
      overlapping neighborhoods. */
      std::size_t movable_bucket = find_bucket_movable_to_empty(empty_bucket_);
      if (movable_bucket >= first_special_index) {
         /* No buckets have contents that can be moved to empty_bucket_; the hash table or the neighborhoods
         need to be resized. */
         return movable_bucket;
      }
      // Move the contents of movable_bucket to empty_bucket_.
      set_bucket_key_value(
         key_type, value_type, empty_bucket_,
         static_cast<std::int8_t *>(keys  .get()) + key_type  .size() * movable_bucket,
         static_cast<std::int8_t *>(values.get()) + value_type.size() * movable_bucket,
         1 | 2 /*move both key and value*/
      );
      hashes[empty_bucket_] = hashes[movable_bucket];
      empty_bucket_ = movable_bucket;
   }
   return empty_bucket_;
}

std::size_t hash_map_impl::get_empty_bucket_for_key(
   type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t key_hash
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, key_hash);

   std::size_t nh_begin, nh_end;
   _std::tie(nh_begin, nh_end) = hash_neighborhood_range(key_hash);
   // Search for an empty bucket in the neighborhood.
   std::size_t bucket = find_empty_bucket(nh_begin, nh_end);
   if (bucket != null_index) {
      return bucket;
   }
   return find_empty_bucket_outside_neighborhood(key_type, value_type, nh_begin, nh_end);
}

std::size_t hash_map_impl::get_existing_or_empty_bucket_for_key(
   type_void_adapter const & key_type, type_void_adapter const & value_type, keys_equal_fn_type keys_equal_fn,
   void const * key, std::size_t key_hash
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, keys_equal_fn, key, key_hash);

   std::size_t nh_begin, nh_end;
   _std::tie(nh_begin, nh_end) = hash_neighborhood_range(key_hash);
   // Look for the key or an empty bucket in the neighborhood.
   std::size_t bucket = lookup_key_or_find_empty_bucket(
      key_type, keys_equal_fn, key, key_hash, nh_begin, nh_end
   );
   if (bucket != null_index) {
      return bucket;
   }
   return find_empty_bucket_outside_neighborhood(key_type, value_type, nh_begin, nh_end);
}

void hash_map_impl::grow_table(type_void_adapter const & key_type, type_void_adapter const & value_type) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/);

   // The “old” names of these four variables will make sense in a moment…
   std::size_t old_total_buckets = total_buckets ? total_buckets * growth_factor : min_buckets;
   _std::unique_ptr<std::size_t[]> old_hashes(new std::size_t[old_total_buckets]);
   auto old_keys  (memory::alloc_bytes_unique(  key_type.size() * old_total_buckets));
   auto old_values(memory::alloc_bytes_unique(value_type.size() * old_total_buckets));
   // At this point we’re safe from exceptions, so we can update the member variables.
   _std::swap(total_buckets, old_total_buckets);
   _std::swap(hashes, old_hashes);
   _std::swap(keys,   old_keys);
   _std::swap(values, old_values);
   // Now the names of these variables make sense :)

   /* Recalculate the neighborhood size. The (missing) “else” to this “if” is for when the actual neighborhood
   size is greater than the ideal, which can happen when dealing with a subpar hash function that resulted in
   more collisions than ideal_neighborhood_size. In that scenario, the table size increase doesn’t change
   anything, since the fix has already been applied with a change in neighborhood_size_ which happened before
   this method was called. */
   if (neighborhood_size_ < ideal_neighborhood_size) {
      if (total_buckets < ideal_neighborhood_size) {
         /* neighborhood_size_ has not yet reached ideal_neighborhood_size, but it can’t exceed total_buckets,
         so set it to the latter. */
         neighborhood_size_ = total_buckets;
      } else {
         // Fix neighborhood_size_ to its ideal value.
         neighborhood_size_ = ideal_neighborhood_size;
      }
   }

   // Initialize hashes[i] with empty_bucket_hash.
   memory::clear(hashes.get(), total_buckets);
   // Re-insert each hash/key/value triplet to move it from the old arrays to the new ones.
   auto old_hash_ptr  = old_hashes.get(), old_hashes_end = old_hash_ptr + old_total_buckets;
   auto old_key_ptr   = static_cast<std::int8_t *>(old_keys  .get());
   auto old_value_ptr = static_cast<std::int8_t *>(old_values.get());
   for (
      ;
      old_hash_ptr < old_hashes_end;
      ++old_hash_ptr, old_key_ptr += key_type.size(), old_value_ptr += value_type.size()
   ) {
      if (*old_hash_ptr != empty_bucket_hash) {
         std::size_t new_bucket = get_empty_bucket_for_key(key_type, value_type, *old_hash_ptr);
         LOFTY_ASSERT(new_bucket < first_special_index,
            LOFTY_SL("failed to find empty bucket while growing hash table; ")
            LOFTY_SL("if it could be found before, why not now when there are more buckets?")
         );

         // Move hash/key/value to the new bucket.
         set_bucket_key_value(key_type, value_type, new_bucket, old_key_ptr, old_value_ptr, 1 | 2);
         hashes[new_bucket] = *old_hash_ptr;
         key_type  .destruct(old_key_ptr);
         value_type.destruct(old_value_ptr);
      }
   }
}

std::size_t hash_map_impl::lookup_key_or_find_empty_bucket(
   type_void_adapter const & key_type, keys_equal_fn_type keys_equal_fn, void const * key,
   std::size_t key_hash, std::size_t nh_begin, std::size_t nh_end
) const {
   LOFTY_TRACE_FUNC(this/*, key_type*/, keys_equal_fn, key, key_hash, nh_begin, nh_end);

   auto hash_ptr      = hashes.get() + nh_begin;
   auto hashes_nh_end = hashes.get() + nh_end;
   auto hashes_end    = hashes.get() + total_buckets;
   /* nh_begin - nh_end may be a wrapping range, so we can only test for inequality and rely on the wrap-
   around logic at the end of the loop body. Also, we need to iterate at least once, otherwise we won’t enter
   the loop at all if the start condition is the same as the end condition, which is the case for
   neighborhood_size_ == total_buckets. */
   do {
      if (
         *hash_ptr == empty_bucket_hash ||
         /* Multiple calculations of the second half of the && should be rare enough (exact key match or hash
         collision) to make recalculating the offset from keys cheaper than keeping a cursor over keys running
         in parallel to hash_ptr. */
         (*hash_ptr == key_hash && keys_equal_fn(
            this,
            static_cast<std::int8_t const *>(keys.get()) +
               key_type.size() * static_cast<std::size_t>(hash_ptr - hashes.get()),
            key
         ))
      ) {
         return static_cast<std::size_t>(hash_ptr - hashes.get());
      }

      // Move on to the next bucket, wrapping around to the first one if needed.
      if (++hash_ptr == hashes_end) {
         hash_ptr = hashes.get();
      }
   } while (hash_ptr != hashes_nh_end);
   return null_index;
}

void hash_map_impl::set_bucket_key_value(
   type_void_adapter const & key_type, type_void_adapter const & value_type, std::size_t bucket, void * key,
   void * value, unsigned move
) {
   LOFTY_TRACE_FUNC(this/*, key_type, value_type*/, bucket, key, value, move);

   if (key) {
      void * dst = static_cast<std::int8_t *>(keys.get()) + key_type.size() * bucket;
      if (move & 1) {
         key_type.move_construct(dst, key);
      } else {
         key_type.copy_construct(dst, key);
      }
   }
   {
      void * dst = static_cast<std::int8_t *>(values.get()) + value_type.size() * bucket;
      if (move & 2) {
         value_type.move_construct(dst, value);
      } else {
         value_type.copy_construct(dst, value);
      }
   }
}


hash_map_impl::iterator_base::iterator_base() :
   owner_map(nullptr),
   bucket(null_index) {
}

hash_map_impl::iterator_base::iterator_base(hash_map_impl const * owner_map_, std::size_t bucket_) :
   owner_map(owner_map_),
   bucket(bucket_),
   rev(owner_map->rev) {
}

void hash_map_impl::iterator_base::increment() {
   LOFTY_TRACE_FUNC(this);

   for (;;) {
      ++bucket;
      if (bucket >= owner_map->total_buckets) {
         bucket = null_index;
         return;
      }
      if (owner_map->hashes[bucket] != empty_bucket_hash) {
         return;
      }
   }
}

void hash_map_impl::iterator_base::validate() const {
   LOFTY_TRACE_FUNC(this);

   if (bucket == null_index || rev != owner_map->rev) {
      LOFTY_THROW(out_of_range, ());
   }
}

}}} //namespace lofty::collections::_pvt

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_KEYED_DEMUX_HXX
#define _LOFTY_KEYED_DEMUX_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/hash_map.hxx>
#include <lofty/event.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Dispatches values from a source, according to keys provided with the source. It allows for multiple
clients to wait on different keys, only unblocking one of them when a value with a matching key is returned by
the source. */
template <typename TKey, typename TValue>
class keyed_demux : public noncopyable {
private:
   //! Tracks a single outstanding get() call.
   struct outstanding_get_t {
      //! Event used to block the coroutine performing the get() call.
      lofty::event event;
      //! Storage to transfer data from the source coroutine to a get() call.
      TValue value;

      //! Default constructor.
      outstanding_get_t() :
         value() {
      }
   };

public:
   //! Default constructor.
   keyed_demux() {
   }

   /*! Schedules the source coroutine, which will call the provided function to obtain values and their keys.
   When a key matches one provided by a caller to get(), that caller will be unblocked, and the value returned
   to it.

   @param source_fn
      Source function. This is supposed to obtain one value, extract a key from it, and return the value. If
      the returned value evaluates to false, the function will not be called again, and all get() calls will
      return a default-constructed value.
   */
   void set_source(_std::function<TValue (TKey *)> source_fn) {
      coroutine([this, source_fn] () {
         TKey key;
         while (auto value = source_fn(&key)) {
            auto outstanding_get(outstanding_gets.find(key));
            if (outstanding_get == outstanding_gets.cend()) {
               // TODO: this is a client bug; log it or maybe invoke some client-provided callback.
               continue;
            }
            outstanding_get->value.value = _std::move(value);
            outstanding_get->value.event.trigger();
            // TODO: allow this class to schedule directly the get() call that is waiting on the event.
            this_coroutine::sleep_for_ms(1);
         }
         /* On end of source, all get() callers are unblocked and get a default-constructed value (delayed to
         the end of this coroutine due to scheduling) . */
         /* TODO: this should be protected by a thread-level mutex, since the .remove() in get() will break
         looping if the unblocked coroutines resume executing in another thread. */
         LOFTY_FOR_EACH(auto outstanding_get, outstanding_gets) {
            outstanding_get.value.event.trigger();
         }
      });
   }

   /*! Waits for a value with the given key to be returned by the source function.

   @param key
      Key associated to the value to obtain from the source function.
   @param timeout_millisecs
      Optional timeout for the wait, in milliseconds. If the wait for the value exceeds this amount, an
      exception of type io::timeout will be thrown.
   @return
      Value returned by the source function for the given key, or a default-constructed value if the source
      function returned a value evaluating to false.
   */
   TValue get(TKey const & key, unsigned timeout_millisecs = 0) {
      auto outstanding_get(_std::get<0>(outstanding_gets.add_or_assign(key, outstanding_get_t())));

      outstanding_get->value.event.wait(timeout_millisecs);

      // Re-retrieve the key/value, since outstanding_gets might have changed in the meantime.
      outstanding_get = outstanding_gets.find(key);
      TValue ret(_std::move(outstanding_get->value.value));
      outstanding_gets.remove(outstanding_get);
      return _std::move(ret);
   }

private:
   //! Tracks all outstanding waits, so that the source coroutine can trigger the associated events as needed.
   collections::hash_map<TKey, outstanding_get_t> outstanding_gets;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_KEYED_DEMUX_HXX

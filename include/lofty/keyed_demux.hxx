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
#include <lofty/coroutine.hxx>
#include <lofty/event.hxx>
#include <lofty/thread.hxx>


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
      /*! Pointer to the event used to block the coroutine performing the get() call. Needs to be a pointer so
      that the caller of wait() won’t be affected by outstanding_gets being resized (and event being moved
      from allocation to allocation) while it’s waiting. */
      lofty::event * event;
      //! Storage to transfer data from the source coroutine to a get() call.
      TValue value;

      /*! Constructor.

      @param event_
         Pointer to the event.
      */
      explicit outstanding_get_t(lofty::event * event_) :
         event(event_),
         value() {
      }
   };

public:
   //! Default constructor.
   keyed_demux() {
   }

   ~keyed_demux() {
      if (source_thread.joinable()) {
         source_thread.interrupt();
         source_thread.join();
      } else if (source_coroutine.joinable()) {
         source_coroutine.interrupt();
         source_coroutine.join();
      }
   }

   /*! Schedules the source loop, which will call the provided function to obtain values and their keys. When
   a key matches one provided by a caller to get(), that caller will be unblocked, and the value returned to
   it.

   The source loop runs on a separate thread or coroutine, depending on whether the calling thread has an
   associated coroutine scheduler.

   @param source_fn
      Source function. This is supposed to obtain one value, extract a key from it, and return the value. If
      the returned value evaluates to false, the function will not be called again, and all get() calls will
      return a default-constructed value.
   */
   void set_source(_std::function<TValue (TKey *)> source_fn) {
      _std::function<void ()> source_loop([this, source_fn] () {
         TKey key;
         try {
            while (auto value = source_fn(&key)) {
               {
                  _std::unique_lock<_std::mutex> lock(outstanding_gets_mutex);
                  auto itr(outstanding_gets.find(key));
                  if (itr == outstanding_gets.cend()) {
                     // TODO: this is a client bug; log it or maybe invoke some client-provided callback.
                     continue;
                  }
                  itr->value.value = _std::move(value);
                  itr->value.event->trigger();
               }
               // TODO: allow this class to schedule directly the get() call that is waiting on the event.
               this_coroutine::sleep_for_ms(1);
            }
         } catch (execution_interruption const &) {
            // source_fn() was interrupted; proceed with releasing all get() callers.
         }
         /* On end of source, all get() callers are unblocked and get a default-constructed value (delayed to
         the end of this coroutine due to scheduling) . */
         /* Need to acquire the mutex because the .remove() in get() will break looping if running in another
         thread, or if an unblocked coroutine resumes executing in another thread. */
         _std::unique_lock<_std::mutex> lock(outstanding_gets_mutex);
         LOFTY_FOR_EACH(auto kv, outstanding_gets) {
            kv.value.event->trigger();
         }
      });
      if (this_thread::coroutine_scheduler()) {
         source_coroutine = coroutine(_std::move(source_loop));
      } else {
         source_thread = thread(_std::move(source_loop));
      }
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
      event get_event;
      {
         _std::unique_lock<_std::mutex> lock(outstanding_gets_mutex);
         outstanding_gets.add_or_assign(key, outstanding_get_t(&get_event));
      }
      try {
         get_event.wait(timeout_millisecs);
      } catch (io::timeout const &) {
         // The event is dead to us.
         _std::unique_lock<_std::mutex> lock(outstanding_gets_mutex);
         outstanding_gets.remove(key);
         throw;
      }

      // Re-retrieve the key/value, since outstanding_gets might have changed in the meantime.
      _std::unique_lock<_std::mutex> lock(outstanding_gets_mutex);
      auto itr(outstanding_gets.find(key));
      TValue ret(_std::move(itr->value.value));
      outstanding_gets.remove(itr);
      return _std::move(ret);
   }

private:
   //! Tracks all outstanding waits, so that the source coroutine can trigger the associated events as needed.
   collections::hash_map<TKey, outstanding_get_t> outstanding_gets;
   //! Guards concurrent access to outstanding_gets.
   _std::mutex outstanding_gets_mutex;
   //! Source thread (thread mode-only) to join on termination.
   thread source_thread;
   //! Source coroutine (coroutine mode-only) to join on termination.
   coroutine source_coroutine;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_KEYED_DEMUX_HXX

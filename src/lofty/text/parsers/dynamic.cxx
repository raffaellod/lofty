/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

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
#include <lofty/collections/vector.hxx>
#include <lofty/text/parsers/dynamic.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

namespace {

//! Backtracking data structure.
struct backtrack {
   dynamic::state const * state;
   bool accepted;

   backtrack(dynamic::state const * state_, bool accepted_) :
      state(state_),
      accepted(accepted_) {
   }
};

//! Used to track the acceptance of repetition states.
struct repetition {
   explicit repetition(dynamic::state const * state_) :
      state(state_),
      count(0) {
   }

   dynamic::state const * state;
   std::uint16_t count;
};

} //namespace


struct dynamic::match::capture_node {
   //! Pointer to the capture_begin state.
   struct state const * state;
   //! Pointer to the parent (containing) capture. Only nullptr for capture 0.
   capture_node * parent;
   //! Owning pointer to the first nested capture, if any.
   _std::unique_ptr<capture_node> first_nested;
   //! Non-owning pointer to the last nested capture, if any.
   capture_node * last_nested;
   //! Non-owning pointer to the previous same-level capture, if any.
   capture_node * prev_sibling;
   //! Owning pointer to the next same-level capture, if any.
   _std::unique_ptr<capture_node> next_sibling;
   //! Offset of the start of the capture.
   std::size_t begin;
   //! Offset of the end of the capture.
   std::size_t end;

   /*! Constructor.

   @param state_
      Pointer to the related capture_begin state.
   */
   explicit capture_node(struct state const * state_) :
      state(state_),
      parent(nullptr),
      last_nested(nullptr),
      prev_sibling(nullptr) {
   }

   //! Destructor.
   ~capture_node() {
      // last_nested is the only non-owning forward pointer, so it must be updated manually.
      if (parent && parent->last_nested == this) {
         parent->last_nested = prev_sibling;
      }
   }

   /*! Adds a new capture_node at the end of the nested capture nodes list.

   @param state_
      Pointer to the related capture_begin state.
   */
   capture_node * append_nested(struct state const * state_) {
      _std::unique_ptr<capture_node> ret_owner(new capture_node(state_));
      auto ret = ret_owner.get();
      if (last_nested) {
         last_nested->next_sibling = _std::move(ret_owner);
         ret->prev_sibling = last_nested;
      } else {
         first_nested = _std::move(ret_owner);
         last_nested = ret;
      }
      ret->parent = this;
      return ret;
   }
};


dynamic::match::match() {
}

dynamic::match::match(match && src) :
   captures_buffer(_std::move(src.captures_buffer)),
   capture0(_std::move(src.capture0)) {
}


dynamic::match::~match() {
}


dynamic::dynamic() :
   initial_state(nullptr) {
}

dynamic::dynamic(dynamic && src) :
   states_list(_std::move(src.states_list)),
   initial_state(src.initial_state) {
   src.initial_state = nullptr;
}

dynamic::~dynamic() {
}

dynamic::state * dynamic::create_begin_state() {
   return create_uninitialized_state(state_type::begin);
}

dynamic::state * dynamic::create_capture_begin_state() {
   return create_uninitialized_state(state_type::capture_begin);
}

dynamic::state * dynamic::create_capture_end_state() {
   return create_uninitialized_state(state_type::capture_end);
}

dynamic::state * dynamic::create_code_point_state(char32_t cp) {
   state * ret = create_uninitialized_state(state_type::range);
   ret->u.range.first_cp = cp;
   ret->u.range.last_cp = cp;
   return ret;
}

dynamic::state * dynamic::create_code_point_range_state(char32_t first_cp, char32_t last_cp) {
   state * ret = create_uninitialized_state(state_type::range);
   ret->u.range.first_cp = first_cp;
   ret->u.range.last_cp = last_cp;
   return ret;
}

dynamic::state * dynamic::create_end_state() {
   return create_uninitialized_state(state_type::end);
}

dynamic::state * dynamic::create_repetition_state(
   state const * repeated_state, std::uint16_t min, std::uint16_t max /*= 0*/
) {
   state * ret = create_uninitialized_state(state_type::repetition);
   ret->u.repetition.repeated_state = repeated_state;
   ret->u.repetition.min = min;
   ret->u.repetition.max = max;
   ret->u.repetition.greedy = true;
   return ret;
}

dynamic::state * dynamic::create_uninitialized_state(state_type type) {
   states_list.push_back(state());
   state * ret = static_cast<state *>(&states_list.back());
   ret->type = type.base();
   ret->next = nullptr;
   ret->alternative = nullptr;
   return ret;
}

dynamic::match dynamic::run(str const & s) const {
   io::text::str_istream istream(external_buffer, &s);
   return run(&istream);
}

dynamic::match dynamic::run(io::text::istream * istream) const {
   LOFTY_TRACE_FUNC(this, istream);

   match ret;
   auto curr_state = initial_state;
   // Cache this condition to quickly determine whether we’re allowed to skip input code points.
   bool begin_anchor = (curr_state && curr_state->type == state_type::begin && !curr_state->alternative);
   // Setup the two sources of code points: a history and a peek buffer from the input stream.
   str & history_buf = ret.captures_buffer, peek_buf = istream->peek_chars(1);
   auto history_begin_itr(history_buf.cbegin()), history_end(history_buf.cend());
   auto history_itr(history_begin_itr), peek_itr(peek_buf.cbegin()), peek_end(peek_buf.cend());

   ret.capture0.reset(new match::capture_node(curr_state));
   auto curr_capture = ret.capture0.get();

   // TODO: change these two variables to use collections::stack once that’s available.
   collections::vector<backtrack> backtracking_stack;
   /* Stack of repetition counters. A new counter is pushed when a new repetition is started, and popped when
   that repetition is a) matched max times or b) backtracked over. */
   collections::vector<repetition> reps_stack;

   bool accepted = (curr_state == nullptr);
   while (curr_state) {
      state const * next_state = curr_state->next;
      switch (curr_state->type) {
         case state_type::range: {
            // Get a code point from either history or the peek buffer.
            char32_t cp;
            bool save_peeked_cp_to_history;
            if (history_itr != history_end) {
               cp = *history_itr;
               save_peeked_cp_to_history = false;
            } else {
               if (peek_itr == peek_end) {
                  istream->consume_chars(peek_buf.size_in_chars());
                  peek_buf = istream->peek_chars(1);
                  peek_itr = peek_buf.cbegin();
                  peek_end = peek_buf.cend();
                  if (peek_itr == peek_end) {
                     // Run out of code points.
                     accepted = false;
                     break;
                  }
               }
               cp = *peek_itr;
               save_peeked_cp_to_history = true;
            }

            accepted = (cp >= curr_state->u.range.first_cp && cp <= curr_state->u.range.last_cp);
            if (accepted) {
               if (save_peeked_cp_to_history) {
                  history_buf += cp;
                  ++history_end;
                  ++peek_itr;
               }
               ++history_itr;
            }
            break;
         }

         case state_type::repetition: {
            repetition * rep;
            if (reps_stack && (rep = &reps_stack.front())->state == curr_state) {
               ++rep->count;
            } else {
               // New repetition: save it on the stack and begin counting.
               reps_stack.push_back(repetition(curr_state));
               rep = &reps_stack.back();
            }
            if (curr_state->u.repetition.max == 0 || rep->count <= curr_state->u.repetition.max) {
               // Repetitions within [min, max] are accepting.
               accepted = (rep->count >= curr_state->u.repetition.min);
               // Try one more repetition.
               next_state = curr_state->u.repetition.repeated_state;
            } else {
               // Repeated max times; pop the stack and move on to the next state.
               accepted = true;
               reps_stack.pop_back();
            }
            goto skip_acceptance_test;
         }

         case state_type::begin:
            accepted = (history_itr == history_begin_itr);
            break;

         case state_type::end:
            if (history_itr == history_end && peek_itr == peek_end) {
               /* We consumed history and peek buffer, but “end” really means end, so also check that the
               stream is empty. */
               /* TODO: this might be redundant, since other code point consumers in this function always do
               this after consuming a code point. */
               istream->consume_chars(peek_buf.size_in_chars());
               peek_buf = istream->peek_chars(1);
               accepted = !peek_buf;
            } else {
               accepted = false;
            }
            break;

         case state_type::capture_begin:
            // Nest this capture into the currently-open capture.
            curr_capture = curr_capture->append_nested(curr_state);
            curr_capture->begin = history_itr.char_index();
            goto skip_acceptance_test;

         case state_type::capture_end:
            curr_capture->end = history_itr.char_index();
            // This capture is over; resume its parent.
            curr_capture = curr_capture->parent;
            goto skip_acceptance_test;
      }

      if (accepted) {
skip_acceptance_test:
         if (!next_state) {
            // No more states; this means that the input was accepted.
            break;
         }
         // Still one or more states to check; this means that we can’t accept the input just yet.
         backtracking_stack.push_back(backtrack(curr_state, accepted));
         curr_state = next_state;
      } else {
         // Consider the next alternative of the current state or a prior one.
         curr_state = curr_state->alternative;
         while (!curr_state && backtracking_stack) {
            auto backtrack(backtracking_stack.pop_back());
            switch (backtrack.state->type) {
               case state_type::range:
                  /* If the state we’re rolling back accepted (and consumed) a code point, it must’ve saved
                  it in history_buf, so recover it from there. */
                  if (backtrack.accepted) {
                     --history_itr;
                  }
                  break;

               case state_type::repetition:
                  // If we’re backtracking the current top-of-the-stack repetition, pop it out of it.
                  if (reps_stack && reps_stack.back().state == backtrack.state) {
                     reps_stack.pop_back();
                  }
                  /* This is a repetition’s Nth occurrence; decide what to do depending on whether N is in the
                  acceptable range. */
                  if (!backtrack.accepted) {
                     // Move on to the alternative, like non-repeating states.
                     break;
                  } else if (backtrack.state->next) {
                     // Move past the repetition, ignoring any alternatives.
                     curr_state = backtrack.state->next;
                     continue;
                  } else {
                     // If there was no following state, the input is accepted.
                     accepted = true;
                     goto break_outer_while;
                  }

               case state_type::capture_begin: {
                  // Discard *curr_capture (by resetting its owner’s pointer) and move back to its parent.
                  auto parent_capture = curr_capture->parent;
                  if (auto prev_sibling = curr_capture->prev_sibling) {
                     prev_sibling->next_sibling.reset();
                  } else {
                     parent_capture->first_nested.reset();
                  }
                  curr_capture = parent_capture;
                  break;
               }

               case state_type::capture_end:
                  // Re-enter the last (closed) nested capture.
                  curr_capture = curr_capture->last_nested;
                  break;

               default:
                  // No special action needed.
                  break;
            }
            // Not a repetition, or Nth occurrence with N not in the acceptable range.
            curr_state = backtrack.state->alternative;
         }
         /* If we run out of alternatives and can’t backtrack any further, and the pattern is not anchored,
         we’re allowed to move one code point to history and try the whole pattern again from the initial
         state. */
         if (!curr_state && !begin_anchor) {
            if (history_itr == history_end) {
               if (peek_itr == peek_end) {
                  istream->consume_chars(peek_buf.size_in_chars());
                  peek_buf = istream->peek_chars(1);
                  if (!peek_buf) {
                     // Run out of code points.
                     break;
                  }
                  peek_itr = peek_buf.cbegin();
                  peek_end = peek_buf.cend();
               }
               history_buf += *peek_itr++;
               ++history_end;
            }
            ++history_itr;
            curr_state = initial_state;
         }
      }
   }
break_outer_while:
   if (!accepted) {
      ret.capture0.reset();
   }
   return _std::move(ret);
}

}}} //namespace lofty::text::parsers

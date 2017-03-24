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
   bool did_consume_cp:1;
   bool repetition:1;
   bool accepted_repetition:1;

   static backtrack make_default(dynamic::state const * state_, bool did_consume_cp) {
      backtrack new_backtrack;
      new_backtrack.state = state_;
      new_backtrack.did_consume_cp = did_consume_cp;
      new_backtrack.repetition = false;
      new_backtrack.accepted_repetition = false;
      return _std::move(new_backtrack);
   }

   static backtrack make_repetition(dynamic::state const * state_, bool accepted) {
      backtrack new_backtrack;
      new_backtrack.state = state_;
      new_backtrack.did_consume_cp = false;
      new_backtrack.repetition = true;
      new_backtrack.accepted_repetition = accepted;
      return _std::move(new_backtrack);
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


dynamic::match::match() :
   accepted(false) {
}

dynamic::match::match(match && src) :
   capture0(std::move(src.capture0)),
   accepted(src.accepted) {
   src.accepted = false;
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
   match ret;

   state const * curr_state = initial_state;
   // Cache this condition to quickly determine whether we’re allowed to skip input code points.
   bool begin_anchor = (curr_state && curr_state->type == state_type::begin && !curr_state->alternative);
   // Setup the two sources of code points: a history and a peek buffer from the input stream.
   str & history_buf = ret.capture0, peek_buf = istream->peek_chars(1);
   auto history_begin_itr(history_buf.cbegin()), history_end(history_buf.cend());
   auto history_itr(history_begin_itr), peek_itr(peek_buf.cbegin()), peek_end(peek_buf.cend());

   // TODO: change these two variables to use collections::stack once that’s available.
   collections::vector<backtrack> backtracking_stack;
   /* Stack of repetition counters. A new counter is pushed when a new repetition is started, and popped when
   that repetition is a) matched max times or b) backtracked over. */
   collections::vector<repetition> reps_stack;

   while (curr_state) {
      state const * next = nullptr;
      bool did_consume_cp = false;
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
                     break;
                  }
               }
               cp = *peek_itr;
               save_peeked_cp_to_history = true;
            }

            if (cp >= curr_state->u.range.first_cp && cp <= curr_state->u.range.last_cp) {
               ret.accepted = true;
               did_consume_cp = true;
               next = curr_state->next;
               if (save_peeked_cp_to_history) {
                  history_buf += cp;
                  ++history_end;
                  ++peek_itr;
               }
               ++history_itr;
            }
            break;
         }

         case state_type::repetition:
            repetition * rep;
            if (reps_stack && (rep = &reps_stack.front())->state == curr_state) {
               ++rep->count;
            } else {
               // New repetition: save it on the stack and begin counting.
               reps_stack.push_back(repetition(curr_state));
               // TODO: FIXME: pushing back but then using front?!
               rep = &reps_stack.front();
            }
            if (curr_state->u.repetition.max == 0 || rep->count <= curr_state->u.repetition.max) {
               if (rep->count >= curr_state->u.repetition.min) {
                  // Repetitions within [min, max] are accepting.
                  ret.accepted = true;
               }
               // Try one more repetition.
               next = curr_state->u.repetition.repeated_state;
            } else {
               // Repeated max times; pop the stack and move on to the next state.
               reps_stack.pop_back();
               next = curr_state->next;
            }

            // This code is very similar to the “if (ret.accepted)” below.
            if (!next) {
               // No more states; this means that the input was accepted.
               break;
            }
            backtracking_stack.push_back(backtrack::make_repetition(curr_state, ret.accepted));
            ret.accepted = false;
            curr_state = next;
            // Skip the accept/backtrack logic at the end of the loop.
            continue;

         case state_type::begin:
            if (history_itr == history_begin_itr) {
               ret.accepted = true;
               next = curr_state->next;
            }
            break;

         case state_type::end:
            if (history_itr == history_end && peek_itr == peek_end) {
               /* We consumed history and peek buffer, but “end” really means end, so also check that the
               stream is empty. */
               /* TODO: this might be redundant, since other code point consumers in this function always do
               this after consuming a code point. */
               istream->consume_chars(peek_buf.size_in_chars());
               peek_buf = istream->peek_chars(1);
               if (!peek_buf) {
                  ret.accepted = true;
                  next = curr_state->next;
               }
            }
            break;

         case state_type::look_ahead:
            // TODO: implement look-ahead assertions.
            break;
      }

      if (ret.accepted) {
         if (!next) {
            // No more states; this means that the input was accepted.
            break;
         }
         // Still one or more states to check; this means that we can’t accept the input just yet.
         backtracking_stack.push_back(backtrack::make_default(curr_state, did_consume_cp));
         ret.accepted = false;
         curr_state = next;
      } else {
         // Consider the next alternative.
         curr_state = curr_state->alternative;
         // Go back to a state that had alternatives, if possible.
         while (!curr_state && backtracking_stack) {
            auto backtrack(backtracking_stack.pop_back());
            // If we’re backtracking the current top-of-the-stack repetition, pop it out of it.
            if (backtrack.repetition && reps_stack && reps_stack.back().state == backtrack.state) {
               reps_stack.pop_back();
            }
            if (backtrack.accepted_repetition) {
               // This must be a repetition’s Nth occurrence, with N in the acceptable range.
               if (!backtrack.state->next) {
                  // If there was no following state, the input is accepted.
                  ret.accepted = true;
                  goto break_outer_while;
               }
               curr_state = backtrack.state->next;
            } else {
               // Not a repetition, or Nth occurrence with N not in the acceptable range.
               curr_state = backtrack.state->alternative;
            }
            // If the state we’re rolling back consumed a code point, it must’ve saved it in history_buf.
            if (backtrack.did_consume_cp) {
               --history_itr;
            }
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
   return std::move(ret);
}

}}} //namespace lofty::text::parsers

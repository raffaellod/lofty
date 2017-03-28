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
   //! Pointer to the state the backtrack refers to.
   dynamic::state const * state;
   //! true if *state accepted the input, or false otherwise.
   bool accepted;
   bool group_closed;

   /*! Constructor.

   @param state_
      Pointer to the state the backtrack refers to.
   @param accepted_
      true if *state accepted the input, or false otherwise.
   */
   backtrack(dynamic::state const * state_, bool accepted_) :
      state(state_),
      accepted(accepted_),
      group_closed(false) {
   }
};

//! Used to track the acceptance of repetition states.
struct repetition {
   //! Pointer to the related repetition state.
   dynamic::state const * state;
   //! Number of times the repetition has occurred.
   unsigned count;

   /*! Constructor.

   @param state_
      Pointer to the related repetition state.
   */
   explicit repetition(dynamic::state const * state_) :
      state(state_),
      count(0) {
   }
};

} //namespace


struct dynamic::match::group_node {
   //! Pointer to the related group state.
   struct state const * state;
   //! Pointer to the parent (containing) group. Only nullptr for capture 0.
   group_node * parent;
   //! Owning pointer to the first nested group, if any.
   _std::unique_ptr<group_node> first_nested;
   //! Non-owning pointer to the last nested group, if any.
   group_node * last_nested;
   //! Non-owning pointer to the previous same-level group, if any.
   group_node * prev_sibling;
   //! Owning pointer to the next same-level group, if any.
   _std::unique_ptr<group_node> next_sibling;
   union {
      struct {
         //! Offset of the start of the capture.
         std::size_t begin;
         //! Offset of the end of the capture.
         std::size_t end;
      } capture;
      struct {
         //! Number of times the repetition has occurred.
         unsigned count;
      } repetition;
   } u;

   /*! Constructor.

   @param state_
      Pointer to the related group state.
   */
   explicit group_node(struct state const * state_) :
      state(state_),
      parent(nullptr),
      last_nested(nullptr),
      prev_sibling(nullptr) {
   }

   //! Destructor.
   ~group_node() {
      // last_nested is the only non-owning forward pointer, so it must be updated manually.
      if (parent && parent->last_nested == this) {
         parent->last_nested = prev_sibling;
      }
   }

   /*! Adds a new group_node at the end of the nested group nodes list.

   @param state_
      Pointer to the related group state.
   */
   group_node * append_nested(struct state const * state_) {
      _std::unique_ptr<group_node> ret_owner(new group_node(state_));
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

   /*! Deletes *this by resetting its owner’s pointer, and returns parent.

   @return
      Pointer to the parent of the deleted node.
   */
   group_node * delete_and_get_parent() {
      auto ret = parent;
      if (prev_sibling) {
         prev_sibling->next_sibling.reset();
      } else {
         parent->first_nested.reset();
      }
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

dynamic::state * dynamic::create_capture_group(state const * first_state) {
   auto ret = create_uninitialized_state(state_type::capture_group);
   ret->u.capture.first_state = first_state;
   return ret;
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

   ret.capture0.reset(new match::group_node(curr_state));
   ret.capture0->u.capture.begin = history_itr.char_index();
   auto curr_group = ret.capture0.get();

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
            if (reps_stack && (rep = &reps_stack.back())->state == curr_state) {
               ++rep->count;
            } else {
               // New repetition: save it on the stack and begin counting.
               reps_stack.push_back(repetition(curr_state));
               rep = &reps_stack.back();
            }
            if (
               curr_state->u.repetition.max == 0 ||
               rep->count <= static_cast<unsigned>(curr_state->u.repetition.max)
            ) {
               // Repetitions within [min, max] are accepting.
               accepted = (rep->count >= static_cast<unsigned>(curr_state->u.repetition.min));
               // Try one more repetition.
               next_state = curr_state->u.repetition.repeated_state;
            } else {
               // Repeated max times; pop the stack and move on to the next state.
               accepted = true;
               reps_stack.pop_back();
            }
            backtracking_stack.push_back(backtrack(curr_state, accepted));
            curr_state = next_state;
            continue;
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

         case state_type::capture_group:
            // Nest this capture into the currently-open capture.
            curr_group = curr_group->append_nested(curr_state);
            curr_group->u.capture.begin = history_itr.char_index();
            next_state = curr_state->u.capture.first_state;
            goto skip_acceptance_test;
      }

      if (accepted) {
skip_acceptance_test:
         backtracking_stack.push_back(backtrack(curr_state, accepted));
         while (!next_state && curr_group->parent) {
            // A nullptr as next state means the end of the current group.
            backtracking_stack.push_back(backtrack(curr_group->state, accepted));
            backtracking_stack.back().group_closed = true;
            switch (curr_group->state->type) {
               case state_type::capture_group:
                  curr_group->u.capture.end = history_itr.char_index();
                  break;
               default:
                  break;
            }
            next_state = curr_group->state->next;
            curr_group = curr_group->parent;
         }
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

               case state_type::capture_group:
                  if (backtrack.group_closed) {
                     // Re-enter the last (closed) nested capture.
                     curr_group = curr_group->last_nested;
                  } else {
                     // Discard *curr_group and move back to its parent.
                     curr_group = curr_group->delete_and_get_parent();
                  }
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

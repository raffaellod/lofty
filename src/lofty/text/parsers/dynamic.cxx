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
   union {
      //! Pointer to the state the backtrack refers to.
      dynamic::state const * state;
      //! Pointer to the group the backtrack refers to.
      dynamic::match::group_node * group;
   } u;
   //! true if u contains a group, or false if it contains a state.
   bool u_is_group;
   //! true if *state accepted the input, or false otherwise.
   bool accepted;
   //! true if entering a group, false if leaving it.
   bool entered_group;

   /*! Constructor for states.

   @param state
      Pointer to the state the backtrack refers to.
   @param accepted_
      true if *state accepted the input, or false otherwise.
   */
   backtrack(dynamic::state const * state, bool accepted_) :
      u_is_group(false),
      accepted(accepted_) {
      u.state = state;
   }

   /*! Constructor for groups.

   @param group
      Pointer to the group the backtrack refers to.
   @param entered_group_
      true if entering a group, false if leaving it.
   @param accepted_
      true if *group->state accepted the input, or false otherwise.
   */
   backtrack(dynamic::match::group_node * group, bool entered_group_, bool accepted_) :
      u_is_group(true),
      accepted(accepted_),
      entered_group(entered_group_) {
      u.group = group;
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
         //! true if the group is being repeatedly matched, or false if the parser has moved on.
         bool counting;
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

dynamic::state * dynamic::create_repetition_group(
   state const * first_state, std::uint16_t min, std::uint16_t max /*= 0*/
) {
   state * ret = create_uninitialized_state(state_type::repetition_group);
   ret->u.repetition.first_state = first_state;
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

   // TODO: change this variable to use collections::stack once that’s available.
   collections::vector<backtrack> backtracking_stack;

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
            // Nest this capture into the currently-open group.
            curr_group = curr_group->append_nested(curr_state);
            curr_group->u.capture.begin = history_itr.char_index();
            next_state = curr_state->u.capture.first_state;
            backtracking_stack.push_back(backtrack(curr_group, true /*entering group*/, accepted));
            goto next_state_after_accepted;

         case state_type::repetition_group:
            // Nest this repetition into the currently-open group.
            curr_group = curr_group->append_nested(curr_state);
            curr_group->u.repetition.count = 0;
            curr_group->u.repetition.counting = true;
            accepted = (curr_state->u.repetition.min == 0);
            if (!accepted || curr_state->u.repetition.greedy) {
               // Want (greedy) or need (min reps > 0) at least one repetition.
               next_state = curr_state->u.repetition.first_state;
               /* Else, we’ll move on to next for now; later, if the input is not accepted, we’ll try with
               more repetitions. */
            }
            backtracking_stack.push_back(backtrack(curr_group, true /*entering group*/, accepted));
            goto next_state_after_accepted;
      }

      if (accepted) {
         backtracking_stack.push_back(backtrack(curr_state, accepted));
next_state_after_accepted:
         // The lack of a next state in a non-topmost group means the end of the current group.
         while (!next_state && curr_group->parent) {
            auto prev_group = curr_group;
            auto group_state = curr_group->state;
            switch (group_state->type) {
               case state_type::capture_group:
                  curr_group->u.capture.end = history_itr.char_index();
                  curr_group = curr_group->parent;
                  next_state = group_state->next;
                  break;

               case state_type::repetition_group:
                  ++curr_group->u.repetition.count;
                  // Repetitions within [min, max] are accepting.
                  accepted = (
                     curr_group->u.repetition.count >= static_cast<unsigned>(group_state->u.repetition.min) &&
                     (
                        group_state->u.repetition.max == 0 ||
                        curr_group->u.repetition.count <= static_cast<unsigned>(group_state->u.repetition.max)
                     )
                  );
                  if (!accepted || (group_state->u.repetition.greedy && (
                     group_state->u.repetition.max == 0 ||
                     curr_group->u.repetition.count < static_cast<unsigned>(group_state->u.repetition.max)
                  ))) {
                     // Want (greedy) or need (min reps > count) at least one more repetition.
                     curr_group->u.repetition.counting = true;
                     next_state = group_state->u.repetition.first_state;
                     // Stay in curr_group.
                  } else {
                     curr_group->u.repetition.counting = false;
                     next_state = group_state->next;
                     curr_group = curr_group->parent;
                  }
                  break;

               default:
                  break;
            }
            // Inject one more backtrack to allow getting back in the group.
            backtracking_stack.push_back(backtrack(prev_group, false /*leaving group*/, accepted));
         }
         curr_state = next_state;
      } else {
         // Consider the next alternative of the current state or a prior one.
         curr_state = curr_state->alternative;
         while (!curr_state && backtracking_stack) {
            auto const & backtrack = backtracking_stack.back();
            auto backtrack_state = backtrack.u_is_group ? backtrack.u.group->state : backtrack.u.state;
            switch (backtrack_state->type) {
               case state_type::range:
                  /* If the state we’re rolling back accepted (and consumed) a code point, it must’ve saved it
                  in history_buf, so recover it from there. */
                  if (backtrack.accepted) {
                     --history_itr;
                  }
                  break;

               case state_type::repetition_group:
                  if (backtrack.accepted && curr_group->u.repetition.counting) {
                     /* Backtracking to an accepted repetition after a rejected one: leave the group and
                     continue, ending the repetitions for the group. */
                     curr_group->u.repetition.counting = false;
                     curr_group = curr_group->parent;
                     next_state = backtrack_state->next;
                     accepted = true;
                     /* Decide what to do depending on whether the number of repetitions is in the
                     acceptable range. */
                     goto next_state_after_accepted;
                  }
                  // Move on to the alternative, like non-repeating groups and states.
                  //no break

               case state_type::capture_group:
                  if (backtrack.entered_group) {
                     // Discard *curr_group and move back to its parent.
                     curr_group = curr_group->delete_and_get_parent();
                  } else {
                     // Re-enter the group.
                     curr_group = backtrack.u.group;
                  }
                  break;

               default:
                  // No special action needed.
                  break;
            }
            // Consider the alternative to the backtracked state and discard the backtrack.
            curr_state = backtrack_state->alternative;
            backtracking_stack.pop_back();
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
   if (accepted) {
      ret.capture0->u.capture.end = history_itr.char_index();
   } else {
      ret.capture0.reset();
   }
   return _std::move(ret);
}

}}} //namespace lofty::text::parsers

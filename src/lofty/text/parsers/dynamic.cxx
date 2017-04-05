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
      dynamic::_group_node * group;
   } u;
   //! true if u contains a group, or false if it contains a state.
   bool u_is_group;
   //! true if *state accepted the input, or false otherwise.
   bool accepted;
   //! true if entering a group, false if leaving it.
   bool entered_group;
   //! true if the backtrack has been rolled back to once. Only used for repetition groups.
   bool hit_once;

   /*! Constructor for states.

   @param state
      Pointer to the state the backtrack refers to.
   @param accepted_
      true if *state accepted the input, or false otherwise.
   */
   backtrack(dynamic::state const * state, bool accepted_) :
      u_is_group(false),
      accepted(accepted_),
      hit_once(false) {
      u.state = state;
   }

   /*! Constructor for groups.

   @param group_node
      Pointer to the group the backtrack refers to.
   @param entering_group
      true if entering a group, false if leaving it.
   @param accepted_
      true if *group->state accepted the input, or false otherwise.
   */
   backtrack(dynamic::_group_node * group_node, bool entering_group, bool accepted_) :
      u_is_group(true),
      accepted(accepted_),
      entered_group(entering_group),
      hit_once(false) {
      u.group = group_node;
   }
};

} //namespace


struct dynamic::_group_node {
   //! Pointer to the related group state.
   struct state const * state;
   //! Pointer to the parent (containing) group. Only nullptr for capture 0.
   _group_node * parent;
   //! Owning pointer to the first nested group, if any.
   _std::unique_ptr<_group_node> first_nested;
   //! Non-owning pointer to the last nested group, if any.
   _group_node * last_nested;
   //! Non-owning pointer to the previous same-level group, if any.
   _group_node * prev_sibling;
   //! Owning pointer to the next same-level group, if any.
   _std::unique_ptr<_group_node> next_sibling;

   //! Destructor.
   ~_group_node() {
      // last_nested is the only non-owning forward pointer, so it must be updated manually.
      if (parent && parent->last_nested == this) {
         parent->last_nested = prev_sibling;
      }
   }

   /*! Casts the group as a capture group, if applicable.

   @return
      Pointer to *this as a capture group, or nullptr if *this is not a _capture_group_node.
   */
   _capture_group_node * as_capture();

   /*! Casts the group as a repetition group, if applicable.

   @return
      Pointer to *this as a repetition group, or nullptr if *this is not a _repetition_group_node.
   */
   _repetition_group_node * as_repetition();

   /*! Deletes *this by resetting its owner’s pointer, and returns parent.

   @return
      Pointer to the parent of the deleted node.
   */
   _group_node * delete_and_get_parent() {
      auto ret = parent;
      if (prev_sibling) {
         prev_sibling->next_sibling.reset();
      } else {
         parent->first_nested.reset();
      }
      return ret;
   }

   /*! Determines whether *this is a _capture_group_node.

   @return
      true if *this is a _capture_group_node, or nullptr if it’s not.
   */
   bool is_capture() const {
      return state->type == state_type::capture_group;
   }

   /*! Determines whether *this is a _repetition_group_node.

   @return
      true if *this is a _repetition_group_node, or nullptr if it’s not.
   */
   bool is_repetition() const {
      return state->type == state_type::repetition_group;
   }

protected:
   /*! Constructor.

   @param state_
      Pointer to the related group state.
   */
   explicit _group_node(struct state const * state_) :
      state(state_),
      parent(nullptr),
      last_nested(nullptr),
      prev_sibling(nullptr) {
   }

   /*! Constructor that inserts the node as the last_nested of a parent node.

   @param parent_
      Pointer to the parent (containing) group.
   @param state_
      Pointer to the related group state.
   */
   _group_node(_group_node * parent_, struct state const * state_) :
      state(state_),
      parent(parent_),
      last_nested(nullptr),
      prev_sibling(parent->last_nested) {
      if (prev_sibling) {
         prev_sibling = parent->last_nested;
         parent->last_nested->next_sibling.reset(this);
      } else {
         parent->first_nested.reset(this);
      }
      parent->last_nested = this;
   }
};


struct dynamic::_capture_group_node : _group_node {
   //! Offset of the start of the capture.
   std::size_t begin;
   //! Offset of the end of the capture.
   std::size_t end;

   /*! Constructor.

   @param state_
      Pointer to the related group state.
   */
   explicit _capture_group_node(struct state const * state_) :
      _group_node(state_) {
   }

   /*! Constructor that inserts the node as the last_nested of a parent node.

   @param parent_
      Pointer to the parent (containing) group.
   @param state_
      Pointer to the related group state.
   */
   _capture_group_node(_group_node * parent_, struct state const * state_) :
      _group_node(parent_, state_) {
   }
};

dynamic::_capture_group_node * dynamic::_group_node::as_capture() {
   return is_capture() ? static_cast<_capture_group_node *>(this) : nullptr;
}


struct dynamic::_repetition_group_node : _group_node {
   //! Number of times the repetition has occurred.
   unsigned count;

   /*! Constructor that inserts the node as the last_nested of a parent node.

   @param parent_
      Pointer to the parent (containing) group.
   @param state_
      Pointer to the related group state.
   */
   _repetition_group_node(_group_node * parent_, struct state const * state_) :
      _group_node(parent_, state_) {
   }
};

dynamic::_repetition_group_node * dynamic::_group_node::as_repetition() {
   return is_repetition() ? static_cast<_repetition_group_node *>(this) : nullptr;
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
   state * ret = create_uninitialized_state(state_type::cp_range);
   ret->u.cp_range.first = cp;
   ret->u.cp_range.last = cp;
   return ret;
}

dynamic::state * dynamic::create_code_point_range_state(char32_t first_cp, char32_t last_cp) {
   state * ret = create_uninitialized_state(state_type::cp_range);
   ret->u.cp_range.first = first_cp;
   ret->u.cp_range.last = last_cp;
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

   auto curr_state = initial_state;
   // Cache this condition to quickly determine whether we’re allowed to skip input code points.
   bool begin_anchor = (curr_state && curr_state->type == state_type::begin && !curr_state->alternative);
   // Setup the two sources of code points: a history and a peek buffer from the input stream.
   str history_buf, peek_buf = istream->peek_chars(1);
   auto history_begin_itr(history_buf.cbegin()), history_end(history_buf.cend());
   auto history_itr(history_begin_itr), peek_itr(peek_buf.cbegin()), peek_end(peek_buf.cend());

   // Empty state to which to associate capture0. Only its type is used.
   static state const capture0_state = {
      /*type*/        state_type::capture_group,
      /*next*/        nullptr,
      /*alternative*/ nullptr,
      {
      }
   };
   _std::unique_ptr<_capture_group_node> capture0_group_node(new _capture_group_node(&capture0_state));
   auto capture0_begin = history_begin_itr;
   _group_node * curr_group = capture0_group_node.get();

   // TODO: change this variable to use collections::stack once that’s available.
   collections::vector<backtrack> backtracking_stack;

   bool accepted = (curr_state == nullptr);
   while (curr_state) {
      state const * next_state = curr_state->next;
      switch (curr_state->type) {
         case state_type::cp_range: {
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

            accepted = (cp >= curr_state->u.cp_range.first && cp <= curr_state->u.cp_range.last);
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

         case state_type::capture_group: {
            auto capture_group = new _capture_group_node(curr_group, curr_state);
            capture_group->begin = history_itr.char_index();
            curr_group = capture_group;
            next_state = curr_state->u.capture.first_state;
            backtracking_stack.push_back(backtrack(curr_group, true /*entering group*/, accepted));
            goto next_state_after_accepted;
         }

         case state_type::repetition_group: {
            auto repetition_group = new _repetition_group_node(curr_group, curr_state);
            repetition_group->count = 0;
            curr_group = repetition_group;
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
      }

      if (accepted) {
         backtracking_stack.push_back(backtrack(curr_state, accepted));
next_state_after_accepted:
         // The lack of a next state in a non-topmost group means the end of the current group.
         while (!next_state && curr_group->parent) {
            auto prev_group = curr_group;
            if (auto capture_group = curr_group->as_capture()) {
               capture_group->end = history_itr.char_index();
               next_state = curr_group->state->next;
               curr_group = curr_group->parent;
            } else if (auto repetition_group = curr_group->as_repetition()) {
               ++repetition_group->count;
               // Repetitions within [min, max] are accepting.
               accepted = (
                  repetition_group->count >= static_cast<unsigned>(curr_group->state->u.repetition.min) &&
                  (
                     curr_group->state->u.repetition.max == 0 ||
                     repetition_group->count <= static_cast<unsigned>(curr_group->state->u.repetition.max)
                  )
               );
               if (!accepted || (curr_group->state->u.repetition.greedy && (
                  curr_group->state->u.repetition.max == 0 ||
                  repetition_group->count < static_cast<unsigned>(curr_group->state->u.repetition.max)
               ))) {
                  // Want (greedy) or need (min reps > count) at least one more repetition.
                  next_state = curr_group->state->u.repetition.first_state;
                  // Stay in curr_group.
               } else {
                  next_state = curr_group->state->next;
                  curr_group = curr_group->parent;
               }
            }
            // Inject one more backtrack to allow getting back in the group.
            backtracking_stack.push_back(backtrack(prev_group, false /*leaving group*/, accepted));
         }
         curr_state = next_state;
      } else {
         // Consider the next alternative of the current state or a prior one.
         curr_state = curr_state->alternative;
         while (!curr_state && backtracking_stack) {
            auto & backtrack = backtracking_stack.back();
            auto backtrack_state = backtrack.u_is_group ? backtrack.u.group->state : backtrack.u.state;
            switch (backtrack_state->type) {
               case state_type::cp_range:
                  /* If the state we’re rolling back accepted (and consumed) a code point, it must’ve saved it
                  in history_buf, so recover it from there. */
                  if (backtrack.accepted) {
                     --history_itr;
                  }
                  break;

               case state_type::repetition_group: {
                  auto repetition_group = backtrack.u.group->as_repetition();
                  if (backtrack.accepted) {
                     if (!backtrack.hit_once) {
                        /* Backtracking to an accepted repetition after a rejected one: leave the group and
                        continue, ending the repetitions for the group. */
                        curr_group = backtrack.u.group->parent;
                        next_state = backtrack_state->next;
                        accepted = true;
                        backtrack.hit_once = true;
                        /* Decide what to do depending on whether the number of repetitions is in the
                        acceptable range. */
                        goto next_state_after_accepted;
                     }
                  }
                  --repetition_group->count;
                  // Move on to the alternative, like non-repeating groups and states.
                  //no break
               }

               case state_type::capture_group:
                  if (backtrack.entered_group) {
                     // Discard *backtrack.u.group and move back to its parent.
                     curr_group = backtrack.u.group->delete_and_get_parent();
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
            capture0_begin = history_itr;
            curr_state = initial_state;
         }
      }
   }
   if (accepted) {
      capture0_group_node->begin = capture0_begin.char_index();
      capture0_group_node->end = history_itr.char_index();
      return match(_std::move(history_buf), _std::move(capture0_group_node));
   } else {
      return match();
   }
}

}}} //namespace lofty::text::parsers

namespace lofty { namespace text { namespace parsers { namespace _pvt {

dynamic_match_capture dm_group::capture_group(unsigned index) const {
   for (
      dynamic::_group_node const * nested = group_node->first_nested.get();
      nested;
      nested = nested->next_sibling.get()
   ) {
      if (nested->is_capture()) {
         if (index == 0) {
            return dynamic_match_capture(nested);
         }
         --index;
      }
   }
   // TODO: throw logic_error or out_of_range.
   return dynamic_match_capture(nullptr);
}

dm_group::_repetition dm_group::repetition_group(unsigned index) const {
   for (
      dynamic::_group_node const * nested = group_node->first_nested.get();
      nested;
      nested = nested->next_sibling.get()
   ) {
      if (nested->is_repetition()) {
         if (index == 0) {
            return _repetition(nested);
         }
         --index;
      }
   }
   // TODO: throw logic_error or out_of_range.
   return _repetition(nullptr);
}


/*explicit*/ dm_group::_repetition::_repetition(dynamic::_group_node const * group_node_) :
   group_node(static_cast<dynamic::_repetition_group_node const *>(group_node_)) {
}

dm_group::_repetition_occurrence dm_group::_repetition::operator[](unsigned index) const {
   auto first_state = group_node->first_nested->state;
   for (
      dynamic::_group_node const * nested = group_node->first_nested.get();
      nested;
      nested = nested->next_sibling.get()
   ) {
      if (nested->state == first_state) {
         if (index == 0) {
            return _repetition_occurrence(nested);
         }
         --index;
      }
   }
   // TODO: throw logic_error or out_of_range.
   return _repetition_occurrence(nullptr);
}

std::size_t dm_group::_repetition::size() const {
   return group_node->count;
}

}}}} //namespace lofty::text::parsers::_pvt

namespace lofty { namespace text { namespace parsers {

std::size_t dynamic_match_capture::begin_char_index() const {
   return static_cast<dynamic::_capture_group_node const *>(group_node)->begin;
}

std::size_t dynamic_match_capture::end_char_index() const {
   return static_cast<dynamic::_capture_group_node const *>(group_node)->end;
}


dynamic::match::match(match && src) :
   dynamic_match_capture(_std::move(src)),
   captures_buffer(_std::move(src.captures_buffer)) {
   src.group_node = nullptr;
}

dynamic::match::match(str && captures_buffer_, _std::unique_ptr<_capture_group_node const> && capture0) :
   dynamic_match_capture(capture0.get()),
   captures_buffer(_std::move(captures_buffer_)) {
   // Take ownership of the whole tree.
   capture0.release();
}

dynamic::match & dynamic::match::operator=(match && src) {
   dynamic_match_capture::operator=(_std::move(src));
   captures_buffer = _std::move(src.captures_buffer);
   src.group_node = nullptr;
   return *this;
}

dynamic::match::~match() {
   // This will cascade to delete the entire tree.
   delete group_node;
}

}}} //namespace lofty::text::parsers

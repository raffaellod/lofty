/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/collections.hxx>
#include <lofty/collections/vector.hxx>
#include <lofty/io/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

namespace {

//! Backtracking data structure.
struct backtrack {
   union {
      //! Pointer to the state the backtrack refers to.
      dynamic_state const * state;
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
   backtrack(dynamic_state const * state, bool accepted_) :
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


class dynamic::_group_node {
public:
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
   explicit _group_node(dynamic_state const * state_) :
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
   _group_node(_group_node * parent_, dynamic_state const * state_) :
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

public:
   //! Pointer to the related group state.
   dynamic_state const * state;
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
};


class dynamic::_capture_group_node : public _group_node {
public:
   /*! Constructor.

   @param state_
      Pointer to the related group state.
   */
   explicit _capture_group_node(dynamic_state const * state_) :
      _group_node(state_) {
   }

   /*! Constructor that inserts the node as the last_nested of a parent node.

   @param parent_
      Pointer to the parent (containing) group.
   @param state_
      Pointer to the related group state.
   */
   _capture_group_node(_group_node * parent_, dynamic_state const * state_) :
      _group_node(parent_, state_) {
   }

   /*! Returns the size of the capture.

   @return
      Length of the capture, in characters.
   */
   std::size_t size() const {
      return end - begin;
   }

public:
   //! Offset of the start of the capture.
   std::size_t begin;
   //! Offset of the end of the capture.
   std::size_t end;
};

dynamic::_capture_group_node * dynamic::_group_node::as_capture() {
   return is_capture() ? static_cast<_capture_group_node *>(this) : nullptr;
}


class dynamic::_repetition_group_node : public _group_node {
public:
   /*! Constructor that inserts the node as the last_nested of a parent node.

   @param parent_
      Pointer to the parent (containing) group.
   @param state_
      Pointer to the related group state.
   */
   _repetition_group_node(_group_node * parent_, dynamic_state const * state_) :
      _group_node(parent_, state_) {
   }

public:
   //! Number of times the repetition has occurred.
   unsigned count;
};

dynamic::_repetition_group_node * dynamic::_group_node::as_repetition() {
   return is_repetition() ? static_cast<_repetition_group_node *>(this) : nullptr;
}


dynamic::dynamic() :
   initial_state(nullptr) {
}

dynamic::dynamic(dynamic && src) :
   owned_states(_std::move(src.owned_states)),
   initial_state(src.initial_state) {
   src.initial_state = nullptr;
}

dynamic::~dynamic() {
}

dynamic_state * dynamic::create_begin_state() {
   return create_owned_state<_state_begin_data>();
}

dynamic_state * dynamic::create_capture_group(dynamic_state const * first_state) {
   auto ret = create_owned_state<_state_capture_group_data>();
   ret->first_state = first_state;
   return ret;
}

dynamic_state * dynamic::create_code_point_state(char32_t cp) {
   auto ret = create_owned_state<_state_cp_range_data>();
   ret->first = cp;
   ret->last = cp;
   return ret;
}

dynamic_state * dynamic::create_code_point_range_state(char32_t first_cp, char32_t last_cp) {
   auto ret = create_owned_state<_state_cp_range_data>();
   ret->first = first_cp;
   ret->last = last_cp;
   return ret;
}

dynamic_state * dynamic::create_end_state() {
   return create_owned_state<_state_end_data>();
}

dynamic_state * dynamic::create_repetition_group(
   dynamic_state const * first_state, std::uint16_t min, std::uint16_t max /*= 0*/
) {
   auto ret = create_owned_state<_state_repetition_group_data>();
   ret->first_state = first_state;
   ret->min = min;
   ret->max = max;
   ret->greedy = true;
   return ret;
}

dynamic_state * dynamic::create_string_state(str const * s) {
   auto ret = create_owned_state<_state_string_data>();
   ret->begin = s->data();
   ret->end = s->data_end();
   return ret;
}

dynamic_state * dynamic::create_string_state(char_t const * begin, char_t const * end) {
   auto ret = create_owned_state<_state_string_data>();
   ret->begin = begin;
   ret->end = end;
   return ret;
}

void dynamic::dump() const {
   collections::vector<dynamic_state const *> context_stack;
   auto curr_state = initial_state;
   auto out(io::text::stderr);
   while (curr_state) {
      // TODO: find a reasonable way to display alternatives.
      auto next_state = curr_state->next;
      for (unsigned i = 0; i < static_cast<unsigned>(context_stack.size()); ++i) {
         out->print(LOFTY_SL("   "));
      }
      out->print(LOFTY_SL("{}"), state_type(curr_state->type));
      switch (curr_state->type) {
         case state_type::capture_group: {
            auto state_with_data = curr_state->with_data<_state_capture_group_data>();
            context_stack.push_back(next_state);
            next_state = state_with_data->first_state;
            break;
         }

         case state_type::cp_range: {
            auto state_with_data = curr_state->with_data<_state_cp_range_data>();
            out->print(LOFTY_SL(" [‘{}’-‘{}’]"), state_with_data->first, state_with_data->last);
            break;
         }

         case state_type::repetition_group: {
            auto state_with_data = curr_state->with_data<_state_repetition_group_data>();
            out->print(LOFTY_SL(" [{}-{}]"), state_with_data->min, state_with_data->max);
            context_stack.push_back(next_state);
            next_state = state_with_data->first_state;
            break;
         }

         case state_type::string: {
            auto state_with_data = curr_state->with_data<_state_string_data>();
            out->print(LOFTY_SL(" “{}”"), str(external_buffer, state_with_data->begin, state_with_data->size()));
            break;
         }

         default:
            break;
      }
      out->print(LOFTY_SL(" @ {}\n"), curr_state);
      while (!next_state && context_stack) {
         next_state = context_stack.pop_back();
      }
      curr_state = next_state;
   }
}

dynamic::match dynamic::run(str const & s) const {
   io::text::str_istream istream(external_buffer, &s);
   return run(&istream);
}

dynamic::match dynamic::run(io::text::istream * istream) const {
   LOFTY_TRACE_FUNC(this, istream);

   auto curr_state = initial_state;
   // Cache this condition to quickly determine whether we’re allowed to skip input code points…
   bool begin_anchor = (curr_state && curr_state->type == state_type::begin && !curr_state->alternative);
   // …and this one to remember how many.
   unsigned skipped_input_cps = 0;
   // Setup the buffer into which code points are read from the input stream.
   str buf = istream->peek_chars(1);
   auto buf_itr(buf.cbegin()), buf_end(buf.cend());

   // Empty state to which to associate capture0. Only its type is used.
   static dynamic_state const capture0_state = {
      /*type*/        state_type::capture_group,
      /*next*/        nullptr,
      /*alternative*/ nullptr
   };
   _std::unique_ptr<_capture_group_node> capture0_group_node(new _capture_group_node(&capture0_state));
   _group_node * curr_group = capture0_group_node.get();

   // TODO: change this variable to use collections::stack once that’s available.
   collections::vector<backtrack> backtracking_stack;

   bool accepted = (curr_state == nullptr);
   while (curr_state) {
      dynamic_state const * next_state = curr_state->next;
      switch (curr_state->type) {
         case state_type::begin:
            accepted = (skipped_input_cps == 0 && buf_itr.char_index() == 0);
            break;

         case state_type::capture_group: {
            auto state_with_data = curr_state->with_data<_state_capture_group_data>();
            // Pointed-to object is owned by curr_group.
            auto capture_group = new _capture_group_node(curr_group, state_with_data);
            capture_group->begin = buf_itr.char_index();
            curr_group = capture_group;
            next_state = state_with_data->first_state;
            backtracking_stack.push_back(backtrack(curr_group, true /*entering group*/, accepted));
            goto next_state_after_accepted;
         }

         case state_type::cp_range: {
            auto state_with_data = curr_state->with_data<_state_cp_range_data>();
            // Get a code point from the buffer, peeking more code points if needed.
            if (buf_itr == buf_end) {
               buf = istream->peek_chars(buf.size_in_chars() + 1);
               buf_end = buf.cend();
               if (buf_itr == buf_end) {
                  // Run out of code points.
                  accepted = false;
                  break;
               }
            }
            char32_t cp = *buf_itr;
            accepted = (cp >= state_with_data->first && cp <= state_with_data->last);
            if (accepted) {
               ++buf_itr;
            }
            break;
         }

         case state_type::end:
            if (buf_itr == buf_end) {
               /* We parsed everything we peeked, but “end” really means end, so check that the stream is
               really empty. */
               buf = istream->peek_chars(buf.size_in_chars() + 1);
               buf_end = buf.cend();
               accepted = (buf_itr == buf_end);
            } else {
               accepted = false;
            }
            break;

         case state_type::repetition_group: {
            auto state_with_data = curr_state->with_data<_state_repetition_group_data>();
            // Pointed-to object is owned by curr_group.
            auto repetition_group = new _repetition_group_node(curr_group, state_with_data);
            repetition_group->count = 0;
            curr_group = repetition_group;
            accepted = (state_with_data->min == 0);
            if (!accepted || state_with_data->greedy) {
               // Want (greedy) or need (min reps > 0) at least one repetition.
               next_state = state_with_data->first_state;
               /* Else, we’ll move on to next for now; later, if the input is not accepted, we’ll try with
               more repetitions. */
            }
            backtracking_stack.push_back(backtrack(curr_group, true /*entering group*/, accepted));
            goto next_state_after_accepted;
         }

         case state_type::string: {
            auto state_with_data = curr_state->with_data<_state_string_data>();
            std::size_t needed_min_buf_char_size = state_with_data->size() + buf_itr.char_index();
            if (needed_min_buf_char_size > buf.size_in_chars()) {
               // Try to enlarge buf so that it has enough characters to compare to the string to match.
               buf = istream->peek_chars(needed_min_buf_char_size);
               buf_end = buf.cend();
               accepted = (buf.size() >= needed_min_buf_char_size);
               if (!accepted) {
                  break;
               }
            }
            accepted = (str_traits::compare(
               buf_itr.ptr(), buf.data() + needed_min_buf_char_size,
               state_with_data->begin, state_with_data->end
            ) == 0);
            if (accepted) {
               buf_itr = buf.iterator_from_char_index(buf_itr.char_index() + state_with_data->size());
            }
            break;
         }
      }

      if (accepted) {
         backtracking_stack.push_back(backtrack(curr_state, accepted));
next_state_after_accepted:
         // The lack of a next state in a non-topmost group means the end of the current group.
         while (!next_state && curr_group->parent) {
            auto prev_group = curr_group;
            if (auto capture_group = curr_group->as_capture()) {
               capture_group->end = buf_itr.char_index();
               next_state = curr_group->state->next;
               curr_group = curr_group->parent;
            } else if (auto repetition_group = curr_group->as_repetition()) {
               auto state_with_data = curr_group->state->with_data<_state_repetition_group_data>();
               ++repetition_group->count;
               // Repetitions within [min, max] are accepting.
               accepted = (
                  repetition_group->count >= static_cast<unsigned>(state_with_data->min) &&
                  (
                     state_with_data->max == 0 ||
                     repetition_group->count <= static_cast<unsigned>(state_with_data->max)
                  )
               );
               if (!accepted || (state_with_data->greedy && (
                  state_with_data->max == 0 ||
                  repetition_group->count < static_cast<unsigned>(state_with_data->max)
               ))) {
                  // Want (greedy) or need (min reps > count) at least one more repetition.
                  next_state = state_with_data->first_state;
                  // Stay in curr_group.
               } else {
                  next_state = state_with_data->next;
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
                  /* If the state we’re rolling back accepted (and consumed) a code point, rewind the iterator
                  to pretend that never happened. */
                  if (backtrack.accepted) {
                     --buf_itr;
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

               case state_type::string:  {
                  auto state_with_data = backtrack_state->with_data<_state_string_data>();
                  // Move the iterator back, pretending the string was never consumed.
                  buf_itr = buf.iterator_from_char_index(buf_itr.char_index() - state_with_data->size());
                  break;
               }

               default:
                  // No special action needed.
                  break;
            }
            // Consider the alternative to the backtracked state and discard the backtrack.
            curr_state = backtrack_state->alternative;
            backtracking_stack.pop_back();
         }
         /* If we run out of alternatives and can’t backtrack any further, and the pattern is not anchored, we
         can skip (discard) one input code point and try the whole pattern again from the initial state. */
         if (!curr_state && !begin_anchor && buf) {
            istream->consume_chars((buf.cbegin() + 1).char_index());
            ++skipped_input_cps;
            buf = istream->peek_chars(1);
            // buf_itr already rewound to index 0, so leave it there.
            buf_end = buf.cend();
            curr_state = initial_state;
         }
      }
   }
   if (accepted) {
      // Retain and consume the accepted part of buf.
      /* TODO: this could be optimized to make no copy of buf once vextr external_buffer can keep the buffer
      alive. */
      buf = str(buf.data(), buf_itr.ptr());
      istream->consume_chars(buf.size_in_chars());
      // Repurpose capture 0’s begin to record the offset of the match in the input.
      capture0_group_node->begin = skipped_input_cps;
      capture0_group_node->end = skipped_input_cps + buf_itr.char_index();
      return match(_std::move(buf), _std::move(capture0_group_node));
   } else {
      return match();
   }
}

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

dynamic_match_capture dynamic_match_repetition::occurrence::capture_group(unsigned index) const {
   LOFTY_TRACE_FUNC(this, index);

   auto requested_index = static_cast<std::ptrdiff_t>(index);
   auto first_state = first_group_node->state;
   for (auto curr = first_group_node; curr; ) {
      if (curr->is_capture()) {
         if (index == 0) {
            return dynamic_match_capture(match, curr);
         }
         --index;
      }
      curr = curr->next_sibling.get();
      if (curr->state == first_state) {
         break;
      }
   }
   LOFTY_THROW(collections::out_of_range, (requested_index, 0, requested_index - 1));
}

dynamic_match_repetition dynamic_match_repetition::occurrence::repetition_group(unsigned index) const {
   LOFTY_TRACE_FUNC(this, index);

   auto requested_index = static_cast<std::ptrdiff_t>(index);
   auto first_state = first_group_node->state;
   for (auto curr = first_group_node; curr; ) {
      if (curr->is_repetition()) {
         if (index == 0) {
            return dynamic_match_repetition(match, curr);
         }
         --index;
      }
      curr = curr->next_sibling.get();
      if (curr->state == first_state) {
         break;
      }
   }
   LOFTY_THROW(collections::out_of_range, (requested_index, 0, requested_index - 1));
}


/*explicit*/ dynamic_match_repetition::dynamic_match_repetition(
   dynamic::match const * match_, dynamic::_group_node const * group_node_
) :
   match(match_),
   group_node(static_cast<dynamic::_repetition_group_node const *>(group_node_)) {
}

dynamic_match_repetition::occurrence dynamic_match_repetition::operator[](std::size_t index) const {
   LOFTY_TRACE_FUNC(this, index);

   auto requested_index = static_cast<std::ptrdiff_t>(index);
   auto first_state = group_node->first_nested->state;
   for (auto nested = group_node->first_nested.get(); nested; nested = nested->next_sibling.get()) {
      if (nested->state == first_state) {
         if (index == 0) {
            return occurrence(match, nested);
         }
         --index;
      }
   }
   LOFTY_THROW(collections::out_of_range, (requested_index, 0, requested_index - 1));
}

std::size_t dynamic_match_repetition::size() const {
   return group_node->count;
}

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

std::size_t dynamic_match_capture::begin_char_index() const {
   LOFTY_TRACE_FUNC(this);

   auto capture_group_ = static_cast<dynamic::_capture_group_node const *>(group_node);
   return (match ? match->begin_char_index() : 0) + capture_group_->begin;
}

dynamic_match_capture dynamic_match_capture::capture_group(unsigned index) const {
   LOFTY_TRACE_FUNC(this, index);

   auto requested_index = static_cast<std::ptrdiff_t>(index);
   for (auto nested = group_node->first_nested.get(); nested; nested = nested->next_sibling.get()) {
      if (nested->is_capture()) {
         if (index == 0) {
            return dynamic_match_capture(match ? match : static_cast<dynamic::match const *>(this), nested);
         }
         --index;
      }
   }
   LOFTY_THROW(collections::out_of_range, (requested_index, 0, requested_index - 1));
}

std::size_t dynamic_match_capture::end_char_index() const {
   LOFTY_TRACE_FUNC(this);

   auto capture_group_ = static_cast<dynamic::_capture_group_node const *>(group_node);
   return (match ? match->begin_char_index() : 0) + capture_group_->end;
}

dynamic_match_repetition dynamic_match_capture::repetition_group(unsigned index) const {
   LOFTY_TRACE_FUNC(this, index);

   auto requested_index = static_cast<std::ptrdiff_t>(index);
   for (auto nested = group_node->first_nested.get(); nested; nested = nested->next_sibling.get()) {
      if (nested->is_repetition()) {
         if (index == 0) {
            return dynamic_match_repetition(match ? match : static_cast<dynamic::match const *>(this), nested);
         }
         --index;
      }
   }
   LOFTY_THROW(collections::out_of_range, (requested_index, 0, requested_index - 1));
}

text::str dynamic_match_capture::str() const {
   LOFTY_TRACE_FUNC(this);

   char_t const * ret_begin;
   std::size_t ret_size;
   if (match) {
      auto capture_group_ = static_cast<dynamic::_capture_group_node const *>(group_node);
      ret_begin = match->captures_buffer.data() + capture_group_->begin;
      ret_size = capture_group_->size();
   } else {
      // Upcast and return all of capture 0.
      auto const & capture_0 = static_cast<dynamic::match const *>(this)->captures_buffer;
      ret_begin = capture_0.data();
      ret_size = capture_0.size_in_chars();
   }
   return text::str(external_buffer, ret_begin, ret_size);
}

text::str dynamic_match_capture::str_copy() const {
   auto s(str());
   return text::str(s.data(), s.data_end());
}

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

dynamic::match::match(
   text::str && captures_buffer_, _std::unique_ptr<_capture_group_node const> && capture0
) :
   dynamic_match_capture(nullptr, capture0.get()),
   captures_buffer(_std::move(captures_buffer_)) {
   // Take ownership of the whole tree.
   capture0.release();
}

dynamic::match::match(match && src) :
   dynamic_match_capture(nullptr, src.group_node),
   captures_buffer(_std::move(src.captures_buffer)) {
   src.group_node = nullptr;
}

dynamic::match & dynamic::match::operator=(match && src) {
   group_node = src.group_node;
   src.group_node = nullptr;
   captures_buffer = _std::move(src.captures_buffer);
   return *this;
}

dynamic::match::~match() {
   // This will cascade to delete the entire tree.
   delete group_node;
}

void dynamic::match::dump() const {
   collections::vector<dynamic::_group_node const *> context_stack;
   dynamic::_group_node const * curr_group = group_node;
   auto out(io::text::stderr);
   while (curr_group) {
      // TODO: find a reasonable way to display alternatives.
      dynamic::_group_node const * next_group = curr_group->next_sibling.get();
      for (unsigned i = 0; i < static_cast<unsigned>(context_stack.size()); ++i) {
         out->print(LOFTY_SL("   "));
      }
      out->print(LOFTY_SL("{} @ {}\n"), state_type(curr_group->state->type), curr_group->state);
      if (curr_group->first_nested) {
         context_stack.push_back(next_group);
         next_group = curr_group->first_nested.get();
      }
      while (!next_group && context_stack) {
         next_group = context_stack.pop_back();
      }
      curr_group = next_group;
   }
}

}}} //namespace lofty::text::parsers

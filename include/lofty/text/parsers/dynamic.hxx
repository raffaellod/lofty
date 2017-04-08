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

#ifndef _LOFTY_TEXT_PARSERS_DYNAMIC_HXX
#define _LOFTY_TEXT_PARSERS_DYNAMIC_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/vector.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

/*! Parser that accepts input based on a dynamically-configurable state machine.

For the ERE pattern “a”, the state machine would be:

   @verbatim
   ┌───┬───────┐
   │“a”│nullptr│
   └───┴───────┘
   @endverbatim
*/
class LOFTY_SYM dynamic {
public:
   //! Tree node with extra data to track captures.
   class _capture_group_node;
   //! Base tree node.
   class _group_node;
   /*! Match returned by run(), providing access to the matched groups. It is the only owner of all resources
   related to a match. */
   class match;
   //! Tree node with extra data to track repetitions.
   class _repetition_group_node;

   //! Possible state types.
   LOFTY_ENUM_AUTO_VALUES(state_type,
      //! Begin matcher: /^/ .
      begin,
      //! Capture group: /(…)/ .
      capture_group,
      //! Code point or code point range matcher: /a/ , /[a-z]/ , etc.
      cp_range,
      //! End matcher: /$/ .
      end,
      //! Repetition matcher; repeatedly matches the states that follow it: /.{n,m}/ .
      repetition_group,
      //! String matcher: /abc/ , /qwerty/ , etc. Improves performance compared to a chain of cp_range states.
      string
   );

   //! Combines state with additional state type-dependent data.
   template <typename T>
   struct _state_aggregator;

   /*! State representation. Instances can be statically allocated, or generated at run-time by calling one of
   the dynamic::create_*_state() methods. */
   struct state {
      //! State type.
      state_type::enum_type type;
      //! Pointer to the next state if this one accepts.
      state const * next;
      //! Pointer to an alternate state to try if this one does not accept.
      state const * alternative;

      /*! Assigns the state that will be tried if this one does not accept.

      @param alternative_
         Alternate state.
      @return
         this.
      */
      state * set_alternative(state const * alternative_) {
         alternative = alternative_;
         return this;
      }

      /*! Assigns the state that will follow if this one accepts.

      @param next_
         Next state.
      @return
         this.
      */
      state * set_next(state const * next_) {
         next = next_;
         return this;
      }

      /*! Casts this into a pointer of an aggregated type.

      @return
         Pointer to the aggregated state.
      */
      template <typename T>
      _state_aggregator<T> const * with_data() const {
         return static_cast<_state_aggregator<T> const *>(this);
      }
   };

   //! Begin additional state data.
   struct _state_begin_data {
      static state_type::enum_type const type = state_type::begin;
   };

   //! Capture additional state data.
   struct _state_capture_group_data {
      static state_type::enum_type const type = state_type::capture_group;

      //! Pointer to the first state whose matching input is to be captured.
      state const * first_state;
   };

   //! Code point range additional state data.
   struct _state_cp_range_data {
      static state_type::enum_type const type = state_type::cp_range;

      //! First code point accepted by the range.
      char32_t first;
      //! Last code point accepted by the range.
      char32_t last;
   };

   //! End additional state data.
   struct _state_end_data {
      static state_type::enum_type const type = state_type::end;
   };

   //! Repetition additional state data.
   struct _state_repetition_group_data {
      static state_type::enum_type const type = state_type::repetition_group;

      //! Pointer to the first state to match repeatedly.
      state const * first_state;
      //! Minimum number of repetitions needed to accept.
      std::uint16_t min;
      //! Maximum number of repetitions needed to accept.
      std::uint16_t max;
      //! If true, more repetitions than min will be attempted first.
      bool greedy;
   };

   //! String additional state data.
   struct _state_string_data {
      static state_type::enum_type const type = state_type::string;

      //! Pointer to the start of the string to match.
      char_t const * begin;
      //! Pointer to the end of the string to match.
      char_t const * end;
   };

   template <typename T>
   class _state_aggregator : public noncopyable, public state, public T {
   public:
      //! Constructor.
      explicit _state_aggregator() {
         state::type = T::type;
         next = nullptr;
         alternative = nullptr;
      }
   };

   //! Allows to statically assemble a structure with the same layout as _state_aggregator<T>.
   template <typename T>
   struct _static_state_aggregator {
      //! State base.
      state base;
      //! Additional state_type-specific data.
      T extra;
   };

public:
   //! Default constructor.
   dynamic();

   /*! Move constructor.

   @param src
      Source object.
   */
   dynamic(dynamic && src);

   //! Destructor.
   ~dynamic();

   /*! Creates a state that matches the start of the input.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_begin_state();

   /*! Creates a capture group.

   @param first_state
      Pointer to the first state in the group. The last state shall have nullptr as next state, so that the
      parser will resume from the capture group’s next state.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_capture_group(state const * first_state);

   /*! Creates a state that matches a specific code point.

   @param cp
      Code point to match.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_code_point_state(char32_t cp);

   /*! Creates a state that matches a code point from the specified inclusive range.

   @param first_cp
      First code point in the range.
   @param last_cp
      Last code point in the range.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_code_point_range_state(char32_t first_cp, char32_t last_cp);

   /*! Creates a state that matches the end of the input.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_end_state();

   /*! Creates a state that matches a number of repetitions of another state list. The last state in the list
   should have this new state assigned as its next.

   @param first_state
      Pointer to the first state to match repeatedly.
   @param min
      Minimum number of repetitions needed to accept.
   @param max
      Maximum number of repetitions needed to accept. If omitted or 0, there will be no upper limit to the
      number of repetitions.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_repetition_group(state const * first_state, std::uint16_t min, std::uint16_t max = 0);

   /*! Creates a state that matches the specified string.

   @param begin
      Pointer to the start of the string.
   @param end
      Pointer to the end of the string.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_string_state(char_t const * begin, char_t const * end);

   /*! Runs the parser against the specified string.

   @param s
      String to parse.
   @return
      Match result. Will evaluate to true if the contents of the stream were accepted by the parser, or false
      otherwise.
   */
   match run(str const & s) const;

   /*! Runs the parser against the specified text stream, consuming code points from it as necessary.

   @param istream
      Pointer to the stream to parse.
   @return
      Match result. Will evaluate to true if the contents of the stream were accepted by the parser, or false
      otherwise.
   */
   match run(io::text::istream * istream) const;

   /*! Assigns an initial state. If not called, the parser will remain empty, accepting all input.

   @param initial_state_
      Pointer to the new initial state.
   */
   void set_initial_state(state const * initial_state_) {
      initial_state = initial_state_;
   }

protected:
   /*! Creates an uninitialized parser state.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released by other code.
   */
   template <typename T>
   _state_aggregator<T> * create_owned_state() {
      _std::unique_ptr<_state_aggregator<T>> new_state(new _state_aggregator<T>());
      auto ret = new_state.get();
      owned_states.push_back(_std::move(new_state));
      return ret;
   }

protected:
   //! Keeps ownership of all dynamically-allocated states.
   collections::vector<_std::unique_ptr<state>> owned_states;
   //! Pointer to the initial state.
   state const * initial_state;
};

#define _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(extra_type, name, next, alternative) \
   static ::lofty::text::parsers::dynamic::_static_state_aggregator< \
      ::lofty::text::parsers::dynamic::extra_type \
   > const name = { \
      /*base*/ { \
         /*type       */ ::lofty::text::parsers::dynamic::extra_type::type, \
         /*next       */ next, \
         /*alternative*/ alternative \
      }, \
      /*extra*/ {

#define _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END() \
      } \
   }

#define LOFTY_TEXT_PARSERS_DYNAMIC_BEGIN_STATE(name, next, alternative) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_begin_data, name, next, alternative) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_CAPTURE_GROUP(name, next, alternative, first_state) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_capture_group_data, name, next, alternative) \
      /*first_state*/ first_state \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_CODEPOINT_STATE(name, next, alternative, cp) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_cp_range_data, name, next, alternative) \
      /*first*/ cp, \
      /*last */ cp \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_CP_RANGE_STATE(name, next, alternative, first_cp, last_cp) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_cp_range_data, name, next, alternative) \
      /*first*/ first_cp, \
      /*last */ last_cp \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_END_STATE(name, next, alternative) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_end_data, name, next, alternative) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(name, next, alternative, first_state, min, max) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_repetition_group_data, name, next, alternative) \
      /*first_state*/ first_state, \
      /*min        */ min, \
      /*max        */ max, \
      /*greedy     */ true \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()

#define LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_MIN_GROUP(name, next, alternative, first_state, min) \
   LOFTY_TEXT_PARSERS_DYNAMIC_REPETITION_GROUP(name, next, alternative, first_state, min, 0)

#define LOFTY_TEXT_PARSERS_DYNAMIC_STRING_STATE(name, next, alternative, begin, end) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_string_data, name, next, alternative) \
      /*begin*/ begin, \
      /*end  */ end \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_END()


//! Matched input captured by lofty::text::parsers::dynamic::run().
class dynamic_match_capture;

}}} //namespace lofty::text::parsers

namespace lofty { namespace text { namespace parsers { namespace _pvt {

//! Base class for dynamic_match_*, providing access to nested groups.
class LOFTY_SYM dm_group : public noncopyable {
public:
   //! Individual repetition occurrence.
   class _repetition_occurrence;

   //! Virtual array of repetition occurrences.
   class LOFTY_SYM _repetition : public noncopyable, public support_explicit_operator_bool<_repetition> {
   private:
      friend class dm_group;

   public:
      /*! Move constructor.

      @param src
         Source object.
      */
      _repetition(_repetition && src) :
         group_node(src.group_node) {
         src.group_node = nullptr;
      }

      /*! Element access operator.

      @param index
         Repetition occurrence index.
      @return
         Accessor to the index-th occurrence.
      */
      _repetition_occurrence operator[](unsigned index) const;

      /*! Boolean evaluation operator.

      @return
         true if the repetition occurred at least once, or false otherwise.
      */
      LOFTY_EXPLICIT_OPERATOR_BOOL() const {
         return size() > 0;
      }

      /*! Returns how many time the repetition occurred in the containing scope.

      @return
         Count of occurrences for the occurrence group.
      */
      std::size_t size() const;

      /* TODO: iterator starts from first_nested, and incrementing it makes it scan for the next occurrence of
      first_nested->state, which is the first group of the next repetition. */

   protected:
      /*! Constructor.

      @param group_node
         Pointer to the node containing data for the repetition group.
      */
      explicit _repetition(dynamic::_group_node const * group_node);

   protected:
      //! Pointer to the node containing data for the repetition group.
      dynamic::_repetition_group_node const * group_node;
   };

public:
   /*! Move constructor.

   @param src
      Source object.
   */
   dm_group(dm_group && src) :
      group_node(_std::move(src.group_node)) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   dm_group & operator=(dm_group && src) {
      group_node = _std::move(src.group_node);
      return *this;
   }

   /*! Returns the capture group inserted at the specified index in the parser state tree.

   @param index
      Capture group index.
   @return
      Corresponding capture group.
   */
   dynamic_match_capture capture_group(unsigned index) const;

   /*! Returns the repetition group inserted at the specified index in the parser state tree.

   @param index
      Repetition group index.
   @return
      Corresponding repetition group.
   */
   _repetition repetition_group(unsigned index) const;

protected:
   /*! Constructor.

   @param group_node
      Pointer to the node containing data for the group.
   */
   explicit dm_group(dynamic::_group_node const * group_node_) :
      group_node(group_node_) {
   }

protected:
   //! Pointer to the node containing data for the group.
   dynamic::_group_node const * group_node;
};

class LOFTY_SYM dm_group::_repetition_occurrence : public dm_group {
private:
   friend class dm_group;

public:
   /*! Move constructor.

   @param src
      Source object.
   */
   _repetition_occurrence(_repetition_occurrence && src) :
      dm_group(_std::move(src)) {
   }

protected:
   /*! Constructor.

   @param group_node
      Pointer to the node containing data for the group.
   */
   explicit _repetition_occurrence(dynamic::_group_node const * group_node_) :
      dm_group(group_node_) {
   }
};

}}}} //namespace lofty::text::parsers::_pvt

namespace lofty { namespace text { namespace parsers {

class LOFTY_SYM dynamic_match_capture : public _pvt::dm_group {
private:
   friend class _pvt::dm_group;

public:
   /*! Move constructor.

   @param src
      Source object.
   */
   dynamic_match_capture(dynamic_match_capture && src) :
      _pvt::dm_group(_std::move(src)) {
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   dynamic_match_capture & operator=(dynamic_match_capture && src) {
      _pvt::dm_group::operator=(_std::move(src));
      return *this;
   }

   /*! Returns the index of the character at the beginning of the capture.

   @return
      Character index.
   */
   std::size_t begin_char_index() const;

   /*! Returns the index of the character past the end of the capture.

   @return
      Character index.
   */
   std::size_t end_char_index() const;

protected:
   /*! Constructor.

   @param group_node
      Pointer to the node containing data for the group.
   */
   explicit dynamic_match_capture(dynamic::_group_node const * group_node_) :
      _pvt::dm_group(group_node_) {
   }
};

class LOFTY_SYM dynamic::match : public dynamic_match_capture, public support_explicit_operator_bool<match> {
private:
   friend match dynamic::run(io::text::istream * istream) const;

public:
   //! Constructor.
   match() :
      dynamic_match_capture(nullptr) {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   match(match && src);

   //! Destructor.
   ~match();

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   match & operator=(match && src);

   /*! Boolean evaluation operator.

   @return
      true if the input was accepted by the parser, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return group_node != nullptr;
   }

protected:
   /*! Constructor for use by the parser.

   @param captures_buffer
      Contains all captures, which are expressed as offset in this string.
   @param capture0_group_node
      Poimter to the top-level implicit capture.
   */
   match(str && captures_buffer, _std::unique_ptr<_capture_group_node const> && capture0_group_node);

protected:
   //! Contains all captures, which are expressed as offset in this string.
   str captures_buffer;
};

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_PARSERS_DYNAMIC_HXX

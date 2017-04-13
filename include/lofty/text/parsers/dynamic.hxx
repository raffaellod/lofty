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

/*! State representation. Instances can be statically allocated, or generated at run-time by calling one of
the lofty::text::parsers::dynamic::create_*_state() methods. */
struct dynamic_state {
public:
   //! Combines state with additional state type-dependent data.
   template <typename T>
   struct _aggregator;

   //! Possible state types.
   LOFTY_ENUM_AUTO_VALUES(_type,
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

public:
   //! State type.
   _type::enum_type type;
   //! Pointer to the next state if this one accepts.
   dynamic_state const * next;
   //! Pointer to an alternate state to try if this one does not accept.
   dynamic_state const * alternative;

public:
   /*! Assigns the state that will be tried if this one does not accept.

   @param alternative_
      Alternate state.
   @return
      this.
   */
   dynamic_state * set_alternative(dynamic_state const * alternative_) {
      alternative = alternative_;
      return this;
   }

   /*! Assigns the state that will follow if this one accepts.

   @param next_
      Next state.
   @return
      this.
   */
   dynamic_state * set_next(dynamic_state const * next_) {
      next = next_;
      return this;
   }

   /*! Casts this into a pointer of an aggregated type.

   @return
      Pointer to the aggregated state.
   */
   template <typename T>
   _aggregator<T> const * with_data() const {
      return static_cast<_aggregator<T> const *>(this);
   }
};

template <typename T>
class dynamic_state::_aggregator : public noncopyable, public dynamic_state, public T {
public:
   //! Default constructor.
   _aggregator() {
      dynamic_state::type = T::type;
      next = nullptr;
      alternative = nullptr;
   }
};

/*! Parser that accepts input based on a dynamically-configurable state machine.

For the ERE pattern “a”, the state machine would be:

   @verbatim
   ┌───┬───────┐
   │“a”│nullptr│
   └───┴───────┘
   @endverbatim
*/
class LOFTY_SYM dynamic {
private:
   //! Shortcut.
   typedef dynamic_state::_type state_type;

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

   //! Begin additional state data.
   struct _state_begin_data {
      static state_type::enum_type const type = state_type::begin;
   };

   //! Capture additional state data.
   struct _state_capture_group_data {
      static state_type::enum_type const type = state_type::capture_group;

      //! Pointer to the first state whose matching input is to be captured.
      dynamic_state const * first_state;
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
      dynamic_state const * first_state;
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

      /*! Returns the size of the string.

      @return
         Length of the string, in characters.
      */
      std::size_t size() const {
         return static_cast<std::size_t>(end - begin);
      }
   };

   //! Allows to statically assemble a structure with the same layout as state::_aggregator<T>.
   template <typename T>
   struct _static_state_aggregator {
      //! State base.
      dynamic_state base;
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
   dynamic_state * create_begin_state();

   /*! Creates a capture group.

   @param first_state
      Pointer to the first state in the group. The last state shall have nullptr as next state, so that the
      parser will resume from the capture group’s next state.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_capture_group(dynamic_state const * first_state);

   /*! Creates a state that matches a specific code point.

   @param cp
      Code point to match.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_code_point_state(char32_t cp);

   /*! Creates a state that matches a code point from the specified inclusive range.

   @param first_cp
      First code point in the range.
   @param last_cp
      Last code point in the range.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_code_point_range_state(char32_t first_cp, char32_t last_cp);

   /*! Creates a state that matches the end of the input.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_end_state();

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
   dynamic_state * create_repetition_group(
      dynamic_state const * first_state, std::uint16_t min, std::uint16_t max = 0
   );

   /*! Creates a state that matches the specified string. The string must remain accessible for the lifetime
   of the parser.

   @param s
      Pointer to the string.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_string_state(str const * s);

   /*! Creates a state that matches the specified char_t array.

   @param begin
      Pointer to the start of the array.
   @param end
      Pointer to the end of the array.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   dynamic_state * create_string_state(char_t const * begin, char_t const * end);

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
   void set_initial_state(dynamic_state const * initial_state_) {
      initial_state = initial_state_;
   }

protected:
   /*! Creates an uninitialized parser state.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released by other code.
   */
   template <typename T>
   dynamic_state::_aggregator<T> * create_owned_state() {
      _std::unique_ptr<dynamic_state::_aggregator<T>> new_state(new dynamic_state::_aggregator<T>());
      auto ret = new_state.get();
      owned_states.push_back(_std::move(new_state));
      return ret;
   }

protected:
   //! Keeps ownership of all dynamically-allocated states.
   collections::vector<_std::unique_ptr<dynamic_state>> owned_states;
   //! Pointer to the initial state.
   dynamic_state const * initial_state;
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

#define LOFTY_TEXT_PARSERS_DYNAMIC_STRING_STATE(name, next, alternative, str) \
   _LOFTY_TEXT_PARSERS_DYNAMIC_STATE_BEGIN(_state_string_data, name, next, alternative) \
      /*begin*/ str, \
      /*end  */ str + LOFTY_SL_SIZE(str) \
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
         match(src.match),
         group_node(src.group_node) {
         src.match = nullptr;
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

      @param match
         Pointer to the match, which contains the input offset and capture 0.
      @param group_node
         Pointer to the node containing data for the repetition group.
      */
      _repetition(dynamic::match const * match, dynamic::_group_node const * group_node);

   protected:
      //! Pointer to the match, which contains the input offset and capture 0.
      dynamic::match const * match;
      //! Pointer to the node containing data for the repetition group.
      dynamic::_repetition_group_node const * group_node;
   };

public:
   /*! Move constructor.

   @param src
      Source object.
   */
   dm_group(dm_group && src) :
      match(src.match),
      group_node(src.group_node) {
      src.match = nullptr;
      src.group_node = nullptr;
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   dm_group & operator=(dm_group && src) {
      match = src.match;
      group_node = src.group_node;
      src.match = nullptr;
      src.group_node = nullptr;
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

   @param match_
      Pointer to the match, which contains the input offset and capture 0.
   @param group_node_
      Pointer to the node containing data for the group.
   */
   dm_group(dynamic::match const * match_, dynamic::_group_node const * group_node_) :
      match(match_),
      group_node(group_node_) {
   }

protected:
   //! Pointer to the match, which contains the input offset and capture 0.
   dynamic::match const * match;
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

   @param match_
      Pointer to the match, which contains the input offset and capture 0.
   @param group_node_
      Pointer to the node containing data for the group.
   */
   _repetition_occurrence(dynamic::match const * match_, dynamic::_group_node const * group_node_) :
      dm_group(match_, group_node_) {
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

   /*! Returns a string containing the captured portion of the matched input. The string must not be stored
   anywhere, since its buffer has the same scope as the match object.

   @return
      Matched string.
   */
   text::str str() const;

   /*! Returns a string containing the captured portion of the matched input.

   @return
      Matched string.
   */
   text::str str_copy() const;

protected:
   /*! Constructor.

   @param match_
      Pointer to the match, which contains the input offset and capture 0.
   @param group_node_
      Pointer to the node containing data for the group.
   */
   dynamic_match_capture(dynamic::match const * match_, dynamic::_group_node const * group_node_) :
      _pvt::dm_group(match_, group_node_) {
   }
};

class LOFTY_SYM dynamic::match : public dynamic_match_capture, public support_explicit_operator_bool<match> {
private:
   friend text::str dynamic_match_capture::str() const;
   friend match dynamic::run(io::text::istream * istream) const;

public:
   //! Default constructor.
   match() :
      dynamic_match_capture(nullptr, nullptr) {
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
   match(text::str && captures_buffer, _std::unique_ptr<_capture_group_node const> && capture0_group_node);

protected:
   //! Contains all captures, which are expressed as offset in this string.
   text::str captures_buffer;
};

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_PARSERS_DYNAMIC_HXX

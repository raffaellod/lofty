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

#include <lofty/collections/queue.hxx>


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
   //! Possible state types.
   LOFTY_ENUM_AUTO_VALUES(state_type,
      //! Begin matcher: /^/ .
      begin,
      //! End matcher: /$/ .
      end,
      //! Look-ahead matcher: /(?=...)/ .
      look_ahead,
      //! Code point or code point range matcher: /a/ , /[a-z]/ , etc.
      range,
      //! Repetition matcher; repeatedly matches the states that follow it: /.{n,m}/ .
      repetition
   );

   /*! State representation. Instances can be statically allocated, or generated at run-time by calling one of
   the dynamic::create_*_state() methods. */
   struct state {
      /*! Assigns the state that will be tried if this one does not accept.

      @param alternative_
         Alternate state.
      @return
         this.
      */
      state * set_alternative(state * alternative_) {
         alternative = alternative_;
         return this;
      }

      /*! Assigns the state that will follow if this one accepts.

      @param next_
         Next state.
      @return
         this.
      */
      state * set_next(state * next_) {
         next = next_;
         return this;
      }

      //! Pointer to the next state if this one accepts.
      state const * next;
      //! Pointer to an alternate state to try if this one does not accept.
      state const * alternative;
      union {
         //! Range data.
         struct {
            //! First character accepted by the range.
            char32_t first_cp;
            //! Last character accepted by the range.
            char32_t last_cp;
         } range;
         //! Repetition data.
         struct {
            //! Pointer to the first state to be matched repeatedly.
            state const * repeated_state;
            //! Minimum number of repetitions needed to accept.
            std::uint16_t min;
            //! Maximum number of repetitions needed to accept.
            std::uint16_t max;
            //! If true, more repetitions than min will be attempted first.
            bool greedy;
         } repetition;
      } u;
      //! State type.
      state_type::enum_type type;
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
   state * create_begin_state() {
      return create_uninitialized_state(state_type::begin);
   }

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
   state * create_end_state() {
      return create_uninitialized_state(state_type::end);
   }

   /*! Creates a state that matches the end of the input.

   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_look_ahead_state(state const * repeated_state);

   /*! Creates a state that matches a number of repetitions of another state list. The last state in the list
   should have this new state assigned as its next.

   @param repeated_state
      Pointer to the first state of the repetition.
   @param min
      Minimum number of repetitions needed to accept.
   @param max
      Maximum number of repetitions needed to accept. If omitted or 0, there will be no upper limit to the
      number of repetitions.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_repetition_state(state const * repeated_state, std::uint16_t min, std::uint16_t max = 0);

   /*! Runs the parser against the specified string.

   @param s
      String to parse.
   @return
      true if the string is accepted by the parser, or false otherwise.
   */
   bool run(str const & s) const;

   /*! Runs the parser against the specified text stream, consuming code points from it as necessary.

   @param istream
      Pointer to the stream to parse.
   @return
      true if the contents of the stream were accepted by the parser, or false otherwise.
   */
   bool run(io::text::istream * istream) const;

   /*! Assigns an initial state. If not called, the parser will remain empty, accepting all input.

   @param initial_state_
      Pointer to the new initial state.
   */
   void set_initial_state(state const * initial_state_) {
      initial_state = initial_state_;
   }

protected:
   /*! Creates an uninitialized parser state.

   @param type
      Type of state to create.
   @return
      Pointer to the newly-created state, which is owned by the parser and must not be released.
   */
   state * create_uninitialized_state(state_type type);

protected:
   //! List of states.
   /* TODO: change this variable to use collections::forward_list; for now it’s a collections::queue just
   because collections::list is a doubly-linked list, which is unnecessary since we never remove states from
   the parser, only add. */
   collections::queue<state> states_list;
   //! Pointer to the initial state.
   state const * initial_state;
};

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_PARSERS_DYNAMIC_HXX

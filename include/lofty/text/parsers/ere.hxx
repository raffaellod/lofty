﻿/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017 Raffaello D. Di Napoli

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

#ifndef _LOFTY_TEXT_PARSERS_ERE_HXX
#define _LOFTY_TEXT_PARSERS_ERE_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <lofty/collections/vector.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

class ere_capture_format : public noncopyable {
public:
   //! Free-text expression, in a syntax dependent on the type (e.g. regex for lofty::text::str).
   str expr;
};

/*! Parses regular expressions with a syntax similar to POSIX Extended Regular Expression and Perl’s regular
expressions, generating a tree of states that will have the specified next state. The expression string must
remain accessible for the lifetime of the dynamic::parser instance.

The biggest difference between this class and other ERE/PCRE implementations is that capturing groups
specified with just “(…)” are to be parsed by the clients of this class.

TODO: (?D) for debugging (e.g. dump tree to stderr).

Current limitations:
•  Ranges are expected to be sorted (e.g. [ACGT], not [TAGC]);
•  Backreferences are not yet supported.

For compatibility with Python’s re module, these special groups shall not be used unless to implement
functionality identical to that of Python’s re module:

•  (…)
•  (?…)
•  (?aiLmsux)
•  (?:…)
•  (?P<name>…)
•  (?P=name)
•  (?#…)
•  (?=…)
•  (?!…)
•  (?<=…)
•  (?<!…)
•  (?(id/name)yes-pattern|no-pattern)

See also Python’s re module: <https://docs.python.org/3.5/library/re.html>.
See also POSIX ERE: <http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap09.html>.
*/
class LOFTY_SYM ere : public noncopyable {
private:
   /*! References one or more states owned by a dynamic parser, allowing for easy concatenation of multiple
   expressions. */
   struct LOFTY_SYM subexpression {
      //! Pointer to the first state.
      dynamic_state * first_state;
      //! Pointer to the first state of the current alternative.
      dynamic_state * curr_alternative_first_state;
      //! Pointer to the current state in the current alternative.
      dynamic_state * curr_state;
      //! List of the last state for all non-current alternatives.
      collections::vector<dynamic_state *, 2> alternative_last_states;

      //! Default constructor.
      subexpression();

      //! Destructor.
      ~subexpression();

      /*! Adds one more alternative (as last) to the first state.

      @param new_state
         Pointer to the first state of the alternative.
      */
      void push_alternative(dynamic_state * new_state);

      /*! Add one more state to the current alternative.

      @param new_state
         New state to add as next.
      */
      void push_next(dynamic_state * new_state);

      /*! Assigns a next state to all alternatives.

      @param next_state
         New next state.
      */
      void terminate_with_next_state(dynamic_state * next_state);
   };

public:
   /*! Constructor.

   @param parser_
      Parser to use to create states.
   @param expr_
      Expression to parse.
   */
   ere(dynamic * parser_, str const & expr_);

   //! Destructor.
   ~ere();

   /*! Returns the highest capture group index, equivalent to the count of capture groups minus one.

   @return
      Maximum capture group index.
   */
   int capture_index_max() const {
      return static_cast<int>(next_capture_index) - 1;
   }

   /*! Creates a new capture group for the specified state or tree of states, then inserts it as next state.

   @param first_state
      First state to be captured.
   */
   void insert_capture_group(dynamic_state const * first_state);

   /*! Parses or resumes parsing the expression, stopping at the first capture group or at the end of the
   expression.

   @param capture_format
      If the return value is 0 or greater, indicating that a capture group was found, the object pointed to by
      this argument will be set to the contents of the capture group parentheses, which is to be used as
      format in a way defined by the caller.
   @param first_state
      If the return value is less than 0, indicating that no capture groups were found, the pointer pointed
      to by this argument will be set to point to the first state generated by the parser. If the expression
      was an empty string, *first_state will be set to nullptr.
   @return
      If the parser found a capture group in the expression, the return value will be the index of the capture
      group and *capture_format will be set accordingly; if the parser reached the end of the expression, the
      return value will be less than 0.
   */
   int parse_up_to_next_capture(ere_capture_format * capture_format, dynamic_state ** first_state);

   /*! Parses the expression, expecting to find no capture groups in it.

   @return
      Pointer to the first state generated by the parser.
   */
   dynamic_state * parse_with_no_captures();

private:
   /*! Parses the contents of a capture group (i.e. the “…” in a “(…)”).

   @param format
      Pointer to an object that will receive the contents of the group.
   */
   void extract_capture(ere_capture_format * format);

   //! Throws a lofty::text::syntax_error for the current position in the expression (expr_itr).
   void throw_syntax_error(str const & description) const;

   //! Parses the contents of a negative bracket expression (i.e. the “…” in a “[^…]”).
   void parse_negative_bracket_expression();

   //! Parses the contents of a positive bracket expression (i.e. the “…” in a “[…]”).
   void parse_positive_bracket_expression();

   /*! Parses a repatition range (e.g. “{3}”, “{10,}”, “{1,6}”).

   @return
      Tuple containing the two inclusive bounds of the range, with 0 in place of any bounds missing in the
      expression.
   */
   _std::tuple<std::uint16_t, std::uint16_t> parse_repetition_range();

   /*! Pushes a next state, wil varying effects depending on the state of *this.

   @param next_state
      New state to push.
   */
   void push_state(dynamic_state * next_state);

   /*! Changes the number of min/max repetitions for the current repetition group. If the current state is not
   a repetition group, it is changed into one.

   @param min
      Minimum number of occurrences needed to satisfy the repetition.
   @param max
      Maximum number of occurrences needed to satisfy the repetition.
   */
   void set_curr_state_repetitions(std::uint16_t min, std::uint16_t max);

private:
   //! Pointer to the dynamic parser that will be used to create states.
   dynamic * parser;
   //! Reference to the original expression.
   str const & expr;
   //! Iterator to the next code point to be consumed from the expression.
   str::const_iterator expr_itr;
   //! Iterator to the end of the expression.
   str::const_iterator expr_end;
   /*! Tracks the last-closed group or range, or the previous state to support replacing it with a group if
   needed. */
   subexpression prev_subexpr;
   //! Tracks the context of the current subexpression, which is always subexpr_stack.back() .
   collections::vector<subexpression, 3> subexpr_stack;
   //! Index of the next capture group.
   unsigned next_capture_index;
   //! If true, the next call to push_state() will create and enter a non-capturing (repetition) group.
   bool enter_rep_group:1;
   //! If true, the next call to push_state() will terminate and pop the current subexpression.
   bool end_subexpr:1;
   //! If true, the next call to push_state() will add an alternative instead of a next state.
   bool begin_alternative:1;
};

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_PARSERS_ERE_HXX

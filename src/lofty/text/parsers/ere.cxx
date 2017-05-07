/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#include <lofty.hxx>
#include <lofty/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/ere.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

ere::subexpression::subexpression() :
   first_state(nullptr),
   curr_alternative_first_state(nullptr),
   curr_state(nullptr) {
}

ere::subexpression::~subexpression() {
}

void ere::subexpression::push_alternative(dynamic_state * new_state) {
   if (curr_state) {
      alternative_last_states.push_back(curr_state);
      curr_alternative_first_state->set_alternative(new_state);
   } else {
      first_state = new_state;
   }
   curr_state = new_state;
   curr_alternative_first_state = new_state;
}

void ere::subexpression::push_next(dynamic_state * new_state) {
   if (curr_state) {
      curr_state->set_next(new_state);
   } else {
      first_state = new_state;
      curr_alternative_first_state = new_state;
   }
   curr_state = new_state;
}

void ere::subexpression::terminate_with_next_state(dynamic_state * next_state) {
   if (curr_state) {
      alternative_last_states.push_back(curr_state);
      curr_state = nullptr;
   }
   LOFTY_FOR_EACH(auto & state, alternative_last_states) {
      state->set_next(next_state);
   }
}


ere::ere(dynamic * parser_, str const & expr_) :
   parser(parser_),
   expr(expr_),
   expr_itr(expr.cbegin()),
   expr_end(expr.cend()),
   next_capture_index(0),
   enter_rep_group(false),
   end_subexpr(false),
   begin_alternative(false) {
   subexpr_stack.push_back(subexpression());
}

ere::~ere() {
}

void ere::insert_capture_group(dynamic_state const * first_state) {
   LOFTY_TRACE_FUNC(this, first_state);

   push_state(parser->create_capture_group(first_state));
}

int ere::parse_up_to_next_capture(str * capture_format, dynamic_state ** first_state) {
   LOFTY_TRACE_FUNC(this, capture_format, first_state);

   bool escape = false;
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (escape) {
         escape = false;
         goto escaped;
      }
      switch (cp) {
         case '.':
            push_state(parser->create_code_point_range_state(0x000000, 0xffffff));
            break;
         case '[':
            if (expr_itr >= expr_end) {
               throw_syntax_error(LOFTY_SL("unexpected end of bracket expression"));
            }
            subexpr_stack.push_back(subexpression());
            if (*expr_itr == '^') {
               if (++expr_itr >= expr_end) {
                  throw_syntax_error(LOFTY_SL("unexpected end of negative bracket expression"));
               }
               parse_negative_bracket_expression();
            } else {
               parse_positive_bracket_expression();
            }
            end_subexpr = true;
            break;
         case '\\':
            escape = true;
            break;
         case '(': {
            if (expr_itr >= expr_end) {
               throw_syntax_error(LOFTY_SL("unexpected end of group"));
            }
            if (*expr_itr != '?') {
               extract_capture(capture_format);
               return static_cast<int>(next_capture_index++);
            }
            ++expr_itr;
            if (expr_itr >= expr_end) {
               throw_syntax_error(LOFTY_SL("unexpected end of non-capturing group"));
            }
            if (*expr_itr == ':') {
               ++expr_itr;
               enter_rep_group = true;
            } else {
               throw_syntax_error(LOFTY_SL("unsupported non-capturing group type"));
            }
            break;
         }
         case ')':
            end_subexpr = true;
            break;
         case '*':
            set_curr_state_repetitions(0, 0);
            break;
         case '+':
            set_curr_state_repetitions(1, 0);
            break;
         case '?':
            set_curr_state_repetitions(0, 1);
            break;
         case '{': {
            auto range(parse_repetition_range());
            set_curr_state_repetitions(_std::get<0>(range), _std::get<1>(range));
            break;
         }
         case '|':
            begin_alternative = true;
            break;
         case '^':
            push_state(parser->create_begin_state());
            break;
         case '$':
            push_state(parser->create_end_state());
            break;
         default:
         escaped:
            push_state(parser->create_code_point_state(cp));
            break;
      }
   }
   if (subexpr_stack.size() != 1) {
      throw_syntax_error(LOFTY_SL("mismatched parentheses"));
   }
   *first_state = subexpr_stack.front().first_state;
   // No need to terminate the last subexpression; it should be already nullptr-terminated.
   subexpr_stack.clear();
   return -1;
}

void ere::throw_syntax_error(str const & description) const {
   LOFTY_THROW(syntax_error, (
      // +1 because the first character is 1, to human beings.
      description, expr, static_cast<unsigned>(expr_itr - expr.cbegin() + 1)
   ));
}

void ere::extract_capture(str * format) {
   LOFTY_TRACE_FUNC(this, format);

   if (expr_itr != expr_end) {
      // The capture specifies a format.
      auto format_begin(expr_itr);
      bool escape = false;
      for (; expr_itr != expr_end; ++expr_itr) {
         if (escape) {
            escape = false;
         } else {
            char32_t cp = *expr_itr;
            if (cp == '\\') {
               escape = true;
            } else if (cp == ')') {
               break;
            }
         }
      }
      if (expr_itr == expr_end) {
         throw_syntax_error(LOFTY_SL("unterminated capturing group"));
      }
      *format = str(external_buffer, format_begin.ptr(), expr_itr.char_index() - format_begin.char_index());
   } else {
      format->clear();
   }

   if (expr_itr == expr_end || *expr_itr != ')') {
      throw_syntax_error(LOFTY_SL("unterminated capturing group"));
   }
   // Also consume the last parenthesis, so it won’t trigger end_subexpr in parse_up_to_next_capture().
   ++expr_itr;
}

void ere::parse_negative_bracket_expression() {
   LOFTY_TRACE_FUNC(this);

   char32_t next_range_begin = *expr_itr++ + 1;
   // Start with the first alternative.
   // TODO: this is not right if *expr_itr is NUL (rare).
   push_state(parser->create_code_point_range_state(0x000000, next_range_begin - 1));
   bool forming_range = false, escape = false;
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (cp == ']' && !escape) {
         if (forming_range) {
            // Turns out the dash/hyphen did not indicate a range; make two new states around it.
            auto & curr_subexpr = subexpr_stack.back();
            curr_subexpr.push_alternative(parser->create_code_point_range_state(next_range_begin, '-' - 1));
            curr_subexpr.push_alternative(parser->create_code_point_range_state('-' + 1, 0xffffff));
         }
         return;
      }
      if (forming_range) {
         forming_range = false;
         next_range_begin = cp + 1;
      } else {
         if (escape) {
            escape = false;
            goto escaped;
         }
         switch (cp) {
            case '-':
               forming_range = true;
               break;
            case '\\':
               escape = true;
               break;
            default:
            escaped:
               if (cp > next_range_begin) {
                  // Skipping one or more code points; make a new range for the gap.
                  auto & curr_subexpr = subexpr_stack.back();
                  curr_subexpr.push_alternative(parser->create_code_point_range_state(next_range_begin, cp - 1));
               }
               next_range_begin = cp + 1;
               break;
         }
      }
   }
   throw_syntax_error(LOFTY_SL("unexpected end of bracket expression"));
}

void ere::parse_positive_bracket_expression() {
   LOFTY_TRACE_FUNC(this);

   auto last_range_state = parser->create_code_point_state(*expr_itr++);
   push_state(last_range_state);
   bool forming_range = false, escape = false;
   // Start a series of alternatives.
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (cp == ']' && !escape) {
         if (forming_range) {
            // Turns out the dash/hyphen did not indicate a range; make a new state for it.
            subexpr_stack.back().push_alternative(parser->create_code_point_state('-'));
         }
         return;
      }
      if (forming_range) {
         forming_range = false;
         // Change the last code point state into a range.
         last_range_state->with_data<dynamic::_state_cp_range_data>()->last = cp;
      } else {
         if (escape) {
            escape = false;
            goto escaped;
         }
         switch (cp) {
            case '-':
               forming_range = true;
               break;
            case '\\':
               escape = true;
               break;
            default:
            escaped:
               // Individual code point: make a new state for it.
               last_range_state = parser->create_code_point_state(cp);
               subexpr_stack.back().push_alternative(last_range_state);
               break;
         }
      }
   }
   throw_syntax_error(LOFTY_SL("unexpected end of bracket expression"));
}

_std::tuple<std::uint16_t, std::uint16_t> ere::parse_repetition_range() {
   LOFTY_TRACE_FUNC(this);

   bool empty = true;
   std::uint16_t begin = 0, end = 0;
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr;
      if (cp < '0' || cp > '9') {
         break;
      }
      begin = static_cast<std::uint16_t>(begin * 10 + static_cast<std::uint16_t>(cp - '0'));
      empty = false;
      ++expr_itr;
   }
   if (expr_itr != expr_end && *expr_itr == ',') {
      while (++expr_itr != expr_end) {
         char32_t cp = *expr_itr;
         if (cp < '0' || cp > '9') {
            break;
         }
         begin = static_cast<std::uint16_t>(begin * 10 + static_cast<std::uint16_t>(cp - '0'));
         empty = false;
      }
   }
   if (expr_itr == expr_end || *expr_itr != '}' || empty) {
      throw_syntax_error(LOFTY_SL("malformed repetition range"));
   }
   // Consume the closing “}”.
   ++expr_itr;
   return _std::make_tuple(begin, end);
}

void ere::push_state(dynamic_state * next_state) {
   LOFTY_TRACE_FUNC(this, next_state);

   if (enter_rep_group) {
      auto repetition_group = parser->create_repetition_group(next_state, 1, 1);
      // Re-terminate the previous sub-expression with the newly-created group.
      prev_subexpr.terminate_with_next_state(repetition_group);
      auto & curr_subexpr = subexpr_stack.back();
      curr_subexpr.curr_state = repetition_group;
   } else if (begin_alternative) {
      begin_alternative = false;
      subexpr_stack.back().push_alternative(next_state);
   } else {
      if (end_subexpr) {
         /* Terminate the last sub-expression with the next state, popping it off the stack but tracking it as
         the previous sub-expression. */
         end_subexpr = false;
         prev_subexpr = subexpr_stack.pop_back();
         prev_subexpr.terminate_with_next_state(next_state);
      } else {
         // Stay in the same sub-expression, but track the previous state as its own sub-expression.
         prev_subexpr.curr_state = nullptr;
         prev_subexpr.alternative_last_states.clear();
         if (auto prev_state = subexpr_stack.back().curr_state) {
            prev_subexpr.push_next(prev_state);
            /*while (auto next_alternative = prev_state->alternative) {
               prev_subexpr.alternative_last_states.push_back(next_alternative);
               next_alternative = next_alternative->alternative;
            }*/
         }
      }
      subexpr_stack.back().push_next(next_state);
   }
}

void ere::set_curr_state_repetitions(std::uint16_t min, std::uint16_t max) {
   LOFTY_TRACE_FUNC(this, min, max);

   // Make sure we have a repetition group to apply the number of occurrences to.
   auto & curr_subexpr = subexpr_stack.back();
   if (!curr_subexpr.curr_state) {
      throw_syntax_error(LOFTY_SL("expression cannot start with ?*+{"));
   }
   if (curr_subexpr.curr_state->type == dynamic_state::_type::repetition_group) {
      // Already a group; just adjust the range.
      // TODO: block expressions like “a{1,3}{2,4}”, which in this implementation become “a{2,4}”.
      auto repetition_group = curr_subexpr.curr_state->with_data<dynamic::_state_repetition_group_data>();
      repetition_group->min = min;
      repetition_group->max = max;
   } else {
      auto repetition_group = parser->create_repetition_group(curr_subexpr.curr_state, min, max);
      // Re-terminate the previous sub-expression with the newly-created group.
      prev_subexpr.terminate_with_next_state(repetition_group);
      if (curr_subexpr.first_state == curr_subexpr.curr_state) {
         curr_subexpr.first_state = repetition_group;
      }
      curr_subexpr.curr_state = repetition_group;
   }
}

}}} //namespace lofty::text::parsers

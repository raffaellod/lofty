/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/numeric.hxx>
#include <lofty/text.hxx>
#include <lofty/text/parsers/dynamic.hxx>
#include <lofty/text/parsers/regex.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

regex_capture_format::regex_capture_format() {
}

regex_capture_format::~regex_capture_format() {
}

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

regex::subexpression::subexpression() :
   first_state(nullptr),
   curr_alternative_first_state(nullptr),
   curr_state(nullptr) {
}

regex::subexpression::~subexpression() {
}

void regex::subexpression::push_alternative(dynamic_state * new_state) {
   if (curr_state) {
      alternative_last_states.push_back(curr_state);
      curr_alternative_first_state->set_alternative(new_state);
   } else {
      first_state = new_state;
   }
   curr_state = new_state;
   curr_alternative_first_state = new_state;
}

void regex::subexpression::push_next(dynamic_state * new_state) {
   if (curr_state) {
      curr_state->set_next(new_state);
   } else {
      first_state = new_state;
      curr_alternative_first_state = new_state;
   }
   curr_state = new_state;
}

void regex::subexpression::terminate_with_next_state(dynamic_state * next_state) {
   if (curr_state) {
      alternative_last_states.push_back(curr_state);
      curr_state = nullptr;
   }
   LOFTY_FOR_EACH(auto & state, alternative_last_states) {
      state->set_next(next_state);
   }
}


regex::regex(dynamic * parser_, str const & expr_) :
   parser(parser_),
   expr(expr_),
   expr_itr(expr.cbegin()),
   expr_end(expr.cend()),
   next_capture_index(0),
   subexprs_to_end(0),
   enter_rep_group(false),
   begin_alternative(false) {
   subexpr_stack.push_back(subexpression());
}

regex::~regex() {
}

void regex::insert_capture_group(dynamic_state const * first_state) {
   push_state(parser->create_capture_group(first_state));
}

int regex::parse_group(regex_capture_format * capture_format) {
   if (expr_itr >= expr_end) {
      throw_syntax_error(LOFTY_SL("unexpected end of group"));
   }
   if (*expr_itr == '?') {
      ++expr_itr;
      if (expr_itr >= expr_end) {
         throw_syntax_error(LOFTY_SL("unexpected end of group modifier"));
      }
      switch (*expr_itr) {
         case ':':
            ++expr_itr;
            enter_rep_group = true;
            return -1;
         case '.': {
            // We have a format variable assignment.
            if (!capture_format) {
               // The client just wants to make sure there are no capturing groups; report that we found one.
               return 0;
            }
            do {
               regex_capture_format::var_pair var;
               ++expr_itr;
               auto itr(expr.find('=', expr_itr));
               if (itr >= expr_end) {
                  throw_syntax_error(LOFTY_SL("expected “=” for “?.var='value';” group modifier"));
               }
               var.name = str(external_buffer, expr_itr.ptr(), itr.char_index() - expr_itr.char_index());
               expr_itr = ++itr;
               if (expr_itr >= expr_end) {
                  throw_syntax_error(LOFTY_SL("unexpected end of “?.var='value';” group modifier"));
               }
               if (*expr_itr != '\'') {
                  throw_syntax_error(
                     LOFTY_SL("expected single quote for value of “?.var='value';” group modifier")
                  );
               }
               ++expr_itr;
               bool escape = false;
               while (expr_itr != expr_end) {
                  char32_t cp = *expr_itr++;
                  if (escape) {
                     var.value += cp;
                     escape = false;
                  } else {
                     if (cp == '\\') {
                        escape = true;
                     } else if (cp == '\'') {
                        break;
                     }
                  }
               }
               if (expr_itr == expr_end) {
                  throw_syntax_error(
                     LOFTY_SL("unexpected end of “?.var='value';” group modifier")
                  );
               }
               capture_format->vars.push_back(_std::move(var));
            } while (*expr_itr == ',');
            if (*expr_itr != ';') {
               throw_syntax_error(
                  LOFTY_SL("expected “,” or “;” after value of “?.var='value';” group modifier")
               );
            }
            ++expr_itr;
            break;
         }
         default:
            throw_syntax_error(LOFTY_SL("unsupported group modifier"));
            break;
      }
   }
   if (!capture_format) {
      // The client just wants to make sure there are no capturing groups; report that we found one.
      return 0;
   }

   if (expr_itr != expr_end) {
      // The capture specifies a format expression.
      auto expr_begin(expr_itr);
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
      capture_format->expr = str(external_buffer, expr_begin.ptr(), expr_itr.char_index() - expr_begin.char_index());
   } else {
      capture_format->expr.clear();
   }

   if (expr_itr == expr_end || *expr_itr != ')') {
      throw_syntax_error(LOFTY_SL("unterminated capturing group"));
   }
   // Also consume the last parenthesis, so it won’t trigger subexprs_to_end in parse_up_to_next_capture().
   ++expr_itr;

   return static_cast<int>(next_capture_index++);
}

void regex::parse_negative_bracket_expression() {
   char32_t next_range_begin = *expr_itr++ + 1;
   // Start with the first alternative.
   // TODO: this is not right if *expr_itr is NUL (rare).
   push_state(parser->create_code_point_range_state(numeric::min<char32_t>::value, next_range_begin - 2));
   bool forming_range = false, escape = false, group_added = false;
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (cp == ']' && !escape) {
         if (!group_added) {
            set_curr_state_repetitions(1, 1);
            group_added = true;
         }
         auto & curr_subexpr = subexpr_stack.back();
         if (forming_range) {
            // Turns out the dash/hyphen did not indicate a range; make two new states around it.
            curr_subexpr.push_alternative(parser->create_code_point_range_state(next_range_begin, '-' - 1));
            next_range_begin = '-' + 1;
         }
         // Add an alternative from next_range_begin to ∞ to end the negative range.
         curr_subexpr.push_alternative(parser->create_code_point_range_state(next_range_begin, numeric::max<char32_t>::value));
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
                  if (!group_added) {
                     set_curr_state_repetitions(1, 1);
                     group_added = true;
                  }
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

void regex::parse_positive_bracket_expression() {
   auto last_range_state = parser->create_code_point_state(*expr_itr++);
   push_state(last_range_state);
   bool forming_range = false, escape = false, group_added = false;
   // Start a series of alternatives.
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (cp == ']' && !escape) {
         if (forming_range) {
            // Turns out the dash/hyphen did not indicate a range; make a new state for it.
            if (!group_added) {
               set_curr_state_repetitions(1, 1);
               group_added = true;
            }
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
               if (!group_added) {
                  set_curr_state_repetitions(1, 1);
                  group_added = true;
               }
               subexpr_stack.back().push_alternative(last_range_state);
               break;
         }
      }
   }
   throw_syntax_error(LOFTY_SL("unexpected end of bracket expression"));
}

_std::tuple<std::uint16_t, std::uint16_t> regex::parse_repetition_range() {
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

int regex::parse_up_to_next_capture(regex_capture_format * capture_format, dynamic_state ** first_state) {
   bool escape = false;
   while (expr_itr != expr_end) {
      char32_t cp = *expr_itr++;
      if (escape) {
         escape = false;
         goto escaped;
      }
      switch (cp) {
         case '.':
            push_state(parser->create_code_point_range_state(
               numeric::min<char32_t>::value, numeric::max<char32_t>::value
            ));
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
            ++subexprs_to_end;
            break;
         case '\\':
            escape = true;
            break;
         case '(': {
            auto capture_index = parse_group(capture_format);
            if (capture_index >= 0) {
               return capture_index;
            }
            break;
         }
         case ')':
            ++subexprs_to_end;
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
   if (enter_rep_group || begin_alternative) {
      throw_syntax_error(LOFTY_SL("unexpected final state"));
   }
   if (subexprs_to_end > 0) {
      push_state(nullptr);
   }
   if (subexpr_stack.size() != 1) {
      throw_syntax_error(LOFTY_SL("mismatched parentheses"));
   }
   *first_state = subexpr_stack.front().first_state;
   // No need to terminate the last sub-expression; it should be already nullptr-terminated.
   subexpr_stack.clear();
   return -1;
}

dynamic_state * regex::parse_with_no_captures() {
   dynamic_state * ret;
   if (parse_up_to_next_capture(nullptr, &ret) >= 0) {
      throw_syntax_error(LOFTY_SL("capturing groups not supported in this expression"));
   }
   return ret;
}

void regex::push_state(dynamic_state * next_state) {
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
      if (subexprs_to_end > 0) {
         /* A sub-expression to end always has a single top-level state, either because it’s a list of
         character ranges (single state with alternatives) or because it’s a group (single state with separate
         branching). This means that we can insert all the sub-expressions to end between the next_state and
         the sub-expression it’s supposed to be the next state of (which has index
         subexpr_stack.size() - subexprs_to_end). */
         do {
            prev_subexpr = subexpr_stack.pop_back();
            prev_subexpr.terminate_with_next_state(next_state);
            next_state = prev_subexpr.first_state;
         } while (--subexprs_to_end > 0);
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

void regex::set_curr_state_repetitions(std::uint16_t min, std::uint16_t max) {
   // Make sure we have a repetition group to apply the number of occurrences to.
   auto & curr_subexpr = subexpr_stack.back();
   if (!curr_subexpr.curr_state) {
      throw_syntax_error(LOFTY_SL("expression cannot start with ?*+{"));
   }
   if (curr_subexpr.first_state->type == dynamic_state::_type::repetition_group) {
      // This sub-expression is already a group; just adjust its range.
      // TODO: block expressions like “a{1,3}{2,4}”, which in this implementation become “a{2,4}”.
      auto repetition_group = curr_subexpr.first_state->with_data<dynamic::_state_repetition_group_data>();
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

void regex::throw_syntax_error(str const & description) const {
   LOFTY_THROW(syntax_error, (
      // +1 because the first character is 1, to human beings.
      description, expr, static_cast<unsigned>(expr_itr - expr.cbegin() + 1)
   ));
}

}}} //namespace lofty::text::parsers

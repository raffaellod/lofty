/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2017 Raffaello D. Di Napoli

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
#include <lofty/text/parsers/ansi_escape_sequences.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

ansi_escape_sequences::ansi_escape_sequences() :
   curr_state(state::not_in_sequence),
   cmd_args_size(0) {
}

ansi_escape_sequences::~ansi_escape_sequences() {
}

bool ansi_escape_sequences::got_one_argument(std::int16_t default0) {
   LOFTY_TRACE_FUNC(this, default0);

   if (cmd_args_size == 0) {
      cmd_args[cmd_args_size++] = default0;
   }
   return cmd_args_size == 1;
}

bool ansi_escape_sequences::got_two_arguments(std::int16_t default0, std::int16_t default1) {
   LOFTY_TRACE_FUNC(this, default0, default1);

   if (cmd_args_size == 0) {
      cmd_args[cmd_args_size++] = default0;
   }
   if (cmd_args_size == 1) {
      cmd_args[cmd_args_size++] = default1;
   }
   return cmd_args_size == 2;
}

bool ansi_escape_sequences::consume_sequence_char(char_t ch) {
   LOFTY_TRACE_FUNC(this, ch);

   switch (curr_state.base()) {
      case state::not_in_sequence:
         // Cannot happen, but here to make the compiler happy.
         break;

      case state::escape:
         if (ch == '[' || ch == ']') {
            // Reinitialize the argument storage, preparing to parse the rest of the sequence.
            seq_start_char = ch;
            cmd_args_size = 0;
            cmd_arg_str.clear();
            curr_state = state::bracket;
         } else if (ch == ')' || ch == '(') {
            curr_state = state::ignore;
         } else if (ch != '\x1b') {
            // Multiple ESC characters are not counted; any other character ends the sequence.
            curr_state = state::not_in_sequence;
            return false;
         }
         break;

      case state::bracket:
         if (ch >= '0' && ch <= '9') {
            if (cmd_args_size < cmd_args_size_max) {
               cmd_args[cmd_args_size++] = static_cast<std::int16_t>(ch - '0');
            }
            curr_state = state::numeric_arg;
         } else if (ch == ';') {
            if (cmd_args_size < cmd_args_size_max) {
               cmd_args[cmd_args_size++] = 0;
            }
         } else if (ch == '?') {
            seq_start_char = ch;
         } else {
            run_sequence(ch);
            curr_state = state::not_in_sequence;
            return false;
         }
         break;

      case state::numeric_arg:
         if (ch >= '0' && ch <= '9') {
            std::int16_t & last_cmd_arg = cmd_args[cmd_args_size];
            last_cmd_arg = static_cast<std::int16_t>(last_cmd_arg * std::int16_t(10) + (ch - '0'));
         } else if (ch == ';') {
            if (cmd_args_size < cmd_args_size_max) {
               cmd_args[cmd_args_size++] = 0;
            }
            if (seq_start_char == ']') {
               curr_state = state::string_arg;
            }
         } else {
            run_sequence(ch);
            curr_state = state::not_in_sequence;
            return true;
         }
         break;

      case state::string_arg:
         if (ch == '\b' || (ch == '\\' && cmd_arg_str && *(cmd_arg_str.cend() - 1) == '\x1b')) {
            run_sequence(ch);
            curr_state = state::not_in_sequence;
         } else {
            cmd_arg_str += ch;
         }
         break;

      case state::ignore:
         curr_state = state::not_in_sequence;
         break;
   }
   return true;
}

void ansi_escape_sequences::run_erase_display_sequence(int mode) {
   LOFTY_TRACE_FUNC(this, mode);

   std::int16_t row, col, rows, cols;
   get_cursor_pos_and_display_size(&row, &col, &rows, &cols);
   if (mode == 0) {
      // Erase from the cursor position to end of the display.
      clear_display_area(
         row, col,
         static_cast<std::size_t>(cols - col) +
            static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows - row - 1)
      );
   } else if (mode == 1) {
      // Erase from row 0 up to the cursor position.
      clear_display_area(0, 0, static_cast<std::size_t>(cols) * static_cast<std::size_t>(row + col));
   } else if (mode == 2) {
      // Erase the entire display.
      clear_display_area(0, 0, static_cast<std::size_t>(cols) * static_cast<std::size_t>(rows));
   }
}

void ansi_escape_sequences::run_erase_row_sequence(int mode) {
   LOFTY_TRACE_FUNC(this, mode);

   std::int16_t row, col, rows, cols;
   get_cursor_pos_and_display_size(&row, &col, &rows, &cols);
   if (mode == 0) {
      // Erase from the cursor position till the end of the row.
      clear_display_area(row, col, static_cast<std::size_t>(cols - col));
   } else if (mode == 1) {
      // Erase from column 0 of the cursor row up to the cursor position.
      clear_display_area(row, 0, static_cast<std::size_t>(col));
   } else if (mode == 2) {
      // Erase current row.
      clear_display_area(row, 0, static_cast<std::size_t>(cols));
   }
}

void ansi_escape_sequences::run_sequence(char_t cmd_char) {
   LOFTY_TRACE_FUNC(this, cmd_char);

   if (seq_start_char == '[') {
      switch (cmd_char) {
         case 'A': // Move cursor up N rows.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(-cmd_args[0], 0);
            }
            break;
         case 'B': // Move cursor down N rows.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(cmd_args[0], 0);
            }
            break;
         case 'C': // Move cursor right N columns.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, cmd_args[0]);
            }
            break;
         case 'D': // Move cursor left N columns.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, -cmd_args[0]);
            }
            break;
         case 'E': // Move cursor down N rows, column 1.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(cmd_args[0], 0, false, true);
            }
            break;
         case 'F': // Move cursor up N rows, column 1.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(-cmd_args[0], 0, false, true);
            }
            break;
         case 'G': // Move cursor to column N.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, cmd_args[0] - 1, false, true);
            }
            break;
         case 'H': // Move cursor to row N, column M.
         case 'f': // Move cursor to row N, column M.
            if (got_two_arguments(1, 1)) {
               safe_set_cursor_pos(cmd_args[0] - 1, cmd_args[1] - 1, true, true);
            }
            break;
         case 'J': // Erase the display from the cursor down (N=0), up (N=1), or everything (N=2).
            if (got_one_argument(0)) {
               run_erase_display_sequence(cmd_args[0]);
            }
            break;
         case 'K': // Erase chars right (N=0), left (N=1) or both (N=2) sides of the current column.
            if (got_one_argument(0)) {
               run_erase_row_sequence(cmd_args[0]);
            }
            break;
         case 'S': // Scroll text up N rows.
            if (got_one_argument(1)) {
               scroll_text(static_cast<std::int16_t>(-cmd_args[0]), 0);
            }
            break;
         case 'T': // Scroll text down N rows.
            if (got_one_argument(1)) {
               scroll_text(cmd_args[0], 0);
            }
            break;
         case 'd': // Move cursor to row N.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(cmd_args[0] - 1, 0, true, false);
            }
            break;
         case 'm': // Set character attributes.
            got_one_argument(0);
            run_set_char_attributes_sequence();
            break;
         case 's': // Save cursor position.
            if (cmd_args_size == 0) {
               get_cursor_pos_and_display_size(&saved_row, &saved_col, nullptr, nullptr);
            }
            break;
         case 'u': // Restore saved cursor position.
            if (cmd_args_size == 0) {
               safe_set_cursor_pos(saved_row, saved_col, true, true);
            }
            break;
      }
   } else if (seq_start_char == ']') {
      if (cmd_args_size == 1 && cmd_args[0] == 0) {
         set_window_title(cmd_arg_str);
      }
   } else if (seq_start_char == '?') {
      if ((cmd_char == 'h' || cmd_char == 'l') && cmd_args_size == 1 && cmd_args[0] == 25) {
         set_cursor_visibility(cmd_char == 'h');
      }
   }
}

void ansi_escape_sequences::run_set_char_attributes_sequence() {
   LOFTY_TRACE_FUNC(this);

   for (unsigned i = 0; i < cmd_args_size; ++i) {
      std::int16_t cmd_arg = cmd_args[i];
      switch (cmd_arg) {
         case 0:
         case 39:
         case 49:
            if (cmd_arg != 49) {
               curr_char_attr.foreground_color = default_char_attr.foreground_color;
            }
            if (cmd_arg != 39) {
               curr_char_attr.background_color = default_char_attr.background_color;
            }
            if (cmd_arg == 0) {
               curr_char_attr.blink_speed   = default_char_attr.blink_speed;
               curr_char_attr.concealed     = default_char_attr.concealed;
               curr_char_attr.crossed_out   = default_char_attr.crossed_out;
               curr_char_attr.intensity     = default_char_attr.intensity;
               curr_char_attr.italic        = default_char_attr.italic;
               curr_char_attr.reverse_video = default_char_attr.reverse_video;
               curr_char_attr.underline     = default_char_attr.underline;
            }
            break;
         case  1: curr_char_attr.intensity     = 2;     break;
         case  2: curr_char_attr.intensity     = 0;     break;
         case  3: curr_char_attr.italic        = true;  break;
         case  4: curr_char_attr.underline     = 1;     break;
         case  5: curr_char_attr.blink_speed   = 1;     break;
         case  6: curr_char_attr.blink_speed   = 2;     break;
         case  7: curr_char_attr.reverse_video = true;  break;
         case  8: curr_char_attr.concealed     = true;  break;
         case  9: curr_char_attr.crossed_out   = true;  break;
         case 21: // This would set double underline on rare terminals, but bold off on others.
         case 22: curr_char_attr.intensity     = 1;     break;
         case 23: curr_char_attr.italic        = false; break;
         case 24: curr_char_attr.underline     = 0;     break;
         case 25: curr_char_attr.blink_speed   = 0;     break;
         case 27: curr_char_attr.reverse_video = false; break;
         case 28: curr_char_attr.concealed     = false; break;
         case 29: curr_char_attr.crossed_out   = false; break;

         case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
            curr_char_attr.foreground_color = static_cast<std::uint8_t>((cmd_arg - 30) & 0x7);
            break;
         case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
            curr_char_attr.background_color = static_cast<std::uint8_t>((cmd_arg - 40) & 0x7);
            break;
      }
   }
   set_char_attributes();
}

void ansi_escape_sequences::safe_set_cursor_pos(
   int row, int col, bool absolute_row /*= false*/, bool absolute_col /*= false*/
) {
   LOFTY_TRACE_FUNC(this, row, col, absolute_row, absolute_col);

   std::int16_t curr_row, curr_col, rows, cols;
   get_cursor_pos_and_display_size(&curr_row, &curr_col, &rows, &cols);

   // If row is relative, make it absolute; then clip it to the display height.
   if (!absolute_row) {
      row += curr_row;
   }
   if (row < 0) {
      if (absolute_row) {
         // A negative value in absolute mode means use the current value.
         row = curr_row;
      } else {
         row = 0;
      }
   } else if (row >= rows) {
      row = rows - 1;
   }

   // If col is relative, make it absolute; then clip it to the display width.
   if (!absolute_col) {
      col += curr_col;
   }
   if (col < 0) {
      if (absolute_col) {
         // A negative value in absolute mode means use the current value.
         col = curr_col;
      } else {
         col = 0;
      }
   } else if (col >= cols) {
      col = cols - 1;
   }

   set_cursor_pos(static_cast<std::int16_t>(row), static_cast<std::int16_t>(col));
}

}}} //namespace lofty::text::parsers

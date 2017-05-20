/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TEXT_PARSERS_ANSI_ESCAPE_SEQUENCES_HXX
#define _LOFTY_TEXT_PARSERS_ANSI_ESCAPE_SEQUENCES_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
   // Note: this file may be included while _LOFTY_HXX_INTERNAL is defined.
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace text { namespace parsers {

//! ANSI terminal 3-bit color palette.
LOFTY_ENUM(ansi_terminal_color,
   (black,   0),
   (red,     1),
   (green,   2),
   (yellow,  3),
   (blue,    4),
   (magenta, 5),
   (cyan,    6),
   (white,   7)
);

//! Parses ANSI escape sequences into data packets.
class LOFTY_SYM ansi_escape_sequences {
public:
   //! Groups together attributes used for graphic rendition (color and other attributes).
   struct char_attributes {
      //! Background color (ansi_terminal_color).
      std::uint8_t background_color:3;
      //! Foreground color (ansi_terminal_color).
      std::uint8_t foreground_color:3;
      //! Generate blinking characters; 0 = off, 1 = slow, 2 = rapid.
      std::uint8_t blink_speed:2;
      //! Display characters using the same color for background and foreground.
      bool concealed:1;
      //! Legible characters, but marked for deletion.
      bool crossed_out:1;
      //! Typeface intensity: 0 = faint, 1 = normal, 2 = bold.
      std::uint8_t intensity:2;
      //! Use an italic typeface.
      bool italic:1;
      //! Switch background and foreground colors.
      bool reverse_video:1;
      //! Underline the text; 0 = off, 1 = single underline, 2 = double underline.
      std::uint8_t underline:2;
   };

private:
   //! Internal automaton states.
   LOFTY_ENUM_AUTO_VALUES(state,
      not_in_sequence, //! Not in an ANSI escape sequence.
      escape,          //! Control Sequence Introducer found.
      bracket,         //! Read a bracket following the CSI.
      numeric_arg,     //! Expecting or reading a numeric argument in the escape sequence.
      string_arg,      //! Expecting or reading a string argument in the escape sequence.
      ignore           //! Ignoring a character after a CSI + parenthesis sequence.
   );

public:
   /*! Analyzes a character, returning true if it’s part of an ANSI escape sequence and was therefore consumed
   by the parser. This is inline-able for performance reasons, since it will need to be called on each input
   character.

   @param ch
      Character to be analyzed.
   @return
      true if the character was consumed, or false otherwise.
   */
   bool consume_char(char_t ch) {
      if (curr_state != state::not_in_sequence) {
         return consume_sequence_char(ch);
      } else if (ch == '\x1b') {
         curr_state = state::escape;
         return true;
      } else {
         return false;
      }
   }

protected:
   //! Default constructor.
   ansi_escape_sequences();

   //! Destructor.
   ~ansi_escape_sequences();

   /*! Assigns a null character (e.g. a space) with the current attributes to the specified area.

   @param row
      Starting row index.
   @param col
      Starting column index.
   @param char_size
      Count of characters to fill, starting from (row, col).
   */
   virtual void clear_display_area(std::int16_t row, std::int16_t col, std::size_t char_size) = 0;

   /*! Invoked when the current cursor position or display size are requested.

   @param row
      If not nullptr, it points to a variable to be set to the cursor row index.
   @param col
      If not nullptr, it points to a variable to be set to the cursor column index.
   @param rows
      If not nullptr, it points to a variable to be set to the display rows count.
   @param cols
      If not nullptr, it points to a variable to be set to the display columns count.
   */
   virtual void get_cursor_pos_and_display_size(
      std::int16_t * row, std::int16_t * col, std::int16_t * rows, std::int16_t * cols
   ) = 0;

   /*! Scrolls the displayed text, adding empty rows/columns as necessary.

   @param rows
      Count of rows the text should be moved up (if positive) or down (if negative).
   @param cols
      Count of columns the text should be moved left (if positive) or right (if negative).
   */
   virtual void scroll_text(std::int16_t rows, std::int16_t cols) = 0;

   //! Invoked to update one or more character attributes, stored in curr_char_attr.
   virtual void set_char_attributes() = 0;

   /*! Invoked to set the current cursor position.

   @param row
      Row index the cursor is required to be moved to.
   @param col
      Column index the cursor is required to be moved to.
   */
   virtual void set_cursor_pos(std::int16_t row, std::int16_t col) = 0;

   /*! Invoked to change the visibility of the cursor.

   @param visible
      true if the cursor is to be made visible, or false it it’s to be hidden.
   */
   virtual void set_cursor_visibility(bool visible) = 0;

   /*! Invoked to change the terminal window title.

   @param title
      New window title.
   */
   virtual void set_window_title(str const & title) = 0;

private:
   /*! Implementation of consume_char() for when the parser state is “in sequence”, i.e. while parsing an
   escape sequence.

   @param ch
      Character to be analyzed.
   @return
      true if the character was consumed, or false otherwise.
   */
   bool consume_sequence_char(char_t ch);

   /*! Returns true if the current command has been provided with exactly one argument, or if it has zero
   arguments; in the latter case, one arguments with value default0 will be added.

   @param default0
      Default argument to be pushed to cmd_args if zero arguments are available.
   @return
      true if exactly one argument is available, or false otherwise.
   */
   bool got_one_argument(std::int16_t default0);

   /*! Returns true if the current command has been provided with exactly two arguments, or if it has zero to
   one arguments; in the latter case, up to two arguments with values default0 and default1 will be added.

   @param default0
      Default argument to be pushed to cmd_args if zero arguments are available.
   @param default1
      Default argument to be pushed to cmd_args if only one argument is available.
   @return
      true if exactly two arguments are available, or false otherwise.
   */
   bool got_two_arguments(std::int16_t default0, std::int16_t default1);

   //! Implementation of run_sequence('J').
   void run_erase_display_sequence(int mode);

   //! Implementation of run_sequence('K').
   void run_erase_row_sequence(int mode);

   /*! Executes the sequence as accumulated in the member variables.

   @param cmd_char
      Last character of the sequence, indicating the command to execute.
   */
   void run_sequence(char_t cmd_char);

   //! Implementation of run_sequence('m').
   void run_set_char_attributes_sequence();

   /*! Set the current cursor position, keeping it constrained to the display size.

   @param row
      New cursor row.
   @param col
      New cursor column.
   @param absolute_row
      If true, row is interpreted as an absolute 0-based row number; if false, it’s applied as a delta from
      the current row.
   @param absolute_col
      If true, col is interpreted as an absolute 0-based column number; if false, it’s applied as a delta from
      the current column.
   */
   void safe_set_cursor_pos(int row, int col, bool absolute_row = false, bool absolute_col = false);

protected:
   //! Initial character attributes.
   char_attributes default_char_attr;
   //! Current character attributes.
   char_attributes curr_char_attr;

private:
   //! Maximum number of arguments needed by any sequence.
   static std::size_t const cmd_args_size_max = 4;
   //! Current automaton state.
   state curr_state;
   //! Character that started the current sequence. Can be ‘[’, ‘]’ or ‘?’ (for “[?”).
   char_t seq_start_char;
   //! Numeric arguments parsed from the current sequence.
   std::int16_t cmd_args[cmd_args_size_max];
   //! Count of elements in cmd_args.
   unsigned cmd_args_size;
   //! String argument parsed from the current sequence.
   str cmd_arg_str;
   //! Stores the row number for the Save/Restore Cursor Position command.
   std::int16_t saved_row;
   //! Stores the column number for the Save/Restore Cursor Position command.
   std::int16_t saved_col;
};

}}} //namespace lofty::text::parsers

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TEXT_PARSERS_ANSI_ESCAPE_SEQUENCES_HXX

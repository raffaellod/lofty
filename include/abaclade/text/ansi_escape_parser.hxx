/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_TEXT_ANSI_ESCAPE_PARSER_HXX
#define _ABACLADE_TEXT_ANSI_ESCAPE_PARSER_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::ansi_escape_parser

namespace abc {
namespace text {

namespace detail {

//! Internal automaton states for ansi_escape_parser.
ABC_ENUM_AUTO_VALUES(ansi_escape_parser_state,
   not_in_sequence, //! Not in an ANSI escape sequence.
   escape,          //! Control Sequence Introducer found.
   bracket,         //! Read a bracket following the CSI.
   numeric_arg,     //! Expecting or reading a numeric argument in the escape sequence.
   string_arg,      //! Expecting or reading a string argument in the escape sequence.
   ignore           //! Ignoring a character after a CSI + parenthesis sequence.
);

} //namespace detail

//! ANSI terminal 3-bit color palette.
ABC_ENUM(ansi_terminal_color,
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
class ABACLADE_SYM ansi_escape_parser {
public:
   //! Groups together attributes used for graphic rendition (color and other attributes).
   struct char_attributes {
      //! Background color.
      // TODO: fix ABC_ENUM to not inject additional enum values, then change this to :3.
      ansi_terminal_color::enum_type clrBackground:4;
      //! Foreground color.
      // TODO: fix ABC_ENUM to not inject additional enum values, then change this to :3.
      ansi_terminal_color::enum_type clrForeground:4;
      //! Generate blinking characters; 0 = off, 1 = slow, 2 = rapid.
      std::uint8_t iBlinkSpeed:2;
      //! Display characters using the same color for background and foreground.
      bool bConcealed:1;
      //! Legible characters, but marked for deletion.
      bool bCrossedOut:1;
      //! Typeface intensity: 0 = faint, 1 = normal, 2 = bold.
      std::uint8_t iIntensity:2;
      //! Use an italic typeface.
      bool bItalic:1;
      //! Switch background and foreground colors.
      bool bReverseVideo:1;
      //! Underline the text; 0 = off, 1 = single underline, 2 = double underline.
      std::uint8_t iUnderline:2;
   };

   //! Shortcut.
   typedef detail::ansi_escape_parser_state state;

public:
   /*! Analyzes a character, returning true if it’s part of an ANSI escape sequence and was
   therefore consumed by the parser. This is inline-able for performance reasons, since it will need
   to be called on each input character.

   ch
      Character to be analyzed.
   return
      true if the character was consumed, or false otherwise.
   */
   bool consume_char(char_t ch) {
      if (m_state != state::not_in_sequence) {
         return consume_sequence_char(ch);
      } else if (ch == '\x1b') {
         m_state = state::escape;
         return true;
      } else {
         return false;
      }
   }

protected:
   //! Constructor.
   ansi_escape_parser();

   //! Destructor.
   ~ansi_escape_parser();

   /*! Assigns a null character (e.g. a space) with the current attributes to the specified area.

   iRow
      Starting row index.
   iCol
      Starting column index.
   cch
      Count of characters to fill, starting from (iRow, iCol).
   */
   virtual void clear_display_area(std::int16_t iRow, std::int16_t iCol, std::size_t cch) = 0;

   /*! Invoked when the current cursor position or display size are requested.

   piRow
      If not nullptr, it points to a variable to be set to the cursor row index.
   piCol
      If not nullptr, it points to a variable to be set to the cursor column index.
   pcRows
      If not nullptr, it points to a variable to be set to the display rows count.
   pcCols
      If not nullptr, it points to a variable to be set to the display columns count.
   */
   virtual void get_cursor_pos_and_display_size(
      std::int16_t * piRow, std::int16_t * piCol, std::int16_t * pcRows, std::int16_t * pcCols
   ) = 0;

   /*! Scrolls the displayed text, adding empty rows/columns as necessary.

   cRows
      Count of rows the text should be moved up (if positive) or down (if negative).
   cCols
      Count of columns the text should be moved left (if positive) or right (if negative).
   */
   virtual void scroll_text(std::int16_t cRows, std::int16_t cCols) = 0;

   //! Invoked to update one or more character attributes, stored in m_chattrCurr.
   virtual void set_char_attributes() = 0;

   /*! Invoked to set the current cursor position.

   iRow
      Row index the cursor is required to be moved to.
   iCol
      Column index the cursor is required to be moved to.
   */
   virtual void set_cursor_pos(std::int16_t iRow, std::int16_t iCol) = 0;

   /*! Invoked to change the visibility of the cursor.

   bVisible
      true if the cursor is to be made visible, or false it it’s to be hidden.
   */
   virtual void set_cursor_visibility(bool bVisible) = 0;

   /*! Invoked to change the terminal window title.

   sTitle
      New window title.
   */
   virtual void set_window_title(istr const & sTitle) = 0;

private:
   /*! Implementation of consume_char() for when the parser state is “in sequence”, i.e. while
   parsing an escape sequence.

   ch
      Character to be analyzed.
   return
      true if the character was consumed, or false otherwise.
   */
   bool consume_sequence_char(char_t ch);

   //! TODO: comment.
   bool got_one_argument(std::int16_t iDefault0);

   //! TODO: comment.
   bool got_two_arguments(std::int16_t iDefault0, std::int16_t iDefault1);

   //! Implementation of run_sequence('J').
   void run_erase_display_sequence(int iMode);

   //! Implementation of run_sequence('K').
   void run_erase_row_sequence(int iMode);

   /*! Executes the sequence as accumulated in the member variables.

   chCmd
      Last character of the sequence, indicating the command to execute.
   */
   void run_sequence(char_t chCmd);

   //! Implementation of run_sequence('m').
   void run_set_char_attributes_sequence();

   /*! Set the current cursor position, keeping it constrained to the display size.

   iRow
      New cursor row.
   iCol
      New cursor column.
   bAbsoluteRow
      If true, iRow is interpreted as an absolute 0-based row number; if false, it’s applied as a
      delta from the current row.
   bAbsoluteCol
      If true, iCol is interpreted as an absolute 0-based column number; if false, it’s applied as a
      delta from the current column.
   */
   void safe_set_cursor_pos(
      int iRow, int iCol, bool bAbsoluteRow = false, bool bAbsoluteCol = false
   );

protected:
   //! TODO: comment.
   char_attributes m_chattrDefault;
   //! TODO: comment.
   char_attributes m_chattrCurr;

private:
   //! Current automaton state.
   state m_state;
   //! Character that started the current sequence. Can be ‘[’, ‘]’ or ‘?’ (for “[?”).
   char_t m_chSeqStart;
   //! Numeric arguments parsed from the current sequence.
   smvector<std::int16_t, 4> m_viCmdArgs;
   //! String argument parsed from the current sequence.
   dmstr m_sCmdArg;
   //! Stores the row number for the Save/Restore Cursor Position command.
   std::int16_t m_iSavedRow;
   //! Stores the column number for the Save/Restore Cursor Position command.
   std::int16_t m_iSavedCol;
};

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TEXT_ANSI_ESCAPE_PARSER_HXX


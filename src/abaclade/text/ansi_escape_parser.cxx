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

#include <abaclade.hxx>
#include <abaclade/text/ansi_escape_parser.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::text::ansi_escape_parser

namespace abc {
namespace text {

ansi_escape_parser::ansi_escape_parser() :
   m_state(state::not_in_sequence) {
}

ansi_escape_parser::~ansi_escape_parser() {
}

bool ansi_escape_parser::got_one_argument(std::int16_t iDefault0) {
   ABC_TRACE_FUNC(this, iDefault0);

   if (m_viCmdArgs.size() == 0) {
      m_viCmdArgs.append(iDefault0);
   }
   return m_viCmdArgs.size() == 1;
}

bool ansi_escape_parser::got_two_arguments(std::int16_t iDefault0, std::int16_t iDefault1) {
   ABC_TRACE_FUNC(this, iDefault0, iDefault1);

   if (m_viCmdArgs.size() == 0) {
      m_viCmdArgs.append(iDefault0);
   }
   if (m_viCmdArgs.size() == 1) {
      m_viCmdArgs.append(iDefault1);
   }
   return m_viCmdArgs.size() == 2;
}

bool ansi_escape_parser::consume_sequence_char(char_t ch) {
   ABC_TRACE_FUNC(this, ch);

   switch (m_state.base()) {
      case state::escape:
         if (ch == '[' || ch == ']') {
            // Reinitialize the argument storage, preparing to parse the rest of the sequence.
            m_chSeqStart = ch;
            m_viCmdArgs.clear();
            m_sCmdArg.clear();
            m_state = state::bracket;
         } else if (ch == ')' || ch == '(') {
            m_state = state::ignore;
         } else if (ch == '\x1b') {
            // Multiple ESC characters are not counted; any other character ends the sequence.
            m_state = state::not_in_sequence;
            return false;
         }
         break;

      case state::bracket:
         if (ch >= '0' && ch <= '9') {
            m_viCmdArgs.append(static_cast<std::int16_t>(ch - '0'));
            m_state = state::numeric_arg;
         } else if (ch == ';') {
            m_viCmdArgs.append(0);
         } else if (ch == '?') {
            m_chSeqStart = ch;
         } else {
            run_sequence(ch);
            m_state = state::not_in_sequence;
            return false;
         }
         break;

      case state::numeric_arg:
         if (ch >= '0' && ch <= '9') {
            std::int16_t * piLastCmdArg = (m_viCmdArgs.end() - 1).base();
            *piLastCmdArg = static_cast<std::int16_t>(
               *piLastCmdArg * std::int16_t(10) + (ch - '0')
            );
         } else if (ch == ';') {
            m_viCmdArgs.append(0);
            if (m_chSeqStart == ']') {
               m_state = state::string_arg;
            }
         } else {
            run_sequence(ch);
            m_state = state::not_in_sequence;
            return true;
         }
         break;

      case state::string_arg:
         if (ch == '\b' || (ch == '\\' && m_sCmdArg && *(m_sCmdArg.cend() - 1) == '\x1b')) {
            run_sequence(ch);
            m_state = state::not_in_sequence;
         } else {
            m_sCmdArg += ch;
         }
         break;

      case state::ignore:
         m_state = state::not_in_sequence;
         break;

      default:
         break;
   }
   return true;
}

void ansi_escape_parser::run_erase_display_sequence(int iMode) {
   ABC_TRACE_FUNC(this, iMode);

   std::int16_t iRow, iCol, cRows, cCols;
   get_cursor_pos_and_display_size(&iRow, &iCol, &cRows, &cCols);
   if (iMode == 0) {
      // Erase from the cursor position to end of the display.
      clear_display_area(
         iRow, iCol,
         static_cast<std::size_t>(cCols - iCol) +
            static_cast<std::size_t>(cCols) * static_cast<std::size_t>(cRows - iRow - 1)
      );
   } else if (iMode == 1) {
      // Erase from row 0 up to the cursor position.
      clear_display_area(
         0, 0, static_cast<std::size_t>(cCols) * static_cast<std::size_t>(iRow + iCol)
      );
   } else if (iMode == 2) {
      // Erase the entire display.
      clear_display_area(0, 0, static_cast<std::size_t>(cCols) * static_cast<std::size_t>(cRows));
   }
}

void ansi_escape_parser::run_erase_row_sequence(int iMode) {
   ABC_TRACE_FUNC(this, iMode);

   std::int16_t iRow, iCol, cRows, cCols;
   get_cursor_pos_and_display_size(&iRow, &iCol, &cRows, &cCols);
   if (iMode == 0) {
      // Erase from the cursor position till the end of the row.
      clear_display_area(iRow, iCol, static_cast<std::size_t>(cCols - iCol));
   } else if (iMode == 1) {
      // Erase from column 0 of the cursor row up to the cursor position.
      clear_display_area(iRow, 0, static_cast<std::size_t>(iCol));
   } else if (iMode == 2) {
      // Erase current row.
      clear_display_area(iRow, 0, static_cast<std::size_t>(cCols));
   }
}

void ansi_escape_parser::run_sequence(char_t chCmd) {
   ABC_TRACE_FUNC(this, chCmd);

   if (m_chSeqStart == '[') {
      switch (chCmd) {
         case 'A': // Move cursor up N rows.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(-m_viCmdArgs[0], 0);
            }
            break;
         case 'B': // Move cursor down N rows.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(m_viCmdArgs[0], 0);
            }
            break;
         case 'C': // Move cursor right N columns.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, m_viCmdArgs[0]);
            }
            break;
         case 'D': // Move cursor left N columns.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, -m_viCmdArgs[0]);
            }
            break;
         case 'E': // Move cursor down N rows, column 1.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(m_viCmdArgs[0], 0, false, true);
            }
            break;
         case 'F': // Move cursor up N rows, column 1.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(-m_viCmdArgs[0], 0, false, true);
            }
            break;
         case 'G': // Move cursor to column N.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(0, m_viCmdArgs[0] - 1, false, true);
            }
            break;
         case 'H': // Move cursor to row N, column M.
         case 'f': // Move cursor to row N, column M.
            if (got_two_arguments(1, 1)) {
               safe_set_cursor_pos(m_viCmdArgs[0] - 1, m_viCmdArgs[1] - 1, true, true);
            }
            break;
         case 'J': // Erase the display from the cursor down (N=0), up (N=1), or everything (N=2).
            if (got_one_argument(0)) {
               run_erase_display_sequence(m_viCmdArgs[0]);
            }
            break;
         case 'K': // Erase chars right (N=0), left (N=1) or both (N=2) sides of the current column.
            if (got_one_argument(0)) {
               run_erase_row_sequence(m_viCmdArgs[0]);
            }
            break;
         case 'S': // Scroll text up N rows.
            if (got_one_argument(1)) {
               scroll_text(static_cast<std::int16_t>(-m_viCmdArgs[0]), 0);
            }
            break;
         case 'T': // Scroll text down N rows.
            if (got_one_argument(1)) {
               scroll_text(m_viCmdArgs[0], 0);
            }
            break;
         case 'd': // Move cursor to row N.
            if (got_one_argument(1)) {
               safe_set_cursor_pos(m_viCmdArgs[0] - 1, 0, true, false);
            }
            break;
         case 'm': // Set character attributes.
            got_one_argument(0);
            run_set_char_attributes_sequence();
            break;
         case 's': // Save cursor position.
            if (!m_viCmdArgs) {
               get_cursor_pos_and_display_size(&m_iSavedRow, &m_iSavedCol, nullptr, nullptr);
            }
            break;
         case 'u': // Restore saved cursor position.
            if (!m_viCmdArgs) {
               safe_set_cursor_pos(m_iSavedRow, m_iSavedCol, true, true);
            }
            break;
      }
   } else if (m_chSeqStart == ']') {
      if (m_viCmdArgs.size() == 1 && m_viCmdArgs[0] == 0) {
         set_window_title(m_sCmdArg);
      }
   } else if (m_chSeqStart == '?') {
      if ((chCmd == 'h' || chCmd == 'l') && m_viCmdArgs.size() == 1 && m_viCmdArgs[0] == 25) {
         set_cursor_visibility(chCmd == 'h');
      }
   }
}

void ansi_escape_parser::run_set_char_attributes_sequence() {
   ABC_TRACE_FUNC(this);

   for (auto it(m_viCmdArgs.cbegin()); it != m_viCmdArgs.cend(); ++it) {
      std::int16_t iCmdArg = *it;
      switch (iCmdArg) {
         case 0:
         case 39:
         case 49:
            if (iCmdArg != 49) {
               m_chattrCurr.clrForeground = m_chattrDefault.clrForeground;
            }
            if (iCmdArg != 39) {
               m_chattrCurr.clrBackground = m_chattrDefault.clrBackground;
            }
            if (iCmdArg == 0) {
               m_chattrCurr.iBlinkSpeed   = m_chattrDefault.iBlinkSpeed;
               m_chattrCurr.bConcealed    = m_chattrDefault.bConcealed;
               m_chattrCurr.bCrossedOut   = m_chattrDefault.bCrossedOut;
               m_chattrCurr.iIntensity    = m_chattrDefault.iIntensity;
               m_chattrCurr.bItalic       = m_chattrDefault.bItalic;
               m_chattrCurr.bReverseVideo = m_chattrDefault.bReverseVideo;
               m_chattrCurr.iUnderline    = m_chattrDefault.iUnderline;
            }
            break;
         case  1: m_chattrCurr.iIntensity    = 2;     break;
         case  2: m_chattrCurr.iIntensity    = 0;     break;
         case  3: m_chattrCurr.bItalic       = true;  break;
         case  4: m_chattrCurr.iUnderline    = 1;     break;
         case  5: m_chattrCurr.iBlinkSpeed   = 1;     break;
         case  6: m_chattrCurr.iBlinkSpeed   = 2;     break;
         case  7: m_chattrCurr.bReverseVideo = true;  break;
         case  8: m_chattrCurr.bConcealed    = true;  break;
         case  9: m_chattrCurr.bCrossedOut   = true;  break;
         case 21: // This would set double underline on rare terminals, but bold off on others.
         case 22: m_chattrCurr.iIntensity    = 1;     break;
         case 23: m_chattrCurr.bItalic       = false; break;
         case 24: m_chattrCurr.iUnderline    = 0;     break;
         case 25: m_chattrCurr.iBlinkSpeed   = 0;     break;
         case 27: m_chattrCurr.bReverseVideo = false; break;
         case 28: m_chattrCurr.bConcealed    = false; break;
         case 29: m_chattrCurr.bCrossedOut   = false; break;

         case 30: case 31: case 32: case 33: case 34: case 35: case 36: case 37:
            m_chattrCurr.clrForeground = static_cast<ansi_terminal_color::enum_type>(iCmdArg - 30);
            break;
         case 40: case 41: case 42: case 43: case 44: case 45: case 46: case 47:
            m_chattrCurr.clrBackground = static_cast<ansi_terminal_color::enum_type>(iCmdArg - 40);
            break;
      }
   }
   set_char_attributes();
}

void ansi_escape_parser::safe_set_cursor_pos(
   int iRow, int iCol, bool bAbsoluteRow /*= false*/, bool bAbsoluteCol /*= false*/
) {
   ABC_TRACE_FUNC(this, iRow, iCol, bAbsoluteRow, bAbsoluteCol);

   std::int16_t iCurrRow, iCurrCol, cRows, cCols;
   get_cursor_pos_and_display_size(&iCurrRow, &iCurrCol, &cRows, &cCols);

   // If iRow is relative, make it absolute; then clip it to the display height.
   if (!bAbsoluteRow) {
      iRow += iCurrRow;
   }
   if (iRow < 0) {
      if (bAbsoluteRow) {
         // A negative value in absolute mode means use the current value.
         iRow = iCurrRow;
      } else {
         iRow = 0;
      }
   } else if (iRow >= cRows) {
      iRow = cRows - 1;
   }

   // If iCol is relative, make it absolute; then clip it to the display width.
   if (!bAbsoluteCol) {
      iCol += iCurrCol;
   }
   if (iCol < 0) {
      if (bAbsoluteCol) {
         // A negative value in absolute mode means use the current value.
         iCol = iCurrCol;
      } else {
         iCol = 0;
      }
   } else if (iCol >= cCols) {
      iCol = cCols - 1;
   }

   set_cursor_pos(static_cast<std::int16_t>(iRow), static_cast<std::int16_t>(iCol));
}

} //namespace text
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////


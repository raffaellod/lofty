/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2016 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>
#include <abaclade/collections/vector.hxx>
#include <abaclade/text/parsers/dynamic.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace parsers {

dynamic::state::state() {
   pstNext = nullptr;
   pstAlternative = nullptr;
   u.range.cpFirst = '\0';
   u.range.cpLast   = '\0';
   u.repetition.pstOut = nullptr;
   u.repetition.cMin = 0;
   u.repetition.cMax = 0;
   st = state_type::range;
}


struct dynamic::backtrack {
   backtrack(state_t const * pstAlternative_, bool bConsumedCp_) :
      pstAlternative(pstAlternative_),
      bConsumedCp(bConsumedCp_) {
   }

   state_t const * pstAlternative;
   bool bConsumedCp;
};


struct dynamic::repetition {
   explicit repetition(state_t const * pmnAnchor_) :
      pmnAnchor(pmnAnchor_),
      c(0),
      bBacktracking(false) {
   }

   state_t const * pmnAnchor;
   std::uint16_t c;
   bool bBacktracking:1;
};


dynamic::dynamic() :
   m_pstInitial(nullptr) {
}

dynamic::dynamic(dynamic && dp) :
   m_qmn(_std::move(dp.m_qmn)),
   m_pstInitial(dp.m_pstInitial) {
   dp.m_pstInitial = nullptr;
}

dynamic::~dynamic() {
}

dynamic::state * dynamic::create_state() {
   m_qmn.push_back(state_t());
   return static_cast<state *>(&m_qmn.back());
}

bool dynamic::run(str const & s) const {
   io::text::str_istream sis(external_buffer, &s);
   return run(&sis);
}

bool dynamic::run(io::text::istream * ptis) const {
   state_t const * pstCurr = m_pstInitial;
   // Setup the two sources of code points: a backtrack and a peek buffer from the input stream.
   str sPeek = ptis->peek_chars(1);
   auto itPeek(sPeek.cbegin()), itPeekEnd(sPeek.cend());
   str sHistory;
   auto itHistory(sHistory.cbegin()), itHistoryEnd(sHistory.cend());

   collections::vector<backtrack> vbtStack;
   collections::vector<repetition> vrepStack;

   // The first code point must necessarily come from sPeek, since sHistory is empty.
   bool bAccepted = false;
   while (pstCurr) {
      state_t const * pstNext = nullptr;
      bool bConsumedCp = false;
      switch (pstCurr->st) {
         case state_type::range: {
            // Get a code point from either history or the peek buffer.
            char32_t cp;
            bool bSaveCpToHistory;
            if (itHistory != itHistoryEnd) {
               cp = *itHistory;
               bSaveCpToHistory = false;
            } else {
               if (itPeek == itPeekEnd) {
                  ptis->consume_chars(sPeek.size_in_chars());
                  sPeek = ptis->peek_chars(1);
                  itPeek = sPeek.cbegin();
                  itPeekEnd = sPeek.cend();
                  if (itPeek == itPeekEnd) {
                     // Run out of code points.
                     break;
                  }
               }
               cp = *itPeek;
               bSaveCpToHistory = true;
            }

            if (cp >= pstCurr->u.range.cpFirst && cp <= pstCurr->u.range.cpLast) {
               bAccepted = true;
               pstNext = pstCurr->pstNext;
               if (bSaveCpToHistory) {
                  sHistory += cp;
                  ++itHistoryEnd;
               }
               ++itHistory;
               bConsumedCp = true;
            }
            break;
         }

         case state_type::repetition:
            /* TODO: this behavior is greedy; add support for non-greedy behavior, i.e. trying to
            match the least number of occurrences first. */
            if (vrepStack && vrepStack.front().pmnAnchor == pstCurr) {
               /* We just got back to the repetition at the top of the stack. Check if we should try
               to match it once more. */
               repetition & rep = vrepStack.front();
               if (rep.bBacktracking) {
                  if (rep.c-- == pstCurr->u.repetition.cMin) {
                     /* Can’t match this repetition any fewer times; this means that the repetition
                     can’t be matched. */
                     ;
                  } else {
                     /* We already know that the repetition can be matched one fewer time, and the
                     stacks already account for that; just skip straight to out of it. */
                     bAccepted = true;
                     pstNext = pstCurr->u.repetition.pstOut;
                  }
               } else {
                  bAccepted = true;
                  if (++rep.c == pstCurr->u.repetition.cMax) {
                     /* Can’t match this repetition any more times; if we get back here, we’ll try
                     to match it fewer times instead. */
                     pstNext = pstCurr->u.repetition.pstOut;
                     rep.bBacktracking = true;
                  } else {
                     pstNext = pstCurr->pstNext;
                  }
               }
            } else {
               // New repetition: save it on the stack, and begin counting.
               vrepStack.push_back(repetition(pstCurr));
               bAccepted = true;
               pstNext = pstCurr->pstNext;
            }
            break;

         case state_type::begin:
            if (itHistory == sHistory.cbegin()) {
               bAccepted = true;
               pstNext = pstCurr->pstNext;
            }
            break;

         case state_type::end:
            if (itHistory == itHistoryEnd && itPeek == itPeekEnd) {
               /* We consumed history and peek buffer, but “end” really means end, so also check
               that the stream is empty. */
               sPeek = ptis->peek_chars(1);
               if (!sPeek) {
                  bAccepted = true;
                  pstNext = pstCurr->pstNext;
               }
            }
            break;
      }

      if (bAccepted) {
         if (!pstNext) {
            break;
         }
         // One or more states to check still; this means that we can’t accept the input just yet.
         bAccepted = false;
         vbtStack.push_back(backtrack(pstCurr->pstAlternative, bConsumedCp));
         pstCurr = pstNext;
      } else {
         // Consider the next alternative.
         pstCurr = pstCurr->pstAlternative;
         // Go back to a state that had alternatives, if possible.
         while (!pstCurr) {
            if (!vbtStack) {
               // Run out of previous states with alternatives.
               break;
            }
            backtrack bt(vbtStack.pop_back());
            pstCurr = bt.pstAlternative;
            /* If the state we’re rolling back consumed a code point, it must’ve saved it in
            sHistory. */
            if (bt.bConsumedCp) {
               --itHistory;
            }
         }
      }
   }
   return bAccepted;
}

}}} //namespace abc::text::parsers

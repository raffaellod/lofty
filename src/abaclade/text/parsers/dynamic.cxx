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

namespace {

//! Backtracking data structure.
struct backtrack {
   dynamic::state const * pst;
   bool bConsumedCp:1;
   bool bRepetition:1;
   bool bAcceptedRepetition:1;

   static backtrack default_(dynamic::state const * pst, bool bConsumedCp) {
      backtrack bt;
      bt.pst = pst;
      bt.bConsumedCp = bConsumedCp;
      bt.bRepetition = false;
      bt.bAcceptedRepetition = false;
      return _std::move(bt);
   }

   static backtrack repetition(dynamic::state const * pst, bool bAccepted) {
      backtrack bt;
      bt.pst = pst;
      bt.bConsumedCp = false;
      bt.bRepetition = true;
      bt.bAcceptedRepetition = bAccepted;
      return _std::move(bt);
   }
};

} //namespace

namespace {

//! Used to track the acceptance of repetition states.
struct repetition {
   explicit repetition(dynamic::state const * pst_) :
      pst(pst_),
      c(0) {
   }

   dynamic::state const * pst;
   std::uint16_t c;
};

} //namespace


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

dynamic::state * dynamic::create_code_point_state(char32_t cp) {
   state * pst = create_uninitialized_state(state_type::range);
   pst->u.range.cpFirst = cp;
   pst->u.range.cpLast = cp;
   return pst;
}

dynamic::state * dynamic::create_code_point_range_state(char32_t cpFirst, char32_t cpLast) {
   state * pst = create_uninitialized_state(state_type::range);
   pst->u.range.cpFirst = cpFirst;
   pst->u.range.cpLast = cpLast;
   return pst;
}

dynamic::state * dynamic::create_repetition_state(
   state const * pstRepeated, std::uint16_t cMin, std::uint16_t cMax /*= 0*/
) {
   state * pst = create_uninitialized_state(state_type::repetition);
   pst->u.repetition.pstRepeated = pstRepeated;
   pst->u.repetition.cMin = cMin;
   pst->u.repetition.cMax = cMax;
   pst->u.repetition.bGreedy = true;
   return pst;
}

dynamic::state * dynamic::create_uninitialized_state(state_type st) {
   m_qmn.push_back(state());
   state * pst = static_cast<state *>(&m_qmn.back());
   pst->st = st.base();
   pst->pstNext = nullptr;
   pst->pstAlternative = nullptr;
   return pst;
}

bool dynamic::run(str const & s) const {
   io::text::str_istream sis(external_buffer, &s);
   return run(&sis);
}

bool dynamic::run(io::text::istream * ptis) const {
   state const * pstCurr = m_pstInitial;
   // Cache this condition to quickly determine whether we’re allowed to skip input code points.
   bool bBeginAnchor = (pstCurr && pstCurr->st == state_type::begin && !pstCurr->pstAlternative);
   // Setup the two sources of code points: a history and a peek buffer from the input stream.
   str sHistory;
   auto itHistoryBegin(sHistory.cbegin()), itHistory(itHistoryBegin), itHistoryEnd(sHistory.cend());
   str sPeek = ptis->peek_chars(1);
   auto itPeek(sPeek.cbegin()), itPeekEnd(sPeek.cend());

   // TODO: change these two variables to use collections::stack once that’s available.
   collections::vector<backtrack> vbtStack;
   /* Stack of repetition counters. A new counter is pushed when a new repetition is started, and
   popped when that repetition is a) matched cMax times or b) backtracked over. */
   collections::vector<repetition> vrepStack;

   bool bAccepted = false;
   while (pstCurr) {
      state const * pstNext = nullptr;
      bool bConsumedCp = false;
      switch (pstCurr->st) {
         case state_type::range: {
            // Get a code point from either history or the peek buffer.
            char32_t cp;
            bool bSavePeekCpToHistory;
            if (itHistory != itHistoryEnd) {
               cp = *itHistory;
               bSavePeekCpToHistory = false;
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
               bSavePeekCpToHistory = true;
            }

            if (cp >= pstCurr->u.range.cpFirst && cp <= pstCurr->u.range.cpLast) {
               bAccepted = true;
               bConsumedCp = true;
               pstNext = pstCurr->pstNext;
               if (bSavePeekCpToHistory) {
                  sHistory += cp;
                  ++itHistoryEnd;
                  ++itPeek;
               }
               ++itHistory;
            }
            break;
         }

         case state_type::repetition:
            repetition * prep;
            if (vrepStack && (prep = &vrepStack.front(), prep->pst == pstCurr)) {
               ++prep->c;
            } else {
               // New repetition: save it on the stack and begin counting.
               vrepStack.push_back(repetition(pstCurr));
               prep = &vrepStack.front();
            }
            if (pstCurr->u.repetition.cMax == 0 || prep->c <= pstCurr->u.repetition.cMax) {
               if (prep->c >= pstCurr->u.repetition.cMin) {
                  // Repetitions within [cMin, cMax] are accepting.
                  bAccepted = true;
               }
               // Try one more repetition.
               pstNext = pstCurr->u.repetition.pstRepeated;
            } else {
               // Repeated cMax times; pop the stack and move on to the next state.
               vrepStack.pop_back();
               pstNext = pstCurr->pstNext;
            }

            // This code is very similar to the “if (bAccepted)” below.
            if (!pstNext) {
               // No more states; this means that the input was accepted.
               break;
            }
            vbtStack.push_back(backtrack::repetition(pstCurr, bAccepted));
            bAccepted = false;
            pstCurr = pstNext;
            // Skip the accept/backtrack logic at the end of the loop.
            continue;

         case state_type::begin:
            if (itHistory == itHistoryBegin) {
               bAccepted = true;
               pstNext = pstCurr->pstNext;
            }
            break;

         case state_type::end:
            if (itHistory == itHistoryEnd && itPeek == itPeekEnd) {
               /* We consumed history and peek buffer, but “end” really means end, so also check
               that the stream is empty. */
               /* TODO: this might be redundant, since other code point consumers in this function
               always do this after consuming a code point. */
               ptis->consume_chars(sPeek.size_in_chars());
               sPeek = ptis->peek_chars(1);
               if (!sPeek) {
                  bAccepted = true;
                  pstNext = pstCurr->pstNext;
               }
            }
            break;

         case state_type::look_ahead:
            // TODO: implement look-ahead assertions.
            break;
      }

      if (bAccepted) {
         if (!pstNext) {
            // No more states; this means that the input was accepted.
            break;
         }
         // Still one or more states to check; this means that we can’t accept the input just yet.
         vbtStack.push_back(backtrack::default_(pstCurr, bConsumedCp));
         bAccepted = false;
         pstCurr = pstNext;
      } else {
         // Consider the next alternative.
         pstCurr = pstCurr->pstAlternative;
         // Go back to a state that had alternatives, if possible.
         while (!pstCurr && vbtStack) {
            backtrack bt(vbtStack.pop_back());
            // If we’re backtracking the current top-of-the-stack repetition, pop it out of it.
            if (bt.bRepetition && vrepStack && vrepStack.back().pst == bt.pst) {
               vrepStack.pop_back();
            }
            if (bt.bAcceptedRepetition) {
               // This must be a repetition’s Nth occurrence, with N in the acceptable range.
               if (!bt.pst->pstNext) {
                  // If there was no following state, the input is accepted.
                  bAccepted = true;
                  goto break_outer_while;
               }
               pstCurr = bt.pst->pstNext;
            } else {
               // Not a repetition, or Nth occurrence with N not in the acceptable range.
               pstCurr = bt.pst->pstAlternative;
            }
            /* If the state we’re rolling back consumed a code point, it must’ve saved it in
            sHistory. */
            if (bt.bConsumedCp) {
               --itHistory;
            }
         }
         /* If we run out of alternatives and can’t backtrack any further, and the pattern is not
         anchored, we’re allowed to move one code point to history and try the whole pattern again
         from the initial state. */
         if (!pstCurr && !bBeginAnchor) {
            if (itHistory == itHistoryEnd) {
               if (itPeek == itPeekEnd) {
                  ptis->consume_chars(sPeek.size_in_chars());
                  sPeek = ptis->peek_chars(1);
                  if (!sPeek) {
                     // Run out of code points.
                     break;
                  }
                  itPeek = sPeek.cbegin();
                  itPeekEnd = sPeek.cend();
               }
               sHistory += *itPeek++;
               ++itHistoryEnd;
            }
            ++itHistory;
            pstCurr = m_pstInitial;
         }
      }
   }
break_outer_while:
   return bAccepted;
}

}}} //namespace abc::text::parsers

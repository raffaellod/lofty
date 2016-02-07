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

struct dynamic::backtrack {
   backtrack(state const * pst_, bool bConsumedCp_, bool bAcceptedRepetition_) :
      pst(pst_),
      bConsumedCp(bConsumedCp_),
      bAcceptedRepetition(bAcceptedRepetition_) {
   }

   state const * pst;
   bool bConsumedCp:1;
   bool bAcceptedRepetition:1;
};


struct dynamic::repetition {
   explicit repetition(state const * pmnAnchor_) :
      pmnAnchor(pmnAnchor_),
      c(0) {
   }

   state const * pmnAnchor;
   std::uint16_t c;
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

   collections::vector<backtrack> vbtStack;
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
            /* TODO: a stack doesn’t work for nested repetitions; the inner-most one will see on the
            top of the stack its own previous repetition instance, messing up the counting, and the
            outer-most ones won’t be able to see themselves in the stack, also messing up the
            counting. */
            repetition * prep;
            if (vrepStack && (prep = &vrepStack.front(), prep->pmnAnchor == pstCurr)) {
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
               // Repeated cMax times; move on to the next state.
               pstNext = pstCurr->pstNext;
            }

            // This code is nearly identical to the “if (bAccepted)” below.

            if (!pstNext) {
               // No more states; this means that the input was accepted.
               break;
            }
            vbtStack.push_back(backtrack(pstCurr, bConsumedCp, bAccepted));
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
      }

      if (bAccepted) {
         if (!pstNext) {
            // No more states; this means that the input was accepted.
            break;
         }
         // Still one or more states to check; this means that we can’t accept the input just yet.
         bAccepted = false;
         vbtStack.push_back(backtrack(pstCurr, bConsumedCp, false /*not an accepted repetition*/));
         pstCurr = pstNext;
      } else {
         // Consider the next alternative.
         pstCurr = pstCurr->pstAlternative;
         // Go back to a state that had alternatives, if possible.
         while (!pstCurr && vbtStack) {
            backtrack bt(vbtStack.pop_back());
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

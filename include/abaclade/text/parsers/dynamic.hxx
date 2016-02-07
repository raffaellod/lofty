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

#ifndef _ABACLADE_TEXT_PARSERS_DYNAMIC_HXX
#define _ABACLADE_TEXT_PARSERS_DYNAMIC_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
   // Note: this file may be included while _ABACLADE_HXX_INTERNAL is defined.
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/collections/queue.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace text { namespace parsers {

/*! Parser that accepts input based on a dynamically-configurable state machine.

For the ERE pattern “a”, the state machine would be:

   @verbatim
   ┌───┬───────┐
   │“a”│nullptr│
   └───┴───────┘
   @endverbatim
*/
class ABACLADE_SYM dynamic {
public:
   //! Possible state_t types.
   ABC_ENUM_AUTO_VALUES(state_type,
      //! Begin matcher (“^”).
      begin,
      //! End matcher (“$”).
      end,
      //! Code point or code point range matcher (e.g. “a”, “[a-z]”).
      range,
      //! Repetition matcher; repeatedly matches the states that follow it.
      repetition
   );

   //! Internal state representation. Public to allow instances to be statically allocated.
   struct state_t {
      //! Pointer to the next state if this one accepts.
      state_t const * pstNext;
      //! Pointer to an alternate state to try if this one does not accept.
      state_t const * pstAlternative;
      union {
         //! Range data.
         struct {
            //! First character accepted by the range.
            char32_t cpFirst;
            //! Last character accepted by the range.
            char32_t cpLast;
         } range;
         //! Repetition data.
         struct {
            //! Pointer to the first state to be matched repeatedly.
            state_t const * pstRepeated;
            //! Minimum number of repetitions needed to accept.
            std::uint16_t cMin;
            //! Maximum number of repetitions needed to accept.
            std::uint16_t cMax;
            bool bGreedy;
         } repetition;
      } u;
      state_type::enum_type st;
   };

public:
   /*! Publicly-accessible state representation. Instances are created by dynamic::create_state(),
   and must be configured by calling one of the set_*() methods. */
   class state : public state_t {
   private:
      friend class dynamic;

   public:
      //! Makes the state accept the start of the input.
      void set_begin() {
         st = state_type::begin;
      }

      /*! Makes the state accept the specified code point.

      @param cp
         Code point to match.
      */
      void set_code_point(char32_t cp) {
         st = state_type::range;
         u.range.cpFirst = cp;
         u.range.cpLast = cp;
      }

      /*! Makes the state accept a code point from the specified inclusive range.

      @param cpFirst
         First code point in the range.
      @param cpLast
         Last code point in the range.
      */
      void set_code_point_range(char32_t cpFirst, char32_t cpLast) {
         st = state_type::range;
         u.range.cpFirst = cpFirst;
         u.range.cpLast = cpLast;
      }

      //! Makes the state accept the end of the input.
      void set_end() {
         st = state_type::end;
      }

      /*! Makes the state accept a code point from the specified inclusive range.

      @param pstRepeated
         Pointer to the first state of the repetition.
      @param cMin
         Minimum number of repetitions needed to accept.
      @param cMax
         Maximum number of repetitions needed to accept.
      */
      void set_repetition(state_t const * pstRepeated, std::uint16_t cMin, std::uint16_t cMax) {
         st = state_type::repetition;
         u.repetition.pstRepeated = pstRepeated;
         u.repetition.cMin = cMin;
         u.repetition.cMax = cMax;
         u.repetition.bGreedy = true;
      }

      /*! Assigns the state that will follow if this one accepts.

      @param pst
         Next state.
      */
      void set_next(state_t * pst) {
         pstNext = pst;
      }

   private:
      //! Default constructor.
      state();
   };

private:
   //! Backtracking data structure.
   struct backtrack;
   //! Used to track the acceptance of repetition states.
   struct repetition;

public:
   //! Default constructor.
   dynamic();

   /*! Move constructor.

   @param dp
      Source object.
   */
   dynamic(dynamic && dp);

   //! Destructor.
   ~dynamic();

   /*! Creates a parser state. Each newly-created state must be configured by calling one of its
   set_*() methods.

   @return
      New parser state. The instance is owned by the parser; the returned pointer should not be
      stored independently.
   */
   state * create_state();

   /*! Runs the parser against the specified string.

   @param s
      String to parse.
   @return
      true if the string is accepted by the parser, or false otherwise.
   */
   bool run(str const & s) const;

   /*! Runs the parser against the specified text stream, consuming code points from it as
   necessary.

   @param ptis
      Pointer to the stream to parse.
   @return
      true if the contents of the stream were accepted by the parser, or false otherwise.
   */
   bool run(io::text::istream * ptis) const;

   /*! Assigns an initial state. If not called, the parser will remain empty, accepting all input.

   @param pstInitial
      Pointer to the new initial state.
   */
   void set_initial_state(state_t const * pstInitial) {
      m_pstInitial = pstInitial;
   }

private:
   //! List of states.
   /* TODO: change to a singly-linked list; it’s a queue now just because collections::list is a
   doubly-linked list, which is unnecessary since we never remove states from the parser, only
   add. */
   collections::queue<state_t> m_qmn;
   //! Pointer to the initial state.
   state_t const * m_pstInitial;
};

}}} //namespace abc::text::parsers

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TEXT_PARSERS_DYNAMIC_HXX

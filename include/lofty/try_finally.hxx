/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Declaration of LOFTY_TRY and LOFTY_FINALLY. */

#ifndef _LOFTY_TRY_FINALLY_HXX
#define _LOFTY_TRY_FINALLY_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Implementation of LOFTY_TRY … LOFTY_FINALLY.
template <typename Try>
struct try_finally {
   //! The “try” block.
   Try try_block;

   /*! Constructor. Accepts and holds the try block.

   @param try_block_
      The “try” block.
   */
   explicit try_finally(Try try_block_) :
      try_block(_std::move(try_block_)) {
   }

   /*! Allows to associate (literally, by operator associativity) a finally block to the try block, without
   needing any closed parentheses after the finally block. A semicolon is still required though.

   @param finally_block
      The “finally” block.
   */
   template <typename Finally>
   void operator||(Finally finally_block) {
      try {
         try_block();
      } catch (...) {
         /* TODO: save the current exception and rethrow it at the end of this block, so it can’t be
         overwritten by a newer exception thrown in finally_block(). */
         finally_block();
         throw;
      }
      finally_block();
   }
};

/*! Deduces the Try type for try_finally.

@param try_block
   The “try” block.
@return
   An object holding try_block, ready to associate it to a “finally” block.
*/
template <typename Try>
try_finally<Try> make_try_finally(Try try_block) {
   return try_finally<Try>(_std::move(try_block));
}

}} //namespace lofty::_pvt

/*! Allows to execute code at the end of a block, regardless of whether exceptions were thrown in it, but
without blocking exceptions from reaching outside the block. This is identical to a try … finally statement in
Python, Java and other languages.

@code
int i = 1;
{
   ++i;
   LOFTY_TRY {
      risky_operation_that_may_throw();
   } LOFTY_FINALLY {
      --i;
   }
}
// At this point i is guaranteed to be 1, even if an exception was thrown.
@endcode
*/
#define LOFTY_TRY \
   ::lofty::_pvt::make_try_finally( \
      [&]

//! Declares statements to be executed at the end of LOFTY_TRY block.
#define LOFTY_FINALLY \
   ) || [&]

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TRY_FINALLY_HXX

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#ifndef _ABACLADE_DEFER_TO_SCOPE_END_HXX
#define _ABACLADE_DEFER_TO_SCOPE_END_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! @cond
Implementation of abc::defer_to_scope_end(). This cannot define a move constructor (and delete
the copy constructor), because if F is a lambda it will lack means for the destructor to check if
it’s been moved out (no operator bool() support); instead, just don’t declare any move/copy
constructors or operators, and rely on the compiler to implement RVO so not to generate more than
one instance for each call to abc::defer_to_scope_end(). */
template <typename F>
class deferred_to_scope_end {
public:
   /*! Constructor.

   @param fn
      Code to execute when the object goes out of scope.
   */
   deferred_to_scope_end(F fn) :
      m_fn(std::move(fn)) {
   }

   //! Destructor. Most important part of the class.
   ~deferred_to_scope_end() {
      m_fn();
   }

private:
   //! Code to execute in the destructor.
   F m_fn;
};
//! @endcond

}} //namespace abc::detail

namespace abc {

/*! Ensures that a function or lambda will execute when the enclosing scope ends. This works by
returning an object whose lifetime will track that of the enclosing scope; when the object is
destructed, the function/lambda is executed. This is functionally equivalent to a try … finally
statement in Python and other languages.

@code
int i = 1;
{
   ++i;
   auto deferred1(abc::defer_to_scope_end([&i] () {
      --i;
   }));
   risky_operation_that_may_throw();
}
// At this point i is guaranteed to be 1, even if an exception was thrown.
@endcode

In order to avoid violating Abaclade’s (and common sense) requirement that destructors never throw
exceptions, the lambda should only perform simple, fail-proof tasks, such as changing the value or a
local or global variable.

Note that this relies on the compiler to implement return value optimization, which means that one
call to defer_to_scope_end() must result in the creation of a single
abc::detail::deferred_to_scope_end instance (see that class for more information).

@param fn
   Code to execute when the returned object goes out of scope.
@return
   Object that will track the enclosing scope.
*/
template <typename F>
inline detail::deferred_to_scope_end<F> defer_to_scope_end(F fn) {
   return detail::deferred_to_scope_end<F>(std::move(fn));
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_DEFER_TO_SCOPE_END_HXX

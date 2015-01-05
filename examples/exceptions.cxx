/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014, 2015
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

/*! DOC:0105 Exceptions and stack traces example

This program showcases Abaclade’s ability to display stack traces when an exception is thrown, as
well as its support for catching null pointer access and similar invalid operations. See the source
code for more comments.
*/

#include <abaclade.hxx>
#include <abaclade/app.hxx>
using namespace abc;


////////////////////////////////////////////////////////////////////////////////////////////////////
// exceptions_app

//! Application class for this program.
class exceptions_app : public app {
public:
   /*! Main function of the program.

   vsArgs
      Arguments that were provided to this program via command line.
   return
      Return value of this program.
   */
   virtual int main(mvector<istr const> const & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      istr s(ABC_SL("Test String"));

      smvector<int, 5> vi;
      vi.append(101);
      vi.append(102);

      auto out(io::text::stdout());
      out->print(ABC_SL("Populated vi with {} and {}\n"), vi[0], vi[1]);

      out->write_line(ABC_SL("Before calling first_function()"));
      out->write_line();
      first_function(s, vi);

      // This will never happen.
      out->write_line(ABC_SL("After calling first_function()"));

      return 0;
   }

   /*! Sample enumeration. Used to demonstrate Abaclade’s support for automatic translation of
   enumerated values into strings. */
   ABC_ENUM(numbers_enum,
      (zero,  0),
      (one,   1),
      (two,   2),
      (three, 3),
      (four,  4)
   );

   void first_function(istr const & s, mvector<int> const & vi) const {
      ABC_TRACE_FUNC(this, s, vi);

      auto out(io::text::stdout());
      exception::write_with_scope_trace(out.get());
      out->write_line();

      out->write_line(ABC_SL("Before calling is_zero()"));
      out->write_line();

      // Passing a null pointer!
      is_zero(numbers_enum::two, nullptr);

      // This will never happen.
      out->write_line(ABC_SL("After calling is_zero()"));
   }

   void is_zero(numbers_enum ne, bool * pbRet) const {
      ABC_TRACE_FUNC(this, ne, pbRet);

      *pbRet = (ne == numbers_enum::zero);
   }
};

ABC_APP_CLASS(exceptions_app)

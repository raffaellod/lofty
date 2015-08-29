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

#include <abaclade.hxx>
#include <abaclade/app.hxx>
using namespace abc;


class test_app : public app {
public:
   virtual int main(mvector<str> & vsArgs) override {
      ABC_TRACE_FUNC(this, vsArgs);

      ABC_UNUSED_ARG(vsArgs);

      auto ptwOut(io::text::stdout());
      ptwOut->set_encoding(text::encoding::utf8);
      ptwOut->write(ABC_SL("I/O test file encoded using "));
      ptwOut->write(ABC_SL("UTF-8"));

      // Test result determined by Abamake’s output comparer.
      return 0;
   }
};

ABC_APP_CLASS(test_app)

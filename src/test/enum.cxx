/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011, 2012, 2013
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abc/module.hxx>
#include <abc/str_iostream.hxx>
#include <abc/trace.hxx>
using namespace abc;


ABC_ENUM(test_enum, \
   (value1, 15), \
   (value2, 56), \
   (value3, 91) \
);

class test_app_module :
   public app_module_impl<test_app_module> {
public:

   int main(mvector<istr const> const & vsArgs) {
      ABC_TRACE_FN((this/*, vsArgs*/));

      ABC_UNUSED_ARG(vsArgs);

      test_enum e(test_enum::value2);

      if (e != test_enum::value2) {
         return 1;
      }
      if (to_str(e) != SL("value2")) {
         return 2;
      }

      return 0;
   }
};

ABC_MAIN_APP_MODULE(test_app_module)


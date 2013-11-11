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
#include <abc/map.hxx>
#include <abc/trace.hxx>
using namespace abc;



class test_app_module :
   public app_module_impl<test_app_module> {
public:

   int main(mvector<istr const> const & vsArgs) {
      ABC_TRACE_FN((this/*, vsArgs*/));

      ABC_UNUSED_ARG(vsArgs);
      {
         // Simple manipulation tests.
         map<istr const, int> m;
         m.add(SL("1"), 1);
         if (m.get_size() != 1 || m[SL("1")] != 1) {
            return 1;
         }

         m.add(SL("2"), 2);
         m.add(SL("3"), 3);
         m.add(SL("4"), 4);
         m.add(SL("5"), 5);
      }

      return 0;
   }
};

ABC_MAIN_APP_MODULE(test_app_module)


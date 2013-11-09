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
#include <abc/file_iostream.hxx>
#include <abc/trace.hxx>
using namespace abc;



class test_app_module :
   public app_module_impl<test_app_module> {
public:

   int main(mvector<istr const> const & vsArgs) {
      ABC_TRACE_FN((this/*, vsArgs*/));

      std::shared_ptr<file_istream> pfis;

      size_t cArgs(vsArgs.size());
      if (cArgs >= 2 && vsArgs[1] == SL("-i")) {
         pfis = file_istream::stdin();
      } else if (cArgs >= 3 && vsArgs[1] == SL("-f") && vsArgs[2]) {
         pfis = std::make_shared<file_istream>(vsArgs[2]);
      }

      if (!pfis) {
         return EXIT_FAILURE;
      }
      auto pstdout(file_ostream::stdout());
      for (dmstr s; pfis->read_line(&s); ) {
         pstdout->write(s);
         pstdout->write(SL("\n"));
      }

      // Test results determined by external program.
      return EXIT_SUCCESS;
   }
};

ABC_MAIN_APP_MODULE(test_app_module)


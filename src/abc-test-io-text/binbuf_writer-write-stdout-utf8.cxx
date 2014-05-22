/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#include <abc.hxx>
#include <abc/module.hxx>
#include <abc/io/text/file.hxx>
using namespace abc;



class test_app_module :
   public app_module_impl<test_app_module> {
public:

   int main(mvector<istr const> const & vsArgs) {
      ABC_TRACE_FN((this/*, vsArgs*/));
      ABC_UNUSED_ARG(vsArgs);

      auto ptwOut(io::text::stdout());
      ptwOut->set_encoding(text::encoding::utf8);
      ptwOut->write(SL("I/O test file encoded using "));
      ptwOut->write(SL("UTF-8"));

      // Test result determined by ABC Make.
      return 0;
   }
};

ABC_MAIN_APP_MODULE(test_app_module)


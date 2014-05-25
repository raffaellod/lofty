/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
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

#ifndef _ABC_TESTING_MODULE_HXX
#define _ABC_TESTING_MODULE_HXX

#ifndef _ABC_HXX
   #error Please #include <abc.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/module.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::app_module


namespace abc {
namespace testing {

/** Testing application module. It interacts with registered abc::testing::test_case-derived
classes, allowing for the execution of test cases.
*/
class ABCTESTINGAPI app_module :
   public abc::app_module {
public:

   /** See abc::app_module::main().
   */
   virtual int main(mvector<istr const> const & vsArgs);
};

} //namespace testing
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_TESTING_MODULE_HXX


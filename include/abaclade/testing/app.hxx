/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013, 2014
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

#ifndef _ABACLADE_TESTING_APP_HXX
#define _ABACLADE_TESTING_APP_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/app.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::app

namespace abc {
namespace testing {

/*! Testing application. It interacts with registered abc::testing::test_case-derived classes,
allowing for the execution of test cases. */
class ABACLADE_TESTING_SYM app : public abc::app {
public:
   //! See abc::app::main().
   virtual int main(mvector<istr const> const & vsArgs) override;
};

} //namespace testing
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_TESTING_APP_HXX

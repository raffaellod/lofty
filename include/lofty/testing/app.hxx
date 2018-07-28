/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2013-2015, 2017-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_TESTING_APP_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_TESTING_APP_HXX
#endif

#ifndef _LOFTY_TESTING_APP_HXX_NOPUB
#define _LOFTY_TESTING_APP_HXX_NOPUB

#include <lofty/app.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace testing {
_LOFTY_PUBNS_BEGIN

/*! Testing application. It interacts with registered lofty::testing::test_case-derived classes, allowing for
the execution of test cases. */
class LOFTY_TESTING_SYM app : public lofty::_LOFTY_PUBNS app {
public:
   //! See lofty::app::main().
   virtual int main(collections::_LOFTY_PUBNS vector<text::_LOFTY_PUBNS str> & args) override;
};

_LOFTY_PUBNS_END
}}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_TESTING_APP_HXX_NOPUB

#ifdef _LOFTY_TESTING_APP_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace testing {
      using _pub::app;
   }}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_TESTING_APP_HXX

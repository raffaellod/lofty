/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/app.hxx>
#include <lofty/logging.hxx>
using namespace lofty;

class test_app : public app {
public:
   virtual int main(collections::vector<text::str> & args) override {
      LOFTY_TRACE_METHOD();

      LOFTY_UNUSED_ARG(args);

      auto stdout(io::text::stdout());
      stdout->set_encoding(text::encoding::utf8);
      stdout->write(LOFTY_SL("I/O test file encoded using "));
      stdout->write(LOFTY_SL("UTF-8"));

      // Test result determined by Complemake’s output comparer.
      return 0;
   }
};

LOFTY_APP_CLASS(test_app)

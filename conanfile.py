#!/usr/bin/env python
# -*- coding: utf-8; mode: python; tab-width: 3; indent-tabs-mode: nil -*-
#
# Copyright 2017 Brent Dimmig
#
# This file is part of Lofty.
#
# Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
# Lesser General Public License as published by the Free Software Foundation.
#
# Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#-------------------------------------------------------------------------------------------------------------

from conans import ConanFile, CMake


class LoftyConan(ConanFile):
    name = "lofty"
    version = "0.2.1"
    license = "LGPL-2.1"
    url = "https://github.com/raffaellod/lofty"
    description = "Coroutines, stack traces and smart I/O for C++11, inspired by Python and Golang."
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False]}
    default_options = "shared=True"
    generators = "cmake"

    def build(self):
        cmake = CMake(self)
        cmake.configure(source_dir=self.source_folder)
        cmake.build()

    def package(self):
        self.copy(pattern="*.hxx", dst="include", src="include")
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["lofty"]


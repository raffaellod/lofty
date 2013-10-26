# -*- coding: utf-8; mode: Makefile; tab-width: 3 -*-
#
# Copyright 2010, 2011, 2012, 2013
# Raffaello D. Di Napoli
#
# This file is part of Application-Building Components (henceforth referred to as ABC).
#
# ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
# Public License as published by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along with ABC. If not, see
# <http://www.gnu.org/licenses/>.
#---------------------------------------------------------------------------------------------------

LDLIBS+=-labc

srcdir:=src
generatedsources:=

ABC_PATH:=./
include $(ABC_PATH)Makefile.inc

CPPFLAGS+=-Iinclude
LDFLAGS+=-L$(O)lib


####################################################################################################
# Phony targets

all: \
	$(O)lib/libabc$(LIBEXT)

test: \
	$(O)bin/abc-test$(EXEEXT)

check: \
	$(O)test.tmp/unittest/0005-cppmacros.cxx.i.log \
	$(O)test.tmp/unittest/0010-module.ut.log \
	$(O)test.tmp/unittest/0020-exception.ut.log \
	$(O)test.tmp/unittest/0030-enum.ut.log \
	$(O)test.tmp/unittest/0050-vector.ut.log \
	$(O)test.tmp/unittest/0080-map.ut.log \
	$(O)test.tmp/unittest/0170-ostream-print.ut.log \
	$(O)test.tmp/unittest/0190-str_ostream.ut.log \
	$(O)test.tmp/unittest/0250-file_ostream.py.log \
	$(O)test.tmp/unittest/0280-file_istream.py.log
#	$(O)test.tmp/unittest/0350-subproc.ut.log


####################################################################################################
# Real targets

# THE libabc.
$(O)lib/libabc$(LIBEXT): \
	$(O)obj/atomic.cxx$(OBJEXT) \
	$(O)obj/bitmanip.cxx$(OBJEXT) \
	$(O)obj/byteorder.cxx$(OBJEXT) \
	$(O)obj/core.cxx$(OBJEXT) \
	$(O)obj/file.cxx$(OBJEXT) \
	$(O)obj/file_iostream.cxx$(OBJEXT) \
	$(O)obj/file_path.cxx$(OBJEXT) \
	$(O)obj/enum.cxx$(OBJEXT) \
	$(O)obj/exception.cxx$(OBJEXT) \
	$(O)obj/iostream.cxx$(OBJEXT) \
	$(O)obj/memory.cxx$(OBJEXT) \
	$(O)obj/module.cxx$(OBJEXT) \
	$(O)obj/str.cxx$(OBJEXT) \
	$(O)obj/str_iostream.cxx$(OBJEXT) \
	$(O)obj/to_str_backend.cxx$(OBJEXT) \
	$(O)obj/text.cxx$(OBJEXT) \
	$(O)obj/trace.cxx$(OBJEXT) \
	$(O)obj/utf_traits.cxx$(OBJEXT) \
	$(O)obj/_vextr.cxx$(OBJEXT)
#	$(O)obj/subproc$(OBJEXT)

# Testing support library.
$(O)lib/libabc-testing$(LIBEXT): \
	$(O)obj/testing/module.cxx$(OBJEXT) \
	$(O)obj/testing/runner.cxx$(OBJEXT) \
	$(O)obj/testing/unit.cxx$(OBJEXT)

# Test suite.
$(O)bin/abc-test$(EXEEXT): \
	$(O)obj/test/abc-test.cxx$(OBJEXT) \
	$(O)obj/test/file_path.cxx$(OBJEXT) \
	$(O)obj/test/str.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT) \
	  $(O)lib/libabc-testing$(LIBEXT)
$(O)bin/abc-test$(EXEEXT): LDLIBS+=-labc-testing

# Test programs.
$(O)bin/unittest/0010-module$(EXEEXT): \
	$(O)obj/unittest/0010-module.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0020-exception$(EXEEXT): \
	$(O)obj/unittest/0020-exception.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0030-enum$(EXEEXT): \
	$(O)obj/unittest/0030-enum.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0050-vector$(EXEEXT): \
	$(O)obj/unittest/0050-vector.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0080-map$(EXEEXT): \
	$(O)obj/unittest/0080-map.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0170-ostream-print$(EXEEXT): \
	$(O)obj/unittest/0170-ostream-print.cxx$(OBJEXT) \
	$(O)obj/mock/iostream.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0190-str_ostream$(EXEEXT): \
	$(O)obj/unittest/0190-str_ostream.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0250-file_ostream$(EXEEXT): \
	$(O)obj/unittest/0250-file_ostream.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
$(O)bin/unittest/0280-file_istream$(EXEEXT): \
	$(O)obj/unittest/0280-file_istream.cxx$(OBJEXT) \
	| $(O)lib/libabc$(LIBEXT)
#$(O)bin/unittest/0350-subproc$(EXEEXT): \
#	$(O)obj/unittest/0350-subproc$(OBJEXT) \
#	| $(O)lib/libabc$(LIBEXT)


# Tweak flags for individual source files.

# Rationale: a lot of switch statements leave the compiler confused about code paths.
$(O)obj/text.cxx$(OBJEXT): CPPFLAGS+=-Wno-uninitialized


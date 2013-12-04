# -*- coding: utf-8; mode: Makefile; tab-width: 8 -*-
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

# Provide a default for installation.
ifeq ($(INSTALL_DIR),)
	INSTALL_DIR:=/usr/local
endif

# Ensure that these end with a backslash.
override INSTALL_DIR:=$(INSTALL_DIR)/
ifneq ($(O),)
	override O:=$(O)/
endif

# File names.
DEPEXT:=.dep
EXEEXT:=
LIBEXT:=.a
OBJEXT:=.o
SOEXT:=.so

# Programs.
CROSS_COMPILE:=
AR:=$(CROSS_COMPILE)ar
CC:=$(CROSS_COMPILE)gcc
CP:=cp
CPP:=$(CROSS_COMPILE)cpp
CXX:=$(CROSS_COMPILE)g++
DEBUGGER:=$(CROSS_COMPILE)gdb
INSTALL:=install
INSTALLDATA:=install -m 644

# Program flags.
CPPFLAGS+= -Wall -Wextra -pedantic -Wundef -Wshadow -Wconversion -Wsign-conversion -Wlogical-op
CPPFLAGS+= -Wmissing-declarations -Wpacked -Wunreachable-code -Winline
# Other interesting GCC cpp flags; enable them once in a while:
#CPPFLAGS+= -Wmissing-noreturn -Wpadded
CFLAGS+= -std=c99 -fvisibility=hidden $(CPPFLAGS)
CXXFLAGS+= -std=c++0x -fnon-call-exceptions -fvisibility=hidden $(CPPFLAGS)
CTUNEFLAGS+= -O2
LDLIBS+=-ldl -lpthread
LDFLAGS+=-Wl,--as-needed
#LDFLAGS+=-Wl,-O
ifdef DEBUG
	CFLAGS+= -ggdb -O0 -DDEBUG=1
	CXXFLAGS+= -ggdb -O0 -DDEBUG=1
	LDFLAGS+= -ggdb
else
	CFLAGS+= $(CTUNEFLAGS)
	CXXFLAGS+= $(CTUNEFLAGS)
	LDFLAGS+= -Wl,-s
endif
ifdef ABC_STL
	CPPFLAGS+= -Iinclude/stl
endif


# Internal variables.

# Set to 1 (later in this file) if make is being invoked to build a “clean” phony target.
CLEAN_PHONY:=0


# These variables are to be defined by the including makefile.

# Top-level directory containing source files; scanned automatically for known source file types.
#srcdir:=
# Sources to be generated
#generatedsources:=


# Eye-candy.

Q:=
CLR_CMD:=
CLR_FAIL:=
CLR_PASS:=
CLR_RST:=
ifndef VERBOSE
	VERBOSE:=0
endif
ifeq ($(VERBOSE),0)
	Q:=@
	ansiesc:=[
	CLR_CMD:=$(ansiesc)1;37m
	CLR_FAIL:=$(ansiesc)1;31m
	CLR_PASS:=$(ansiesc)1;32m
	CLR_RST:=$(ansiesc)0m
endif



####################################################################################################
# Phony targets

.PHONY: all

# Like clean, but may refrain from deleting a few files that people normally don't want to
# recompile.
.PHONY: mostlyclean
ifeq ($(MAKECMDGOALS),mostlyclean)
CLEAN_PHONY:=1
endif
mostlyclean:
	$(Q)rm -rf $(O)bin $(O)lib $(O)$(O)obj $(O)test.tmp

# Deletes all files from the current directory that are normally created by building the program.
.PHONY: clean
ifeq ($(MAKECMDGOALS),clean)
CLEAN_PHONY:=1
endif
clean: mostlyclean
	$(Q)rm -rf $(O)dep

# Deletes almost everything that can be reconstructed with this Makefile.
.PHONY: maintainer-clean
ifeq ($(MAKECMDGOALS),maintainer-clean)
CLEAN_PHONY:=1
endif
maintainer-clean: clean
ifneq ($(generatedsources),)
	$(Q)rm -f $(generatedsources)
endif

# Deletes all files from the current directory that are created by configuring or building the
# program.
.PHONY: distclean
ifeq ($(MAKECMDGOALS),distclean)
CLEAN_PHONY:=1
endif
distclean: maintainer-clean
	$(Q)rm -f Makefile.conf

# Creates a distribution tar file for this program.
# The tar file should be set up so that the file names in the tar file start with a subdirectory
# name which is the name of the package it is a distribution for. This name can include the version
# number.
.PHONY: dist
dist:

# Performs self-tests (if any). The user must build the program before running the tests, but needs
# not install the program.
.PHONY: test
test:
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  TEST  $(CLR_RST) $<"
endif
	$(Q)env LD_LIBRARY_PATH=$(ABC_PATH)/lib:${LD_LIBRARY_PATH} $<

# Performs self-tests (if any). The user must build the program before running the tests, but needs
# not install the program.
.PHONY: check
check:
	$(Q)$(ABC_PATH)tools/makecheck.py --summary $^

.PHONY: install
install:
# Install in $(INSTALL_DIR) using the same relative path from $(O). If a file is executable, install
# it as such.
	@for SRC in $^; do \
		DST=$(INSTALL_DIR)$${SRC#$(O)}; \
		test -x $${SRC} && CMD=$(INSTALL) || CMD=$(INSTALLDATA); \
		if test $(VERBOSE) = 0; then \
			echo "$(CLR_CMD)INSTALL $(CLR_RST) $${DST}"; \
		else \
			echo $(INSTALL) $${SRC} $${DST}; \
		fi; \
		$${CMD} $${SRC} $${DST}; \
	done



####################################################################################################
# Real targets

# Link rule for executables.
$(O)bin/%$(EXEEXT):
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@env $(foreach var, OBJEXT CC CXX CPP VERBOSE CLR_CMD CLR_RST, $(var)="$($(var))") \
		$(ABC_PATH)tools/posix/ld.sh $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Link rule for shared libraries.
$(O)lib/%$(SOEXT): CFLAGS+=-fPIC
$(O)lib/%$(SOEXT): CXXFLAGS+=-fPIC
$(O)lib/%$(SOEXT):
	@test -d $(dir $@) || mkdir -p $(dir $@)
	@env $(foreach var, OBJEXT CC CXX CPP VERBOSE CLR_CMD CLR_RST, $(var)="$($(var))") \
		$(ABC_PATH)tools/posix/ld.sh $(LDFLAGS) -shared -o $@ $^ $(LDLIBS)


# Librarian rule for static libraries.
$(O)lib/%$(LIBEXT):
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  AR    $(CLR_RST) $@"
endif
	$(Q)$(AR) rc $@ $?


# Compiler rule for C sources.
$(O)obj/%.c$(OBJEXT): $(srcdir)/%.c
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  CC    $(CLR_RST) $<"
endif
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

# Compiler rule for C++ sources.
$(O)obj/%.cxx$(OBJEXT): $(srcdir)/%.cxx
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  CXX   $(CLR_RST) $<"
endif
	$(Q)$(CXX) $(CXXFLAGS) -c -o $@ $<


# Preprocessing-only rule for C sources.
$(O)obj/%.c.i: $(srcdir)/%.c
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  CPP   $(CLR_RST) $<"
endif
	$(Q)$(CC) $(CFLAGS) -E -o $@ $<

# Preprocessing-only rule for C++ sources.
$(O)obj/%.cxx.i: $(srcdir)/%.cxx
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  CXXPP $(CLR_RST) $<"
endif
	$(Q)$(CXX) $(CXXFLAGS) -E -o $@ $<


# Dependencies-gathering rule for source files that require dependency analysis.
$(O)dep/%$(DEPEXT): $(srcdir)/%
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifeq ($(VERBOSE),0)
	@echo "$(CLR_CMD)  MKDEP $(CLR_RST) $<"
else
	@echo "$(ABC_PATH)tools/posix/mkdep.sh $< $@"
endif
	@env $(foreach var, CPP CFLAGS CXXFLAGS, $(var)="$($(var))") \
		$(ABC_PATH)tools/posix/mkdep.sh $< $@


# The DEPrule macro will output a -include *.dep statement for each source file we can get a list of
# dependencies for. This will cause the above dependencies-gathering rule to be activated first, and
# the file will then be included. We use - to avoid raising an error if the .dep file is missing;
# GNU make is smart enough to understand it will need to generate it before including it.

# Avoid rebuilding the dependencies files during a `make clean` or similar target.
ifeq ($(CLEAN_PHONY),0)

define DEPrule
-include $$(O)dep/$(srcfile)$$(DEPEXT)
endef

$(foreach srcfile,$(shell \
	find $(srcdir) -type f \( -name '*.c' -o  -name '*.cxx' \) -printf '%P\n' \
),$(eval $(call DEPrule)))

endif #ifeq ($(CLEAN_PHONY),0) …


# Rule to run stand-alone unit tests when make check is invoked.
$(O)test.tmp/%.ut.log: $(O)bin/%
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifneq ($(VERBOSE),0)
	@echo "$(ABC_PATH)tools/makecheck.py --unittest $@ $^"
endif
	@env \
		LD_LIBRARY_PATH=$(ABC_PATH)/lib:${LD_LIBRARY_PATH} \
		$(foreach var, \
			DEBUG DEBUGGER VERBOSE CLR_CMD CLR_PASS CLR_FAIL CLR_RST, \
		$(var)="$($(var))") \
		$(ABC_PATH)tools/makecheck.py --unittest $@ $^


# Rule to run Python script-driven unit tests when make check is invoked.
$(O)test.tmp/%.py.log: $(O)bin/% $(srcdir)/%.py
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifneq ($(VERBOSE),0)
	@echo "$(ABC_PATH)tools/makecheck.py --unittest $@ $^"
endif
	@env \
		LD_LIBRARY_PATH=$(ABC_PATH)/lib:${LD_LIBRARY_PATH} \
		$(foreach var, \
			DEBUG DEBUGGER VERBOSE CLR_CMD CLR_PASS CLR_FAIL CLR_RST, \
		$(var)="$($(var))") \
		$(ABC_PATH)tools/makecheck.py --unittest $@ $^


# Rule to compare C/C++ preprocessed files with the expected outputs when make check is invoked.
# Note: the file name of the output and expected output is the same; only the directories differ.
$(O)test.tmp/%.cxx.i.log: $(O)obj/%.cxx.i $(srcdir)/%.cxx.i
	@test -d $(dir $@) || mkdir -p $(dir $@)
ifneq ($(VERBOSE),0)
	@echo "$(ABC_PATH)tools/makecheck.py --difftest $@ $^"
endif
	@env $(foreach var, \
		DEBUG DEBUGGER VERBOSE CLR_CMD CLR_PASS CLR_FAIL CLR_RST, \
	$(var)="$($(var))") \
		$(ABC_PATH)tools/makecheck.py --difftest $@ $^



####################################################################################################
# Other directives

# More POSIX-compliant behavior.
.POSIX:
# Disable old-fashioned suffix rules.
.SUFFIXES:
# Don’t delete intermediate files if the recipe that caused them to be generated fails.
.SECONDARY:

# Don’t try to pull files from source control systems.
%:: %,v
%:: RCS/%,v
%:: RCS/%
%:: s.%
%:: SCCS/s.%


#!/bin/sh
# -*- coding: utf-8; mode: sh; tab-width: 3; indent-tabs-mode: nil -*-
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

# Generates a list of dependencies (.dep) for a source file.


srcfile="${1}"
depfile="${2}"


# Prints a list of dependencies.
mkdep() {
   case "${1}" in
   (*.c)
      ${CPP} -MM -MG -MT "\$(O)obj/${srcfile##*/}\$(OBJEXT)" ${CFLAGS} "${1}"
      ;;
   (*.cxx|*.cpp|*.cc)
      ${CPP} -MM -MG -MT "\$(O)obj/${srcfile##*/}\$(OBJEXT)" ${CXXFLAGS} "${1}"
      ;;
   (*)
      echo "${0##*/}: error: Unable to determine dependencies for ${srcfile}" >&2
      return 1
      ;;
   esac
}
dep="$(mkdep "${srcfile}")" || exit ${?}

# Make the dependencies a single line, and add the result to the list.
echo "${dep}" |
sed -ne '
   H                      # Append every line to the hold space.
   $ {                    # In the end, ...
      x                   # Move the hold space to the pattern space.
      s/ *\\[\r\n]* */ /g # Properly replace any continuation newlines.
      s/[\r\n]*//g        # Delete any remaining newlines.
      p                   # Print the result.
   }
' >"${depfile}"


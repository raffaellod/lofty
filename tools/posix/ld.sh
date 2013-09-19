#!/bin/sh
# -*- coding: utf-8; mode: sh; tab-width: 3 -*-
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

# Invokes the C or the C++ link driver depending on whether any C++ units are to be linked.


# This will contain the chosen link driver.
ld=
# Keep track of the output for debugging purposes.
output=
# The remaining arguments are the rest of the command line, which includes the object files we want
# to look at.
for arg; do
	if [ "${output}" = -next ]; then
		output="${arg}"
	else
		case "${arg}" in
		(-o)
			output=-next
			;;
		(-*)
			;;
		(*)
			# If a C++ file caused the C++ linker driver to be selected, we won’t override that.
			if [ "${ld}" != "${CXX}" ]; then
				argsrc="${arg%${OBJEXT}}"
				if [ "${argsrc}" != "${arg}" ]; then
					case "${argsrc}" in
					(*.c)
						ld="${CC}"
						;;
					(*.cxx|*.cpp|*.cc)
						ld="${CXX}"
						# A single C++ file forces the linker driver to be the C++ one, even if we have
						# other non-C++ files.
						break
						;;
					esac
				fi
			fi
			;;
		esac
	fi
done
if [ -z "${ld}" ]; then
	echo "${0##*/}: error: Unable to determine which link driver to use${output:+ for ${output}}" >&2
	return 1
fi

# Execute the command.
if [ ${VERBOSE} = 0 ]; then
	echo "${CLR_CMD}  LD    ${CLR_RST} ${output}"
else
	echo "${ld} ${@}"
fi
exec ${ld} "${@}"


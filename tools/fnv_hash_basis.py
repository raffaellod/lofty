#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3 -*-
#---------------------------------------------------------------------------------------------------
# Application-Building Components
# Copyright 2010-2013 Raffaello D. Di Napoli
#---------------------------------------------------------------------------------------------------
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

import sys


def fnv_hash_basis(cBits, iFNVPrime):
	"""Calculates the basis number for the FNV-1a hash algorithm by Fowler/Noll/Vo.

	This is a Python adaption of the de facto standard alhorithm in calc script.
	"""

	# Constants.
	sFNVBasis = 'chongo <Landon Curt Noll> /\\../\\'

	# Calculate the hash.
	iHashMod = 2 ** cBits
	iFNVBasis = 0
	for ch in sFNVBasis:
#		sys.stdout.write('Char {} (basis = {})\n'.format(ch, iFNVBasis))
		iFNVBasis *= iFNVPrime
		iFNVBasis %= iHashMod
		iFNVBasis ^= ord(ch)
	sys.stdout.write('Using prime = {:#x}, {}-bit basis = {}\n'.format(iFNVPrime, cBits, iFNVBasis))


if __name__ == '__main__':
	fnv_hash_basis(int(sys.argv[1], 0), int(sys.argv[2], 0))


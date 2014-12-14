#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3; indent-tabs-mode: nil -*-
#
# Copyright 2010, 2011, 2012, 2013, 2014
# Raffaello D. Di Napoli
#
# This file is part of Abaclade.
#
# Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
# General Public License as published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along with Abaclade. If not, see
# <http://www.gnu.org/licenses/>.
#---------------------------------------------------------------------------------------------------

"""Python adaption of the FNV-1a hash algorithm by Fowler/Noll/Vo, based on the de facto standard
alhorithm in calc script. See  See <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
"""

import sys


# String the hash of which is the FNV basis number.
FNV_BASIS_SOURCE = b'chongo <Landon Curt Noll> /\\../\\'

def fnv_hash_basis(cBits, iFNVPrime):
   """Calculates the basis number for the FNV-1a hash algorithm.

   cBits
      Size of the hash, in bits.
   iFNVPrime
      FNV Prime adequate for hashes of cBits size.
   """

   # Calculate the hash.
   iHashMod = 2 ** cBits
   iFNVBasis = 0
   for ch in FNV_BASIS_SOURCE:
      iFNVBasis *= iFNVPrime
      iFNVBasis %= iHashMod
      iFNVBasis ^= ch
   print('Using prime = {0} ({0:#x}), {1}-bit basis = {2} ({2:#x})'.format(
      iFNVPrime, cBits, iFNVBasis
   ))

if __name__ == '__main__':
   fnv_hash_basis(int(sys.argv[1], 0), int(sys.argv[2], 0))

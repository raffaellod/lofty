#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3; indent-tabs-mode: nil -*-
#
# Copyright 2010-2015 Raffaello D. Di Napoli
#
# This file is part of Abaclade.
#
# Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
# Lesser General Public License as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
# the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
# General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
# not, see <http://www.gnu.org/licenses/>.
#---------------------------------------------------------------------------------------------------

"""Python adaptation of the FNV-1a hash algorithm by Fowler/Noll/Vo, based on the de facto standard
algorithm in calc script. See  See <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
"""

import sys


# String the hash of which is the FNV basis number.
FNV_BASIS_SOURCE = b'chongo <Landon Curt Noll> /\\../\\'

def fnv_hash_basis(cBits, iFNVPrime):
   """Calculates the basis number for the FNV-1a hash algorithm.

   int cBits
      Size of the hash, in bits.
   int iFNVPrime
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

def main(iterArgs):
   """Implementation of __main__.

   iterable(str*) iterArgs
      Command-line arguments.
   int return
      Command return status.
   """

   fnv_hash_basis(int(iterArgs[1], 0), int(iterArgs[2], 0))
   return 0

if __name__ == '__main__':
   sys.exit(main(sys.argv))

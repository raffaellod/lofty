#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3; indent-tabs-mode: nil -*-
#
# Copyright 2010-2017 Raffaello D. Di Napoli
#
# This file is part of Lofty.
#
# Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
# Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
# option) any later version.
#
# Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
# warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
# for more details.
#
# You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
# <http://www.gnu.org/licenses/>.
#-------------------------------------------------------------------------------------------------------------

"""Python adaptation of the FNV-1a hash algorithm by Fowler/Noll/Vo, based on the de facto standard
algorithm in calc script. See  See <http://www.isthe.com/chongo/tech/comp/fnv/> for details.
"""

import sys


####################################################################################################

def fnv_hash_basis(hash_bits, fnv_prime):
   """Calculates the basis number for the FNV-1a hash algorithm.

   int hash_bits
      Size of the hash, in bits.
   int fnv_prime
      FNV Prime adequate for hashes of hash_bits size.
   int return
      Computed FNV basis.
   """

   mask = (1 << hash_bits) - 1
   fnv_basis = 0
   for i in b'chongo <Landon Curt Noll> /\\../\\':
      fnv_basis = ((fnv_basis * fnv_prime) & mask) ^ i
   return fnv_basis

def auto_base_int(s):
   """Converts a string notation of an integer into the corresponding number.

   str s
      String to convert.
   int return
      Converted integer.
   """

   return int(s, 0)

def main(args):
   """Implementation of __main__.

   iterable(str*) args
      Command-line arguments.
   int return
      Command return status.
   """

   import argparse

   # Parse the command line.
   argparser = argparse.ArgumentParser(add_help=False)
   argparser.add_argument(
      '--help', action='help',
      help='Show this informative message and exit.'
   )
   argparser.add_argument(
      'bits', type=int,
      help='Size of the hash, in bits.'
   )
   argparser.add_argument(
      'fnv_prime', metavar='FNV-prime', type=auto_base_int,
      help='FNV Prime adequate for hashes of <bits> size. May be specified in 0x or 0b notation.'
   )
   args = argparser.parse_args()

   fnv_basis = fnv_hash_basis(args.bits, args.fnv_prime)
   print('Using prime = {0} ({0:#x}), {1}-bit basis = {2} ({2:#x})'.format(
      args.fnv_prime, args.bits, fnv_basis
   ))

   return 0

if __name__ == '__main__':
   sys.exit(main(sys.argv))

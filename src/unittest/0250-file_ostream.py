#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3 -*-
#---------------------------------------------------------------------------------------------------
# Application-Building Components
# Copyright 2011-2013 Raffaello D. Di Napoli
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

import os
import subprocess


class unittest(object):

	def __init__(self, runner, sUnitName, sTestBinFilename):
		'''
		Constructor.
		'''

		self.m_runner = runner
		self.m_sUnitName = sUnitName
		self.m_sTestBinFilename = sTestBinFilename


	def run(self):
		'''
		Executes the unit test.
		'''

		runner = self.m_runner
		sTestBinFilename = os.environ['TEST_BIN']

		runner.subtest_begin('file_ostream to stdout (host encoding, file-backed)')
		runner.subtest_createfile('OUTPUT_EXPECTED', 'Testing stdout (host encoding)\n')
		iRet = subprocess.call(
			[sTestBinFilename, '-o'],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_ostream to file (host encoding)')
		runner.subtest_createfile('OUTPUT_EXPECTED', 'Testing file (host encoding)\n')
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_OUTPUT']],
			stdin = None,
			stdout = None
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_ostream to file (UTF-8)')
		runner.subtest_createfile('OUTPUT_EXPECTED',
			'\x54\x65\x73\x74\x69\x6e\x67\x20\x66\x69\x6c\x65\x20\x28\x55\x54\x46\x2d\x38\x20\x65' +
			'\x6e\x63\x6f\x64\x69\x6e\x67\x29\x0a'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_OUTPUT'], '-utf8'],
			stdin = None,
			stdout = None
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_ostream to file (UTF-16BE)')
		runner.subtest_createfile('OUTPUT_EXPECTED',
			'\x00\x54\x00\x65\x00\x73\x00\x74\x00\x69\x00\x6e\x00\x67\x00\x20\x00\x66\x00\x69' +
			'\x00\x6c\x00\x65\x00\x20\x00\x28\x00\x55\x00\x54\x00\x46\x00\x2d\x00\x31\x00\x36' +
			'\x00\x42\x00\x45\x00\x20\x00\x65\x00\x6e\x00\x63\x00\x6f\x00\x64\x00\x69\x00\x6e' +
			'\x00\x67\x00\x29\x00\x0a'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_OUTPUT'], '-utf16be'],
			stdin = None,
			stdout = None
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_ostream to file (UTF-32LE)')
		runner.subtest_createfile('OUTPUT_EXPECTED',
			'\x54\x00\x00\x00\x65\x00\x00\x00\x73\x00\x00\x00\x74\x00\x00\x00\x69\x00\x00\x00' +
			'\x6e\x00\x00\x00\x67\x00\x00\x00\x20\x00\x00\x00\x66\x00\x00\x00\x69\x00\x00\x00' +
			'\x6c\x00\x00\x00\x65\x00\x00\x00\x20\x00\x00\x00\x28\x00\x00\x00\x55\x00\x00\x00' +
			'\x54\x00\x00\x00\x46\x00\x00\x00\x2d\x00\x00\x00\x33\x00\x00\x00\x32\x00\x00\x00' +
			'\x4c\x00\x00\x00\x45\x00\x00\x00\x20\x00\x00\x00\x65\x00\x00\x00\x6e\x00\x00\x00' +
			'\x63\x00\x00\x00\x6f\x00\x00\x00\x64\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00' +
			'\x67\x00\x00\x00\x29\x00\x00\x00\x0a\x00\x00\x00'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_OUTPUT'], '-utf32le'],
			stdin = None,
			stdout = None
		)
		runner.subtest_end(iRet)


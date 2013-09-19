#!/usr/bin/python
# -*- coding: utf-8; mode: python; tab-width: 3 -*-
#
# Copyright 2011, 2012, 2013
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

import os
import subprocess


class unittest(object):

	THREE_LINES = 'Line 1st\nOther line\nLast line\n'

	def __init__(self, runner, sUnitName, sTestBinFilename):
		"""Constructor."""

		self.m_runner = runner
		self.m_sUnitName = sUnitName
		self.m_sTestBinFilename = sTestBinFilename


	def run(self):
		"""Executes the unit test."""

		runner = self.m_runner
		sTestBinFilename = os.environ['TEST_BIN']

		runner.subtest_begin('file_istream from stdin (host encoding)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		iRet = subprocess.call(
			[sTestBinFilename, '-i'],
			stdin = open(os.environ['SUBTEST_OUTPUT_EXPECTED'], 'rb'),
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from file (host encoding)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_OUTPUT_EXPECTED']],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from file (UTF-8)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		runner.subtest_createfile('INPUT',
			'\x4c\x69\x6e\x65\x20\x31\x73\x74\x0a\x4f\x74\x68\x65\x72\x20\x6c\x69\x6e\x65\x0a\x4c' +
			'\x61\x73\x74\x20\x6c\x69\x6e\x65\x0a'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_INPUT']],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from file (UTF-8+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		runner.subtest_createfile('INPUT',
			'\xef\xbb\xbf\x4c\x69\x6e\x65\x20\x31\x73\x74\x0a\x4f\x74\x68\x65\x72\x20\x6c\x69\x6e' +
			'\x65\x0a\x4c\x61\x73\x74\x20\x6c\x69\x6e\x65\x0a'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_INPUT']],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from file (UTF-16BE+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		runner.subtest_createfile('INPUT',
			'\xfe\xff\x00\x4c\x00\x69\x00\x6e\x00\x65\x00\x20\x00\x31\x00\x73\x00\x74\x00\x0a' +
			'\x00\x4f\x00\x74\x00\x68\x00\x65\x00\x72\x00\x20\x00\x6c\x00\x69\x00\x6e\x00\x65' +
			'\x00\x0a\x00\x4c\x00\x61\x00\x73\x00\x74\x00\x20\x00\x6c\x00\x69\x00\x6e\x00\x65' +
			'\x00\x0a'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_INPUT']],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from file (UTF-32LE+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		runner.subtest_createfile('INPUT',
			'\xff\xfe\x00\x00\x4c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
			'\x20\x00\x00\x00\x31\x00\x00\x00\x73\x00\x00\x00\x74\x00\x00\x00\x0a\x00\x00\x00' +
			'\x4f\x00\x00\x00\x74\x00\x00\x00\x68\x00\x00\x00\x65\x00\x00\x00\x72\x00\x00\x00' +
			'\x20\x00\x00\x00\x6c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
			'\x0a\x00\x00\x00\x4c\x00\x00\x00\x61\x00\x00\x00\x73\x00\x00\x00\x74\x00\x00\x00' +
			'\x20\x00\x00\x00\x6c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
			'\x0a\x00\x00\x00'
		)
		iRet = subprocess.call(
			[sTestBinFilename, '-f', os.environ['SUBTEST_INPUT']],
			stdin = None,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		)
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from piped stdin (host encoding)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		with subprocess.Popen(
			[sTestBinFilename, '-i'],
			stdin = subprocess.PIPE,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		) as utb:
			utb.stdin.write(bytearray(self.THREE_LINES, encoding = 'latin_1'))
			utb.communicate()
			iRet = utb.returncode
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from piped stdin (UTF-8)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		with subprocess.Popen(
			[sTestBinFilename, '-i'],
			stdin = subprocess.PIPE,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		) as utb:
			utb.stdin.write(bytearray(
				'\x4c\x69\x6e\x65\x20\x31\x73\x74\x0a\x4f\x74\x68\x65\x72\x20\x6c\x69\x6e\x65\x0a\x4c' +
				'\x61\x73\x74\x20\x6c\x69\x6e\x65\x0a',
				encoding = 'latin_1'
			))
			utb.communicate()
			iRet = utb.returncode
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from piped stdin (UTF-8+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		with subprocess.Popen(
			[sTestBinFilename, '-i'],
			stdin = subprocess.PIPE,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		) as utb:
			utb.stdin.write(bytearray(
				'\xef\xbb\xbf\x4c\x69\x6e\x65\x20\x31\x73\x74\x0a\x4f\x74\x68\x65\x72\x20\x6c\x69\x6e' +
				'\x65\x0a\x4c\x61\x73\x74\x20\x6c\x69\x6e\x65\x0a',
				encoding = 'latin_1'
			))
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from piped stdin (UTF-16BE+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		with subprocess.Popen(
			[sTestBinFilename, '-i'],
			stdin = subprocess.PIPE,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		) as utb:
			utb.stdin.write(bytearray(
				'\xfe\xff\x00\x4c\x00\x69\x00\x6e\x00\x65\x00\x20\x00\x31\x00\x73\x00\x74\x00\x0a' +
				'\x00\x4f\x00\x74\x00\x68\x00\x65\x00\x72\x00\x20\x00\x6c\x00\x69\x00\x6e\x00\x65' +
				'\x00\x0a\x00\x4c\x00\x61\x00\x73\x00\x74\x00\x20\x00\x6c\x00\x69\x00\x6e\x00\x65' +
				'\x00\x0a',
				encoding = 'latin_1'
			))
		runner.subtest_end(iRet)

		runner.subtest_begin('file_istream from piped stdin (UTF-32LE+BOM)')
		runner.subtest_createfile('OUTPUT_EXPECTED', self.THREE_LINES)
		with subprocess.Popen(
			[sTestBinFilename, '-i'],
			stdin = subprocess.PIPE,
			stdout = open(os.environ['SUBTEST_OUTPUT'], 'wb')
		) as utb:
			utb.stdin.write(bytearray(
				'\xff\xfe\x00\x00\x4c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
				'\x20\x00\x00\x00\x31\x00\x00\x00\x73\x00\x00\x00\x74\x00\x00\x00\x0a\x00\x00\x00' +
				'\x4f\x00\x00\x00\x74\x00\x00\x00\x68\x00\x00\x00\x65\x00\x00\x00\x72\x00\x00\x00' +
				'\x20\x00\x00\x00\x6c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
				'\x0a\x00\x00\x00\x4c\x00\x00\x00\x61\x00\x00\x00\x73\x00\x00\x00\x74\x00\x00\x00' +
				'\x20\x00\x00\x00\x6c\x00\x00\x00\x69\x00\x00\x00\x6e\x00\x00\x00\x65\x00\x00\x00' +
				'\x0a\x00\x00\x00',
				encoding = 'latin_1'
			))
		runner.subtest_end(iRet)


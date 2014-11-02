/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_IO_TEXT_FILE_HXX
#define _ABACLADE_IO_TEXT_FILE_HXX

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> before this file
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/os/path.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text globals

namespace abc {
namespace io {
namespace text {

/*! Returns the text writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM std::shared_ptr<binbuf_writer> stderr();

/*! Returns the text reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM std::shared_ptr<binbuf_reader> stdin();

/*! Returns the text writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM std::shared_ptr<binbuf_writer> stdout();

/*! Opens a file for text-mode access.

@param op
   Path to the file.
@param am
   Desired access mode.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text I/O object for the file.
*/
ABACLADE_SYM std::shared_ptr<binbuf_base> open(
   os::path const & op, access_mode am, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode reading.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader for the file.
*/
inline std::shared_ptr<binbuf_reader> open_reader(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::unknown
) {
   return std::dynamic_pointer_cast<binbuf_reader>(open(op, access_mode::read, enc));
}

/*! Opens a file for text-mode writing.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer for the file.
*/
inline std::shared_ptr<binbuf_writer> open_writer(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::utf8
) {
   return std::dynamic_pointer_cast<binbuf_writer>(open(op, access_mode::write, enc));
}

} //namespace text
} //namespace io
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _ABACLADE_IO_TEXT_FILE_HXX


/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
Raffaello D. Di Napoli

This file is part of Application-Building Components (henceforth referred to as ABC).

ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License along with ABC. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABC_IO_TEXT_FILE_HXX
#define _ABC_IO_TEXT_FILE_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/file_path.hxx>
#include <abc/io/binary/buffered.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text globals

namespace abc {
namespace io {
namespace text {

/** Returns the text writer associated to the standard error output file (stderr).

return
   Standard error file.
*/
ABCAPI std::shared_ptr<binbuf_writer> stderr();


/** Returns the text reader associated to the standard input file (stdin).

return
   Standard input file.
*/
ABCAPI std::shared_ptr<binbuf_reader> stdin();


/** Returns the text writer associated to the standard output file (stdout).

return
   Standard output file.
*/
ABCAPI std::shared_ptr<binbuf_writer> stdout();


/** Opens a file for text-mode access.

fp
   Path to the file.
am
   Desired access mode.
return
   Pointer to a text I/O object for the file.
*/
ABCAPI std::shared_ptr<binbuf_base> open(
   file_path const & fp, access_mode am,
   abc::text::encoding enc = abc::text::encoding::unknown,
   abc::text::line_terminator lterm = abc::text::line_terminator::unknown
);


/** Opens a file for text-mode reading.

fp
   Path to the file.
return
   Pointer to a text reader for the file.
*/
inline std::shared_ptr<binbuf_reader> open_reader(
   file_path const & fp,
   abc::text::encoding enc = abc::text::encoding::unknown,
   abc::text::line_terminator lterm = abc::text::line_terminator::unknown
) {
   return std::dynamic_pointer_cast<binbuf_reader>(open(fp, access_mode::read, enc, lterm));
}


/** Opens a file for text-mode writing.

fp
   Path to the file.
return
   Pointer to a text writer for the file.
*/
inline std::shared_ptr<binbuf_writer> open_writer(
   file_path const & fp,
   abc::text::encoding enc = abc::text::encoding::utf8,
   abc::text::line_terminator lterm = abc::text::line_terminator::unknown
) {
   return std::dynamic_pointer_cast<binbuf_writer>(open(fp, access_mode::write, enc, lterm));
}

} //namespace text
} //namespace io
} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef _ABC_IO_TEXT_FILE_HXX


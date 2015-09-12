/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
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

#ifndef _ABACLADE_IO_TEXT_HXX
#define _ABACLADE_IO_TEXT_HXX

#ifndef _ABACLADE_HXX
   #error "Please #include <abaclade.hxx> before this file"
#endif
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abaclade/io.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////

// Forward declarations.

namespace abc { namespace io { namespace binary {

class reader;
class writer;

}}} //namespace abc::io::binary

namespace abc { namespace io { namespace text {

class reader;
class writer;
class binbuf_base;
class binbuf_reader;
class binbuf_writer;

//! Text writer associated to the standard error output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stderr;
//! Text reader associated to the standard input file.
extern ABACLADE_SYM _std::shared_ptr<reader> stdin;
//! Text writer associated to the standard output file.
extern ABACLADE_SYM _std::shared_ptr<writer> stdout;

/*! Creates and returns a text reader for the specified binary reader.

@param pbr
   Pointer to a binary reader.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader operating on top of the specified binary reader.
*/
ABACLADE_SYM _std::shared_ptr<reader> make_reader(
   _std::shared_ptr<binary::reader> pbr, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Creates and returns a text writer for the specified binary writer.

@param pbw
   Pointer to a binary writer.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer operating on top of the specified binary writer.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_writer(
   _std::shared_ptr<binary::writer> pbw, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode reading.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_reader> open_reader(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode writing.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer for the file.
*/
ABACLADE_SYM _std::shared_ptr<binbuf_writer> open_writer(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::utf8
);

}}} //namespace abc::io::text

namespace abc { namespace io { namespace text { namespace detail {

/*! Creates and returns a text writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stderr();

/*! Creates and returns a text reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM _std::shared_ptr<reader> make_stdin();

/*! Creates and returns a text writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM _std::shared_ptr<writer> make_stdout();

}}}} //namespace abc::io::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <abaclade/io/text/binbuf.hxx>

#endif //ifndef _ABACLADE_IO_TEXT_HXX

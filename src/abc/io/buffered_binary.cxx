/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014
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

#include <abc/core.hxx>
#include <abc/io/buffered_binary.hxx>
#include <abc/numeric.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
   #include <unistd.h> // *_FILENO ssize_t close() isatty() open() read() write()
   #include <fcntl.h> // O_*
   #include <sys/stat.h> // S_*, stat()
// #include <sys/mman.h> // mmap(), munmap(), PROT_*, MAP_*
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io globals


namespace abc {

namespace io {

std::shared_ptr<binary_base> buffer_binary(std::shared_ptr<binary_base> pbb) {
   ABC_TRACE_FN((/*pbb*/));

   if (std::shared_ptr<binary_reader> pbr = std::dynamic_pointer_cast<binary_reader>(pbb)) {
      return std::make_shared<default_buffered_binary_reader>(std::move(pbr));
   }
   if (std::shared_ptr<binary_writer> pbw = std::dynamic_pointer_cast<binary_writer>(pbb)) {
      return std::make_shared<default_buffered_binary_writer>(std::move(pbw));
   }
   // TODO: use a better exception class.
   ABC_THROW(argument_error, ());
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_reader


namespace abc {

namespace io {

default_buffered_binary_reader::default_buffered_binary_reader(std::shared_ptr<binary_reader> pbr) :
   m_pbr(std::move(pbr)) {
}


/*virtual*/ default_buffered_binary_reader::~default_buffered_binary_reader() {
}


/*virtual*/ size_t default_buffered_binary_reader::read(void * p, size_t cbMax) {
   ABC_TRACE_FN((this, p, cbMax));

   return m_pbr->read(p, cbMax);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::default_buffered_binary_writer


namespace abc {

namespace io {

default_buffered_binary_writer::default_buffered_binary_writer(std::shared_ptr<binary_writer> pbw) :
   m_pbw(std::move(pbw)) {
}


/*virtual*/ default_buffered_binary_writer::~default_buffered_binary_writer() {
}


/*virtual*/ void default_buffered_binary_writer::flush() {
   ABC_TRACE_FN((this));

   m_pbw->flush();
}


/*virtual*/ size_t default_buffered_binary_writer::write(void const * p, size_t cb) {
   ABC_TRACE_FN((this, p, cb));

   return m_pbw->write(p, cb);
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


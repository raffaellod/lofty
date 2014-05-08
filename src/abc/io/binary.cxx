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

#include <abc/core.hxx>
#include <abc/io/binary.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_base


namespace abc {

namespace io {

binary_base::binary_base() {
}


/*virtual*/ binary_base::~binary_base() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_reader


namespace abc {

namespace io {

binary_reader::binary_reader() :
   binary_base() {
}


/*virtual*/ binary_reader::~binary_reader() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::binary_writer


namespace abc {

namespace io {

binary_writer::binary_writer() :
   binary_base() {
}


/*virtual*/ binary_writer::~binary_writer() {
}

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


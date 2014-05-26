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

#include <abc.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_vector_to_str_backend


namespace abc {

_vector_to_str_backend::_vector_to_str_backend(istr const & sFormat) :
   m_sSeparator(SL(", ")) {
   ABC_UNUSED_ARG(sFormat);
}


_vector_to_str_backend::~_vector_to_str_backend() {
}


void _vector_to_str_backend::_write_end(io::text::writer * ptwOut) {
   ABC_TRACE_FN((this, ptwOut));

   m_tsbStr.write(istr(SL("}")), ptwOut);
}


void _vector_to_str_backend::_write_separator(io::text::writer * ptwOut) {
   ABC_TRACE_FN((this, ptwOut));

   m_tsbStr.write(m_sSeparator, ptwOut);
}


void _vector_to_str_backend::_write_start(io::text::writer * ptwOut) {
   ABC_TRACE_FN((this, ptwOut));

   m_tsbStr.write(istr(SL("{")), ptwOut);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2011, 2012, 2013
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

#include <abc/module.hxx>
#include <abc/str_iostream.hxx>
#include <abc/trace.hxx>
using namespace abc;



class test_module :
	public module_impl<test_module> {
public:

	int main(vector<istr const> const & vsArgs) {
		abc_trace_fn((/*vsArgs*/));

		UNUSED_ARG(vsArgs);

		str_ostream sos;
		istr sEnc(SL("host"));

		sos.write(istr8(U8SL("Testing string (UTF-8 encoding)")));
#ifdef U16SL
		sos.write(istr16(U16SL("Testing string (UTF-16 encoding)")));
#endif
#ifdef U32SL
		sos.write(istr32(U32SL("Testing string (UTF-32 encoding)")));
#endif		

		return EXIT_SUCCESS;
	}
};

ABC_DECLARE_MODULE_IMPL_CLASS(test_module)


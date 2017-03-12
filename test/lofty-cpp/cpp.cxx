/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015, 2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>

LOFTY_TEST_CMP_BEGIN

#define ACTUALLY_EMPTY
#define EMPTY_ONLY_IF_CALLED()
#define NOT_QUITE_EMPTY() x
#define ZERO 0

LOFTY_CPP_NOT(0)
LOFTY_CPP_NOT(1)
LOFTY_CPP_NOT(ZERO)
LOFTY_CPP_NOT(a)

LOFTY_CPP_IF(0, a, b)
LOFTY_CPP_IF(1, a, b)
LOFTY_CPP_IF(ZERO, (a), (b))
LOFTY_CPP_IF(a, (a), (b))

LOFTY_CPP_IS_EMPTY()
LOFTY_CPP_IS_EMPTY(/**/)
LOFTY_CPP_IS_EMPTY(ACTUALLY_EMPTY)
LOFTY_CPP_IS_EMPTY(EMPTY_ONLY_IF_CALLED)
LOFTY_CPP_IS_EMPTY(EMPTY_ONLY_IF_CALLED())
LOFTY_CPP_IS_EMPTY(NOT_QUITE_EMPTY)
LOFTY_CPP_IS_EMPTY(NOT_QUITE_EMPTY())
LOFTY_CPP_IS_EMPTY(a)
LOFTY_CPP_IS_EMPTY((a))
LOFTY_CPP_IS_EMPTY(a, b)
LOFTY_CPP_IS_EMPTY((a, b))
LOFTY_CPP_IS_EMPTY((a), (b))
LOFTY_CPP_IS_EMPTY(a, b, c)
LOFTY_CPP_IS_EMPTY((a, b, c))
LOFTY_CPP_IS_EMPTY((a), (b), (c))

LOFTY_CPP_LIST_COUNT()
LOFTY_CPP_LIST_COUNT(/**/)
LOFTY_CPP_LIST_COUNT(ACTUALLY_EMPTY)
LOFTY_CPP_LIST_COUNT(EMPTY_ONLY_IF_CALLED)
LOFTY_CPP_LIST_COUNT(EMPTY_ONLY_IF_CALLED())
LOFTY_CPP_LIST_COUNT(NOT_QUITE_EMPTY)
LOFTY_CPP_LIST_COUNT(NOT_QUITE_EMPTY())
LOFTY_CPP_LIST_COUNT(a)
LOFTY_CPP_LIST_COUNT((a))
LOFTY_CPP_LIST_COUNT((a, 1))
LOFTY_CPP_LIST_COUNT(a, b)
LOFTY_CPP_LIST_COUNT((a, 1), (b, 2))
LOFTY_CPP_LIST_COUNT(a, b, c)
LOFTY_CPP_LIST_COUNT((a, b, c))
LOFTY_CPP_LIST_COUNT((a, 1), (b, 2), (c, 3))

#define SCALAR_WALKER(x) x
LOFTY_CPP_LIST_WALK(SCALAR_WALKER, a)
LOFTY_CPP_LIST_WALK(SCALAR_WALKER, a, b, c, d)

#define TUPLE_WALKER(x, y) x = y,
LOFTY_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1))
LOFTY_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1), (b, 2), (c, 3), (d, 4))

LOFTY_TEST_CMP_END

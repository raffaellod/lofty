/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2011-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#include <abaclade.hxx>

ABCMK_CMP_BEGIN

ABC_CPP_IF(0)(a, b)
ABC_CPP_IF(1)(a, b)
ABC_CPP_IF(0)((a), (b))
ABC_CPP_IF(1)((a), (b))

#define ACTUALLY_EMPTY
#define EMPTY_ONLY_IF_CALLED()
#define NOT_QUITE_EMPTY() x

ABC_CPP_IS_EMPTY()
ABC_CPP_IS_EMPTY(/**/)
ABC_CPP_IS_EMPTY(ACTUALLY_EMPTY)
ABC_CPP_IS_EMPTY(EMPTY_ONLY_IF_CALLED)
ABC_CPP_IS_EMPTY(EMPTY_ONLY_IF_CALLED())
ABC_CPP_IS_EMPTY(NOT_QUITE_EMPTY)
ABC_CPP_IS_EMPTY(NOT_QUITE_EMPTY())
ABC_CPP_IS_EMPTY(a)
ABC_CPP_IS_EMPTY((a))
ABC_CPP_IS_EMPTY(a, b)
ABC_CPP_IS_EMPTY((a, b))
ABC_CPP_IS_EMPTY((a), (b))
ABC_CPP_IS_EMPTY(a, b, c)
ABC_CPP_IS_EMPTY((a, b, c))
ABC_CPP_IS_EMPTY((a), (b), (c))

ABC_CPP_LIST_COUNT()
ABC_CPP_LIST_COUNT(/**/)
ABC_CPP_LIST_COUNT(ACTUALLY_EMPTY)
ABC_CPP_LIST_COUNT(EMPTY_ONLY_IF_CALLED)
ABC_CPP_LIST_COUNT(EMPTY_ONLY_IF_CALLED())
ABC_CPP_LIST_COUNT(NOT_QUITE_EMPTY)
ABC_CPP_LIST_COUNT(NOT_QUITE_EMPTY())
ABC_CPP_LIST_COUNT(a)
ABC_CPP_LIST_COUNT((a))
ABC_CPP_LIST_COUNT((a, 1))
ABC_CPP_LIST_COUNT(a, b)
ABC_CPP_LIST_COUNT((a, 1), (b, 2))
ABC_CPP_LIST_COUNT(a, b, c)
ABC_CPP_LIST_COUNT((a, b, c))
ABC_CPP_LIST_COUNT((a, 1), (b, 2), (c, 3))

#define SCALAR_WALKER(x) x
ABC_CPP_LIST_WALK(SCALAR_WALKER, a)
ABC_CPP_LIST_WALK(SCALAR_WALKER, a, b, c, d)

#define TUPLE_WALKER(x, y) x = y,
ABC_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1))
ABC_CPP_TUPLELIST_WALK(TUPLE_WALKER, (a, 1), (b, 2), (c, 3), (d, 4))

ABCMK_CMP_END

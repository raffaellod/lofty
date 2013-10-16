﻿/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2013
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

#ifndef ABC_TESTING_UNIT_HXX
#define ABC_TESTING_UNIT_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif
#include <abc/testing/module.hxx>
#include <memory>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit


namespace abc {

namespace testing {

/** Base class for unit tests.
*/
class unit {
public:

	/** Constructor.
	*/
	unit();


	/** Destructor.
	*/
	virtual ~unit();


	/** Initializes the object. Split into a method separated from the constructor so that derived
	classes don’t need to declare a constructor just to forward its arguments.

	pmod
		Pointer to the testing module.
	*/
	void init(module * pmod);


	/** Executes the unit test.
	*/
	virtual void run();


protected:

	/** Validates an assertion.

	bExpr
		Result of the assertion expression.
	pszExpr
		Failed assertion.
	*/
	void assert(bool bExpr, char const * pszExpr);


	/** Validates an expectation.

	bExpr
		Result of the expectation expression.
	pszExpr
		Failed assertion.
	*/
	void expect(bool bExpr, char const * pszExpr);
};

} //namespace testing

} //namespace abc


/** Asserts that the specified expression evaluates as non-false (true); throws an exception if the
assertion fails.

expr
	Expression that should evaulate to non-false (true).
*/
#define ABC_TESTING_ASSERT(expr) \
	this->assert(!!(expr), #expr)


/** Verifies that the specified expression evaluates as non-false (true).

expr
	Expression that should evaulate to non-false (true).
*/
#define ABC_TESTING_EXPECT(expr) \
	this->expect(!!(expr), #expr)


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit_factory_impl


namespace abc {

namespace testing {

/** Registers a abc::testing::unit-derived class with abc::testing::module, providing the module
with a callback enabling it to create an instance of the unit.
*/
class unit_factory_impl {
protected:

	/** Factory function, returning an abc::testing::unit instance. */
	typedef std::unique_ptr<unit> (* factory_fn)(module * pmod);
	/** Linked list item. */
	struct factory_list_item {
		factory_list_item * pfliNext;
		factory_fn pfnFactory;
	};


public:

	/** Constructor.

	pfli
		Pointer to the derived class’s factory list item.
	*/
	unit_factory_impl(factory_list_item * pfli) {
		pfli->pfliNext = sm_pfliHead;
		sm_pfliHead = pfli;
	}


private:

	/** Pointer to the head of the list of factory function. */
	static factory_list_item * sm_pfliHead;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit_factory


namespace abc {

namespace testing {

/** Template version of abc::testing::unit_factory_impl, able to instantiate classes derived from
abc::testing::unit-derived.
*/
template <class T>
class unit_factory :
	public unit_factory_impl {
public:

	/** Constructor.
	*/
	unit_factory() :
		unit_factory_impl(&sm_fli) {
	}


	/** Class factory for T.

	pmod
		Module to provide to the unit.
	*/
	static std::unique_ptr<unit> factory(module * pmod) {
		std::unique_ptr<T> pt(new T());
		pt->init(pmod);
		return std::move(pt);
	}


private:

	/** Entry in the list of factory functions for this class. */
	static factory_list_item sm_fli;
};


/** Registers an abc::testing::unit-derived class with the testing module.

cls
	Unit class.
*/
#define ABC_TESTING_UNIT_REGISTER(cls) \
	namespace abc { \
	namespace testing { \
	namespace { \
	\
	unit_factory<cls> uf; \
	template <> \
	/*static*/ unit_factory_impl::factory_list_item unit_factory<cls>::sm_fli = { \
		NULL, \
		unit_factory<cls>::factory \
	}; \
	\
	} /*namespace*/ \
	} /*namespace testing*/ \
	} /*namespace abc*/

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_TESTING_UNIT_HXX


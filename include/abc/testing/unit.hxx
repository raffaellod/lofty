/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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
#include <abc/testing/runner.hxx>



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

	prunner
		Pointer to the test runner.
	*/
	void init(runner * prunner);


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
	void assert(bool bExpr, istr const & sExpr);


	/** Validates an expectation.

	bExpr
		Result of the expectation expression.
	pszExpr
		Failed assertion.
	*/
	void expect(bool bExpr, istr const & sExpr);


protected:

	/** Runner executing this test. */
	runner * m_prunner;
};

} //namespace testing

} //namespace abc


/** Asserts that the specified expression evaluates as non-false (true); throws an exception if the
assertion fails.

expr
	Expression that should evaulate to non-false (true).
*/
#define ABC_TESTING_ASSERT(expr) \
	this->assert(!!(expr), SL(#expr))


/** Verifies that the specified expression evaluates as non-false (true).

expr
	Expression that should evaulate to non-false (true).
*/
#define ABC_TESTING_EXPECT(expr) \
	this->expect(!!(expr), SL(#expr))


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit_factory_impl


namespace abc {

namespace testing {

/** Maintains a list of abc::testing::unit-derived classes that can be used by an
abc::testing::runner instance to instantiate and execute each unit.
*/
class unit_factory_impl {
public:

	/** Factory function, returning an abc::testing::unit instance. */
	typedef std::unique_ptr<unit> (* factory_fn)(runner * prunner);
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


	/** Returns a pointer to the head of the list of factory functions, which the caller can then use
	to walk the entire list (ending when an item’s next pointer is NULL).

	return
		Pointer to the head of the list.
	*/
	static factory_list_item * get_factory_list_head() {
		return sm_pfliHead;
	}


private:

	/** Pointer to the head of the list of factory functions. */
	static factory_list_item * sm_pfliHead;
};

} //namespace testing

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::testing::unit_factory


namespace abc {

namespace testing {

/** Template version of abc::testing::unit_factory_impl, able to instantiate classes derived from
abc::testing::unit.
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

	prunner
		Runner to provide to the unit.
	*/
	static std::unique_ptr<unit> factory(runner * prunner) {
		std::unique_ptr<T> pt(new T());
		pt->init(prunner);
		return std::move(pt);
	}


private:

	/** Entry in the list of factory functions for this class. */
	static factory_list_item sm_fli;
};


/** Registers an abc::testing::unit-derived class for execution by an abc::testing::runner instance.

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


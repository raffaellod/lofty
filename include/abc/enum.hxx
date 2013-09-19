/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

Copyright 2010, 2011, 2012, 2013
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

#ifndef ABC_ENUM_HXX
#define ABC_ENUM_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/char.hxx>
#include <abc/cppmacros.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations


namespace abc {

// Forward declaration from abc/iostream.hxx.
class ostream;

// Forward declaration from abc/to_string_backend.hxx.
template <typename T>
class to_string_backend;

/// Enumeration member (name/value pair).
struct enum_member;

/// Implementation of the specializations of to_string_backend for enum_impl specializations.
class _enum_to_string_backend_impl;

/// Implementation of enumeration classes.
template <class T>
class enum_impl;


/// TODO: comment + document design.
//
// Loosely based on <http://www.python.org/dev/peps/pep-0435/>
//
// TODO: allow specifying a default value (instead of having __default = max + 1).
// TODO: support for bit-field enumerations? Allow logical operation, smart conversion to/from
// string, etc.
#define ABC_ENUM(name, ...) \
	struct ABC_CPP_CAT(_, name, _e) { \
	\
		/** Publicly-accessible enumerated constants. */ \
		enum enum_type { \
			ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER, __VA_ARGS__) \
			__default = 0 \
		}; \
	\
	\
		/** Returns a pointer to the name/value map to be used by abc::enum_impl. */ \
		static ::abc::enum_member const * _get_map() { \
			static ::abc::enum_member const sc_map[] = { \
				ABC_CPP_TUPLELIST_WALK(_ABC_ENUM_MEMBER_ARRAY_ITEM, __VA_ARGS__) \
				{ NULL, __default } \
			}; \
			return sc_map; \
		} \
	}; \
	typedef ::abc::enum_impl<ABC_CPP_CAT(_, name, _e)> name

/// Expands into an enum name/value assignment.
#define _ABC_ENUM_MEMBER(name, value) \
			name = value,

/// Expands into an abc::enum_member initializer.
#define _ABC_ENUM_MEMBER_ARRAY_ITEM(name, value) \
				{ ABC_CPP_TOSTRING(name), value },

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_member


namespace abc {

struct enum_member {

	/// Name.
	char_t const * pszName;
	/// Value.
	int iValue;


	/// Finds and returns the member associated to the specified enumerated value or name.
	static enum_member const * find_in_map(enum_member const * pem, int i);
	static enum_member const * find_in_map(enum_member const * pem, char_t const * psz);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_enum_to_string_backend_impl


namespace abc {

class _enum_to_string_backend_impl {
public:

	/// Constructor.
	_enum_to_string_backend_impl(char_range const & crFormat);


protected:

	/// See to_string_backend::write().
	void write_impl(int i, enum_member const * pem, ostream * posOut);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::enum_impl


namespace abc {

template <class T>
class enum_impl :
	public T {
public:

	typedef typename T::enum_type enum_type;

	/// Constructor.
	//
	enum_impl() :
		m_e(enum_type::__default) {
	}
	enum_impl(enum_type e) :
		m_e(e) {
	}
	enum_impl(enum_impl const & e) :
		m_e(e.m_e) {
	}
	// Conversion from integer.
	explicit enum_impl(int i) :
		m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), i)->iValue)) {
	}
	// Conversion from string.
	explicit enum_impl(char_t const * psz) :
		m_e(static_cast<enum_type>(enum_member::find_in_map(T::_get_map(), psz)->iValue)) {
	}


	/// Assignment operator.
	//
	enum_impl & operator=(enum_impl const & e) {
		m_e = e.m_e;
		return *this;
	}
	enum_impl & operator=(enum_type e) {
		m_e = e;
		return *this;
	}


	/// Returns the current base enumerated value.
	//
	enum_type base() const {
		return m_e;
	}


	/// Returns the name of the current enumerated value.
	//
	char_t const * name() const {
		return _member()->pszName;
	}


protected:

	/// Returns a pointer to the name/value pair for the current value.
	//
	enum_member const * _member() const {
		return enum_member::find_in_map(T::_get_map(), m_e);
	}


private:

	/// Enumerated value.
	enum_type m_e;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	template <class T> \
	inline bool operator op(abc::enum_impl<T> const & e1, abc::enum_impl<T> const & e2) { \
		return e1.base() op e2.base(); \
	} \
	template <class T> \
	inline bool operator op(abc::enum_impl<T> const & e1, typename T::enum_type e2) { \
		return e1.base() op e2; \
	} \
	template <class T> \
	inline bool operator op(typename T::enum_type e1, abc::enum_impl<T> const & e2) { \
		return e1 op e2.base(); \
	}
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL


namespace abc {

// Specialization of to_string_backend.
template <class T>
class to_string_backend<enum_impl<T>> :
	public _enum_to_string_backend_impl {
public:

	/// Constructor.
	//
	to_string_backend(char_range const & crFormat = char_range()) :
		_enum_to_string_backend_impl(crFormat) {
	}


	/// See to_string_backend::write().
	//
	void write(enum_impl<T> e, ostream * posOut) {
		_enum_to_string_backend_impl::write_impl(e.base(), e._get_map(), posOut);
	}
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_ENUM_HXX


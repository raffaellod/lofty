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

#ifndef ABC_FILE_PATH_HXX
#define ABC_FILE_PATH_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/string.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

/// DOC:7101 abc::file_path
//
// File paths are always stored in absolute notation, prepending the current directory on assignment
// if necessary.
//
// Under Win32, all DOS-style paths (e.g. “C:\My\File”) are normalized to the Win32 namespace, which
// means they all start with “\\?\” (automatically prepended, forming e.g. “\\?\C:\My\File”). This
// prefix is also considered the root, although trying to do anything with it other than
// concatenating more path components will most likely result in exceptions being thrown.
// Nonetheless, this convention allows to have a single root in Win32 just like under POSIX.
//
// Reference for Python’s implementation: <http://docs.python.org/3/library/os.path.html>
// Reference for Win32: <http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx>

/// Filesystem path.
class file_path;

// Enumerates directory entries.
#if 0
class _file_path_iterator;
#endif

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_path


namespace abc {

class file_path :
	public support_explicit_operator_bool<file_path> {

#if 0
	friend class _file_path_iterator;
#endif

public:

	/// Constructor.
	//
	file_path() {
	}
	file_path(file_path const & fp) :
		m_s(fp.m_s) {
	}
	file_path(file_path && fp) :
		m_s(std::move(fp.m_s)) {
	}
	file_path(cstring const & s) :
		m_s(normalize(s)) {
	}
	file_path(wstring && s) :
		m_s(normalize(std::move(s))) {
	}
	file_path(wdstring && s) :
		m_s(normalize(std::move(s))) {
	}


	/// Assignment operator.
	//
	file_path & operator=(file_path const & fp) {
		m_s = fp.m_s;
		return *this;
	}
	file_path & operator=(file_path && fp) {
		m_s = std::move(fp.m_s);
		return *this;
	}
	file_path & operator=(cstring const & s) {
		m_s = normalize(s);
		return *this;
	}
	file_path & operator=(wstring && s) {
		m_s = normalize(std::move(s));
		return *this;
	}


	/// Returns true if the length is greater than 0.
	//
	explicit_operator_bool() const {
		return m_s.get_size() > 0;
	}


	/// Automatic cast to string.
	//
	operator cstring const &() const {
		return m_s;
	}


	/// Concatenation-assignment operator.
	//
	file_path & operator+=(cstring const & s) {
		m_s = normalize(m_s + s);
		return *this;
	}


	/// Concatenation operator.
	//
	file_path operator+(cstring const & s) const {
		return file_path(*this) += s;
	}


	/// Path-correct concatenation-assignment operator. Joins the current path with the provided
	// string, inserting a separator if necessary.
	file_path & operator/=(cstring const & s);


	/// Path-correct concatenation operator. See operator/=() for details.
	//
	file_path operator/(cstring const & s) const {
		return file_path(*this) /= s;
	}


	/// Support for relational operators.
	//
	int compare_to(cstring const & s) const {
		return m_s.compare_to(s);
	}


	/// Returns a read-only pointer to the path string. See wdstring::get_data().
	//
	char_t const * get_data() const {
		return m_s.get_data();
	}


	/// Returns the count of characters in the path.
	//
	size_t get_size() const {
		return m_s.get_size();
	}


	/// Returns the last component in this path.
	wdstring get_base_name() const;

	/// Returns the current directory.
	static file_path get_current_dir();

	/// Returns the directory containing this file.
	file_path get_parent_dir() const;

	/// Returns the root (POSIX) or the namespace root (Windows).
	static file_path get_root();


	/// Returns the platform-dependent path component separator.
	//
	static cstring get_separator() {
		return cstring(smc_aszSeparator);
	}


#if 0
	// Returns an iterator over entries in the path matching the specified
	// pattern.
	_file_path_iterator find(string const & sPattern) const;
#endif

	/// Returns true if the specified string represents an absolute path.
	static bool is_absolute(cstring const & s);

	/// Returns true if this path represents a directory.
	bool is_dir() const;

	/// Returns true if this->get_parent_dir() == *this.
	bool is_root() const;


private:

	/// Normalizes (and validates) a path:
	// - converts every mixed slash sequence to a single separator;
	// - interprets . and .. special components;
	// - removes any trailing separators.
	static wdstring normalize(wdstring s);


private:

	/// Full file path, always in normalized form.
	wdstring m_s;
	/// Platform-specific path component separator.
	static char_t const smc_aszSeparator[1 + 1 /*NUL*/];
	/// Platform-specific root path.
	static char_t const smc_aszRoot[];
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
	inline bool operator op(abc::file_path const & fp1, abc::file_path const & fp2) { \
		return fp1.compare_to(fp2) op 0; \
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
template <>
class to_string_backend<file_path> :
	public to_string_backend<cstring> {
public:

	/// Constructor.
	to_string_backend(char_range const & crFormat = char_range());

	/// Writes the path, applying the specified format.
	void write(file_path const & fp, ostream * posOut);
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::file_path>  {

	size_t operator()(abc::file_path const & fp) const {
		return std::hash<abc::cstring>()(static_cast<abc::cstring const &>(fp));
	}
};

} //namespace std


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::_file_path_iterator


namespace abc {

#if 0
class _file_path_iterator {
public:

	// Constructor.
	//
	_file_path_iterator(file_path const & pathDir, string const & sPattern) :
		m_pathBaseDir(pathDir),
		m_hSearch(find_first_file((m_pathBaseDir / sPattern).get_data(), &m_wfd)),
		m_bEOF(m_hSearch == INVALID_HANDLE_VALUE) {
		if (!m_bEOF)
			m_pathCurr = next_file_path();
	}
	_file_path_iterator(_file_path_iterator && iter) throw() :
		m_pathBaseDir(std::move(iter.m_pathBaseDir)),
		m_hSearch(iter.m_hSearch),
		m_bEOF(iter.m_bEOF) {
		mem_copy(&m_wfd, 1, &iter.m_wfd);
		iter.m_hSearch = INVALID_HANDLE_VALUE;
	}


	// Destructor.
	//
	~_file_path_iterator() throw() {
		if (m_hSearch != INVALID_HANDLE_VALUE)
			::FindClose(m_hSearch);
	}


	// Assignment operator.
	//
	_file_path_iterator & operator=(_file_path_iterator && iter) throw();


	// Dereferencing operator.
	//
	file_path & operator*() throw() {

		return m_pathCurr;

	}
	file_path const & operator*() const throw() {
		return m_pathCurr;
	}


	// Dereferencing member access operator.
	//
	file_path & operator->() throw() {
		return m_pathCurr;
	}
	file_path const & operator->() const throw() {
		return m_pathCurr;
	}


	// Prefix increment operator.
	//
	_file_path_iterator & operator++() {
		if (::FindNextFileW(m_hSearch, &m_wfd))
			m_pathCurr = next_file_path();
		else {
			unsigned long iErr(::GetLastError());
			if (iErr != ERROR_NO_MORE_FILES)
				throw_os_error(iErr);
			// Remember we hit the bottom.
			m_bEOF = true;
		}
		return *this;
	}


	// Returns true if there are still files to be enumerated.
	//
	operator bool() const throw() {
		return !m_bEOF;
	}


private:

	// Wrapper for ::FindFirstFile(), to support RIIA.
	//
	static HANDLE find_first_file(
		char_t const * pszPattern, WIN32_FIND_DATA * pwfd
	) {
		HANDLE h(::FindFirstFileW(pszPattern, pwfd));
		if (h == INVALID_HANDLE_VALUE) {

			unsigned long iErr(::GetLastError());
			if (iErr != ERROR_FILE_NOT_FOUND)
				throw_os_error(iErr);
		}
		return h;
	}


	// Returns the path from the m_wfd member.
	//
	file_path next_file_path() const {
		return file_path(
			m_pathBaseDir / string(
				m_wfd.cFileName, ::wcslen(m_wfd.cFileName)
			),
			m_wfd.dwFileAttributes
		);
	}


private:

	// Directory being enumerated.
	file_path m_pathBaseDir;
	// Search data.
	WIN32_FIND_DATA m_wfd;
	// Fake handle to the search.
	HANDLE m_hSearch;
	// true if we run out of files.
	bool m_bEOF;
	// Current item.
	file_path m_pathCurr;
};


// Now this can be implemented.
inline _file_path_iterator file_path::find(string const & sPattern) const {
	return _file_path_iterator(*this, sPattern);
}
#endif

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_PATH_HXX


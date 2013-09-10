// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#include <abc/file_path.hxx>
#include <abc/trace.hxx>
#if ABC_HOST_API_POSIX
	#include <errno.h> // errno E*
	#include <sys/stat.h> // S_*, stat()
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_path


namespace abc {

#if ABC_HOST_API_POSIX

/// Wrapper for a stat structure that self-loads with information on the file.
//
class file_stat :
	public ::stat {
public:

	/// Constructor.
	//
	file_stat(file_path const & fp) {
		if (::stat(fp.get_data(), this)) {
			throw_os_error();
		}
	}
};

#elif ABC_HOST_API_WIN32

/// Checks whether the file has the specified attribute(s) set.
//
static bool file_attrs(DWORD fi) const {
	DWORD fiAttrs(::GetFileAttributes(get_data()));
	if (fiAttrs == INVALID_FILE_ATTRIBUTES) {
		throw_os_error();
	}
	return (fiAttrs & fi) == fi;
}

#endif



char_t const file_path::smc_aszSeparator[] =
#if ABC_HOST_API_POSIX
	SL("/");
#elif ABC_HOST_API_WIN32
	SL("\\");
#else
	#error TODO-PORT: HOST_API
#endif
char_t const file_path::smc_aszRoot[] =
#if ABC_HOST_API_POSIX
	SL("/");
#elif ABC_HOST_API_WIN32
	SL("\\\\?\\");
#else
	#error TODO-PORT: HOST_API
#endif



file_path & file_path::operator/=(cstring const & s) {
	abc_trace_fn((this, s));

	// Only the root already ends in a separator; everything else needs one.
	m_s = normalize((!m_s || is_root() ? wdstring(m_s) : m_s + smc_aszSeparator[0]) + s);
	return *this;
}


wdstring file_path::get_base_name() const {
	abc_trace_fn((this));

	// An empty path has no base name.
	if (!m_s || is_root()) {
		return m_s;
	}
	wdstring::const_iterator it(m_s.find_last(char32_t(smc_aszSeparator[0])));
	// it != NULL always, because this is not the root.
	assert(it);
	return m_s.substr(it + 1 /*smc_aszSeparator*/);
}


/*static*/ file_path file_path::get_current_dir() {
	abc_trace_fn(());

	wdstring s;
#if ABC_HOST_API_POSIX
	s.grow_for([] (char_t * pch, size_t cchMax) -> size_t {
		if (::getcwd(pch, cchMax)) {
			// The length will be necessarily less than cchMax, so grow_for() will stop.
			return text::utf_traits<>::str_len(pch);
		}
		if (errno != ERANGE) {
			throw_os_error(errno);
		}
		// Report that the provided buffer was too small.
		return cchMax;
	});
#elif ABC_HOST_API_WIN32
	s.grow_for([] (char_t * pch, size_t cchMax) -> size_t {
		DWORD cch(::GetCurrentDirectory(cchMax, pch));
		if (!cch) {
			throw_os_error();
		}
		return cch;
	});
#else
	#error TODO-PORT: HOST_API
#endif
	return std::move(s);
}


file_path file_path::get_parent_dir() const {
	abc_trace_fn((this));

	// An empty path has no parent directory.
	if (!m_s || is_root()) {
		// The root is its own parent.
		return file_path(m_s);
	}
	wdstring::const_iterator it(m_s.find_last(char32_t(smc_aszSeparator[0])));
#if ABC_HOST_API_POSIX
	if (it == m_s.cbegin() + 0 /*"/"*/) {
		// The parent is the root, so keep the slash or we’ll end up with an empty string.
		++it;
	}
#elif ABC_HOST_API_WIN32
	if (it == m_s.cbegin() + 6 /*"\\?\C:\"*/) {
		// The parent is a volume root, so keep the slash or we’ll end up with a volume designator.
		++it;
	}
#else
	#error TODO-PORT: HOST_API
#endif
	return m_s.substr(0, it - m_s.cbegin());
}


// This can’t be in the header file, because the size of smc_aszRoot is only known here.
/*static*/ file_path file_path::get_root() {
	abc_trace_fn(());

	return wdstring(smc_aszRoot);
}


/*static*/ bool file_path::is_absolute(cstring const & s) {
	abc_trace_fn((s));

#if ABC_HOST_API_POSIX
	return s.get_size() >= 1 /*"/"*/ && s[0] == '/';
#elif ABC_HOST_API_WIN32
	size_t cch(s.get_size());
	char_t const * pch(s.get_data());
	// Win32 namespace root: best case.
	if (cch >= 4 /*"\\?\"*/ && pch[0] == '\\' && pch[1] == '\\' && pch[2] == '?' && pch[3] == '\\') {
		return true;
	}
	// DOS-style root, starting with a volume designator.
	if (cch >= 2 /*"X:"*/ && pch[1] == ':') {
		return true;
	}
	return false;
#else
	#error TODO-PORT: HOST_API
#endif
}


bool file_path::is_dir() const {
	abc_trace_fn((this));

#if ABC_HOST_API_POSIX
	return S_ISDIR(file_stat(*this).st_mode);
#elif ABC_HOST_API_WIN32
	return file_attrs(FILE_ATTRIBUTE_DIRECTORY);
#else
	#error TODO-PORT: HOST_API
#endif
}


bool file_path::is_root() const {
	abc_trace_fn((this));

#if ABC_HOST_API_POSIX
	return m_s.get_size() == 1 /*"/"*/;
#elif ABC_HOST_API_WIN32
	return m_s.get_size() == 7 /*"\\?\C:\"*/;
#else
	#error TODO-PORT: HOST_API
#endif
}


/*static*/ wdstring file_path::normalize(wdstring s) {
	abc_trace_fn((s));

	size_t cch(s.get_size());
	// An empty string is okay.
	if (!cch) {
		return std::move(s);
	}
	// If it’s a relative path, make it absolute.
	if (!is_absolute(s)) {
		wdstring sAbs(std::move(get_current_dir().m_s));
		sAbs += smc_aszSeparator[0];
		sAbs += s;
		s = std::move(sAbs);
		cch = s.get_size();
	}
	// Check for the correct root format, and save the index of its separator.
	char_t const * pch0(s.get_data());
	char_t const * pchRootSep(pch0);
#if ABC_HOST_API_POSIX
	// Nothing else to do.
#elif ABC_HOST_API_WIN32
	if (pch0[0] != '\\') {
		// The path is not in “\\?\X:\path” format; make it so.
		s = smc_aszRoot + s;
		cch += countof(smc_aszRoot);
	}
	// Check the root format.
	if (
		cch < 7 /*"\\?\X:\"*/ ||
		pch0[0] != '\\' || pch0[1] != '\\' || pch0[2] != '?' ||
		pch0[3] != '\\' || pch0[5] != ':'  || pch0[6] != '\\'
	) {
		throw_os_error(ERROR_BAD_PATHNAME);
	}
	{
		// Check and normalize the volume designator.
		char_t ch(pch0[4]);
		if (ch >= 'a' && ch <= 'z') {
			ch -= 'a' - 'A';
			s[4] = ch;
		} else if (ch < 'A' || ch > 'Z') {
			throw_os_error(ERROR_INVALID_DRIVE);
		}
	}
	// Point to the last of the separators checked for above.
	pchRootSep += 6;
#else
	#error TODO-PORT: HOST_API
#endif
	// Collapse sequences of separators, normalize separators, and interpret . and .. components.

	// Skip any character up to the root separator; the separator itself is included to activare the
	// logic that parses dots and slashes.
	char_t * pchDst(const_cast<char_t *>(pchRootSep));
	bool bFoundSep(false);
	size_t cDots(0);
	for (char_t const * pchSrc(pchRootSep), * pchMax(pch0 + cch); pchSrc < pchMax; ++pchSrc) {
		char_t ch(*pchSrc);
		if (ch == '.' && cDots < 2) {
			// Count for “.” and “..”.
			++cDots;
		} else if (ch == '\\' || ch == '/') {
			if (!bFoundSep) {
				// No preceding separators: track this as the first one.
				bFoundSep = true;
			} else if (!cDots) {
				// No dots between this separator and the previous one: skip this repetition.
				continue;
			} else {
				// We found “/./” or “/../”: discard the first separator and the dots.
				pchDst -= cDots /*"." or ".."*/ + 1 /*'/'*/;
				// If we found “/../”, go back two separators, and discard anything in between if the
				// previous separator is not the root separator (in which case that’d be enough).
				if (cDots > 1 && pchDst > pchRootSep) {
					// Find the previous separator, skipping the one we just got back to.
					wdstring::const_iterator itOneUp(s.find_last(
						char32_t(smc_aszSeparator[0]),
						wdstring::const_iterator(pchDst - 1 /*'/'*/)
					));
					// Resume writing from the separator we just found.
					pchDst = const_cast<char_t *>(itOneUp.base());
				}
			}
			// Overwrite with the correct path separator.
			ch = smc_aszSeparator[0];
			cDots = 0;
		} else {
			bFoundSep = false;
			cDots = 0;
		}
		*pchDst++ = ch;
	}
	// If the last characters written were “.” or “..”, undo writing them.
	pchDst -= cDots;
	// If we found “/../”, go back two separators and discard anything in between if the previous
	// separator is not the root separator (in which case that’d be enough).
	if (cDots > 1 && pchDst > pchRootSep) {
		// Find the previous separator, skipping the one we just got back to.
		wdstring::const_iterator itOneUp(
			s.find_last(char32_t(smc_aszSeparator[0]), wdstring::const_iterator(pchDst - 1 /*'/'*/))
		);
		// Resume writing from the separator we just found.
		pchDst = const_cast<char_t *>(itOneUp.base());
	}
	// Also undo writing a trailing non-root separator.
	char_t * pchLast(pchDst - 1);
	if (pchLast > pchRootSep && *pchLast == smc_aszSeparator[0]) {
		pchDst = pchLast;
	}

	// Adjust the length based on the position of the last character written.
	s.set_size(size_t(pchDst - s.get_data()));
	return std::move(s);
}


to_string_backend<file_path>::to_string_backend(char_range const & crFormat /*= char_range()*/) {
	auto it(crFormat.cbegin());

	// TODO: parse the format string.

	// If we still have any characters, they are garbage.
	if (it != crFormat.cend()) {
		abc_throw(syntax_error(
			SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
		));
	}
}


void to_string_backend<file_path>::write(file_path const & fp, ostream * posOut) {
	// TODO: apply format options.
	*posOut << static_cast<cstring const &>(fp);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::os::path

namespace abc {
namespace os {

/*! DOC:7101 abc::os::path

An abc::os::path instance is always either an empty path string ("") or a path that is not
necessarily normalized or absolute, but has no incorrect or redundant path separators; e.g. an
abc::os::path instance will never contain “/a//b///c”, and under Win32 it will never be “C:/a” or
“a\\\b/c”.

Under Win32, all absolute DOS-style paths (e.g. “C:\My\File”) are normalized to the Win32 File
Namespace, which means they all start with “\\?\”, forming e.g. “\\?\C:\My\File”. This prefix is
also considered the root, although trying to do anything with it other than concatenating more path
components will most likely result in exceptions being thrown. Nonetheless, this convention allows
to have a single root under Win32 just like under POSIX.

abc::os::path instances can be used with the OS’s file API by using abc::os::path::os_str(), which
will return a string suitable for such use. Under Win32, this will make the path absolute in order
to be able to use the Win32 File Namespace, which has advantages and drawbacks; os_str() mitigates
the drawbacks, allowing to overcome limitations in the Win32 file API. See abc::os::path::os_str()
for more information.

Reference for Python’s approach: “os.path — Common pathname manipulations” <http://docs.python.org/
3/library/os.path.html>
Reference for Win32: “Naming Files, Paths, and Namespaces” <http://msdn.microsoft.com/en-us/library/
windows/desktop/aa365247.aspx>
*/

// Enumerates directory entries.
#if 0
class _path_iterator;
#endif

//! Filesystem path.
class ABACLADE_SYM path : public support_explicit_operator_bool<path> {

#if 0
   friend class _path_iterator;
#endif

public:
   /*! Constructor.

   @param op
      Source path.
   @param s
      Source string.
   */
   path() {
   }
   path(path const & op) :
      m_s(op.m_s) {
   }
   path(path && op) :
      m_s(std::move(op.m_s)) {
   }
   path(dmstr s) :
      m_s(validate_and_adjust(std::move(s))) {
   }

   /*! Assignment operator.

   @param op
      Source path.
   @param s
      Source string.
   @return
      *this.
   */
   path & operator=(path const & op) {
      m_s = op.m_s;
      return *this;
   }
   path & operator=(path && op) {
      m_s = std::move(op.m_s);
      return *this;
   }
   path & operator=(dmstr s) {
      m_s = validate_and_adjust(std::move(s));
      return *this;
   }

   /*! Returns true if the path length is greater than 0.

   @return
      true if the length of the path string is greater than 0, or false otherwise.
   */
   ABC_EXPLICIT_OPERATOR_BOOL() const {
      return bool(m_s);
   }

   /*! Automatic cast to string.

   @return
      An immutable, constant reference to the internal path string.
   */
   operator istr const &() const {
      return m_s;
   }

   /*! Concatenation-assignment operator.

   @param s
      String to append.
   @return
      *this.
   */
   path & operator+=(istr const & s) {
      m_s = validate_and_adjust(m_s + s);
      return *this;
   }

   /*! Concatenation operator.

   @param s
      String to append.
   @return
      Resulting path.
   */
   path operator+(istr const & s) const {
      return path(*this) += s;
   }

   /*! Path-correct concatenation-assignment operator. Joins the current path with the provided
   string, inserting a separator if necessary.

   @param s
      Path component(s) to append.
   @return
      *this.
   */
   path & operator/=(istr const & s);

   /*! Path-correct concatenation operator. See operator/=() for details.

   @param s
      Path component(s) to append.
   @return
      Resulting path.
   */
   path operator/(istr const & s) const {
      return path(*this) /= s;
   }

   /*! Returns the absolute and normalized version of the path. If the path is not already absolute,
   it will be assumed to be relative to abc::os::path::current_dir(). Under Win32 there’s a
   current directory for each volume, so the base directory will be different depending on whether
   the path includes a volume designator and on which volume it identifies.

   @return
      Absolute, normalized path.
   */
   path absolute() const;

   /*! Returns the base name of (last component in) the path.

   @return
      Last component in the path.
   */
   path base_name() const;

   /*! Returns the current working directory (${PWD} in POSIX, %CD% in Windows).

   @return
      Current directory.
   */
   static path current_dir();

#if ABC_HOST_API_WIN32
   /*! Returns the current directory for the specified volume.

   @param chVolume
      Volume designator.
   @return
      Current directory in chVolume.
   */
   static path current_dir_for_volume(char_t chVolume);
#endif //if ABC_HOST_API_WIN32

#if 0
   /*! Returns an iterator over entries in the path matching the specified pattern.

   TODO: comment signature.
   */
   _path_iterator find(istr const & sPattern) const;
#endif

   /*! Returns true if the path is in absolute form. Under Win32, this means that the path is
   prefixed with “\\?\”, e.g. “\\?\C:\my\path”.

   @return
      true if the path is absolute, or false otherwise.
   */
   bool is_absolute() const {
      return is_absolute(m_s);
   }

   /*! Returns true if the path represents a directory.

   @return
      true if the path represents a directory, of false otherwise.
   */
   bool is_dir() const;

   /*! Returns true if the path is absolute and this->parent_dir() == *this.

   @return
      true if the path represents a root directory, of false otherwise.
   */
   bool is_root() const {
      return get_root_length(m_s, false) == m_s.size_in_chars();
   }

   /*! Returns a normalized version of the path by interpreting sequences such as “.” and “..”. The
   resulting replacements may lead to a different path if the original path includes symbolic links.

   @return
      Normalized path.
   */
   path normalize() const;

   /*! Returns a string representation of the path suitable to use with the OS’s file API.

   Under Win32, this returns the absolute (and normalized) version of the path, in order to overcome
   both the MAX_PATH limitation by using the Win32 File Namespace prefix, as well as parsing the
   path in a way that Windows won’t do for Win32 File Namespace-prefixed paths, as documented in
   “Naming Files, Paths, and Namespaces” <http://msdn.microsoft.com/en-us/library/windows/desktop/
   aa365247.aspx>:

      “[The Win32 File Namespace prefix and its UNC sub-namespace] indicate that the path should be
      passed to the system with minimal modification, which means that you cannot use forward
      slashes to represent path separators, or a period to represent the current directory, or
      double dots to represent the parent directory.”

   @return
      String representation of the path suitable for use with the OS’s file API.
   */
#if ABC_HOST_API_POSIX
   // Under POSIX we don’t need an intermediate string, so the return type can be istr const &.
   istr const & os_str() const {
      return m_s;
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   istr os_str() const;
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

   /*! Returns the directory containing the path.

   @return
      Parent directory of the path.
   */
   path parent_dir() const;

   /*! Returns the root (POSIX) or the Win32 File Namespace root (Win32).

   @return
      Root directory.
   */
   static path root();

   /*! Returns the platform-dependent path component separator.

   @return
      Path component separator.
   */
   static istr separator() {
      return istr(smc_aszSeparator);
   }

   /*! Returns the count of characters in the path.

   @return
      Count of characters.
   */
   std::size_t size() const {
      return m_s.size();
   }

private:
   /*! Locates the first character of the final component in the path, e.g. “a” in “a” (all), “a” in
   “/a”, “/b/a” (POSIX), “\\?\UNC\a”, “\\?\UNC\b\a”, “\\?\X:\a”, “\\?\X:\b\a”, “\a”, “\b\a”, “X:a”
   “X:b\a” (Win32).

   @return
      Iterator pointing to the first character of the final component in m_s, or the beginning of
      m_s if the path does not contain a root component/prefix.
   */
   dmstr::const_iterator base_name_start() const;

   /*! Returns the length of the root part of the specified path or, in other words, the index of
   the first character in the path that is not part of the root, e.g. “a” in “/a” (POSIX),
   “\\?\UNC\a”, “\\?\X:\a”, “\a”, “X:a” (Win32); the last two only if bIncludeNonAbsolute because
   they represent relative paths in Win32: “\a” is relative to the current directory’s volume
   designator, “X:a” is relative to the current directory for volume designator X.

   @param s
      Path to parse. Must comply with the rules set for abc::os::path’s internal representation.
   @param bIncludeNonRoot
      If true, non-absolute prefixes such as “\” and ”X:” (Win32) will be considered root prefixes;
      if false, they won’t.
   @return
      Length of the root part in s, or 0 if s does not start with a root part.
   */
   static std::size_t get_root_length(istr const & s, bool bIncludeNonAbsolute);

   /*! Returns true if the specified string represents an absolute path. Under Win32, this means
   that the path needs to be prefixed with “\\?\”, e.g. “\\?\C:\my\path”; a path starting with a
   volume designator (e.g. “C:\my\path”) is not considered absolute, as far as abc::os::path is
   concerned (and it will never be stored as-is in m_s either).

   @param s
      Path to parse. Must comply with the rules set for abc::os::path’s internal representation.
   @return
      true if s represents an absolute path, or false otherwise.
   */
   static bool is_absolute(istr const & s);

   /*! Validates and adjusts a path to make it suitable as abc::os::path’s internal representation:
   •  Collapses sequences of consecutive path separators into a single separator;
   •  Removes any trailing separators;
   •  (Win32 only) Replaces forward slashes with backslashes;
   •  (Win32 only) Prefixes absolute paths (e.g. “C:\my\path”) with the Win32 File Namespace prefix
      (e.g. “\\?\C:\my\path”).

   @param s
      Path to parse.
   @return
      Path suitable for abc::os::path’s internal representation.
   */
   static dmstr validate_and_adjust(dmstr s);

private:
   //! Full path, always in normalized form.
   dmstr m_s;
   //! Platform-specific path component separator.
   static char_t const smc_aszSeparator[1 /*"/" or "\"*/ + 1 /*NUL*/];
   //! Platform-specific root path.
   static char_t const smc_aszRoot[
#if ABC_HOST_API_POSIX
      1 /*"/"*/ + 1 /*NUL*/
#elif ABC_HOST_API_WIN32
      4 /*"\\?\"*/ + 1 /*NUL*/
#else
   #error "TODO: HOST_API"
#endif
   ];
#if ABC_HOST_API_WIN32
   //! Root for UNC paths in the Win32 File Namespace.
   static char_t const smc_aszUNCRoot[8 /*"\\?\UNC\"*/ + 1 /*NUL*/];
#endif
};

// Relational operators.
#define ABC_RELOP_IMPL(op) \
   inline bool operator op(path const & op1, path const & op2) { \
      return static_cast<istr const &>(op1) op static_cast<istr const &>(op2); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
ABC_RELOP_IMPL(>)
ABC_RELOP_IMPL(>=)
ABC_RELOP_IMPL(<)
ABC_RELOP_IMPL(<=)
#undef ABC_RELOP_IMPL

} //namespace os
} //namespace abc

namespace std {

// Specialization of std::hash for abc::os::path.
template <>
struct hash<abc::os::path> : public hash<abc::text::istr> {
   //! See std::hash::operator()().
   std::size_t operator()(abc::os::path const & op) const {
      return hash<abc::text::istr>::operator()(op);
   }
};

} //namespace std

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::to_str_backend ‒ specialization for abc::os::path

namespace abc {

template <>
class ABACLADE_SYM to_str_backend<os::path> : public to_str_backend<istr> {
public:
   /*! Changes the output format.

   @param sFormat
      Formatting options.
   */
   void set_format(istr const & sFormat);

   /*! Writes a string, applying the formatting options.

   @param op
      Path to write.
   @param ptwOut
      Pointer to the writer to output to.
   */
   void write(os::path const & op, io::text::writer * ptwOut);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::os::_path_iterator

namespace abc {
namespace os {

#if 0
class _path_iterator {
public:
   //! Constructor.
   _path_iterator(path const & pathDir, istr const & sPattern) :
      m_pathBaseDir(pathDir),
      m_hSearch(find_first_file((m_pathBaseDir / sPattern).os_str().c_str(), &m_wfd)),
      m_bEOF(m_hSearch == INVALID_HANDLE_VALUE) {
      if (!m_bEOF) {
         m_pathCurr = next_path();
      }
   }
   _path_iterator(_path_iterator && iter) :
      m_pathBaseDir(std::move(iter.m_pathBaseDir)),
      m_hSearch(iter.m_hSearch),
      m_bEOF(iter.m_bEOF) {
      memory::copy(&m_wfd, &iter.m_wfd);
      iter.m_hSearch = INVALID_HANDLE_VALUE;
   }

   //* Destructor.
   ~_path_iterator() {
      if (m_hSearch != INVALID_HANDLE_VALUE) {
         ::FindClose(m_hSearch);
      }
   }

   //* Assignment operator.
   _path_iterator & operator=(_path_iterator && iter);

   //* Dereferencing operator.
   path & operator*() {
      return m_pathCurr;
   }
   path const & operator*() const {
      return m_pathCurr;
   }

   //* Dereferencing member access operator.
   path & operator->() {
      return m_pathCurr;
   }
   path const & operator->() const {
      return m_pathCurr;
   }

   //* Prefix increment operator.
   _path_iterator & operator++() {
      if (::FindNextFileW(m_hSearch, &m_wfd)) {
         m_pathCurr = next_path();
      } else {
         DWORD iErr = ::GetLastError();
         if (iErr != ERROR_NO_MORE_FILES) {
            exception::throw_os_error(iErr);
         }
         // Remember we hit the bottom.
         m_bEOF = true;
      }
      return *this;
   }

   //* Returns true if there are still files to be enumerated.
   operator bool() const {
      return !m_bEOF;
   }

private:
   //* Wrapper for ::FindFirstFile(), to support RIIA.
   static HANDLE find_first_file(char_t const * pszPattern, WIN32_FIND_DATA * pwfd) {
      HANDLE h = ::FindFirstFileW(pszPattern, pwfd);
      if (h == INVALID_HANDLE_VALUE) {
         DWORD iErr = ::GetLastError();
         if (iErr != ERROR_FILE_NOT_FOUND) {
            exception::throw_os_error(iErr);
         }
      }
      return h;
   }

   //* Returns the path from the m_wfd member.
   path next_path() const {
      return path(
         m_pathBaseDir / istr(m_wfd.cFileName, ::wcslen(m_wfd.cFileName)),
         m_wfd.dwFileAttributes
      );
   }

private:
   //* Directory being enumerated.
   path m_pathBaseDir;
   //* Search data.
   WIN32_FIND_DATA m_wfd;
   //* Fake handle to the search.
   HANDLE m_hSearch;
   //* true if we run out of files.
   bool m_bEOF;
   //* Current item.
   path m_pathCurr;
};


// Now this can be defined.

inline _path_iterator path::find(istr const & sPattern) const {
   return _path_iterator(*this, sPattern);
}
#endif

} //namespace os
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_not_found_error

namespace abc {

//! A file could not be found.
class ABACLADE_SYM file_not_found_error :
   public virtual environment_error,
   public exception::extended_info {
public:
   /*! Constructor.

   @param x
      Source error.
   */
   file_not_found_error();
   file_not_found_error(file_not_found_error const & x);

   //! Assignment operator. See abc::environment_error::operator=().
   file_not_found_error & operator=(file_not_found_error const & x);

   /*! Returns the path that couldn’t be found.

   @return
      Path that couldn’t be found at the moment it was accessed.
   */
   os::path const & path() const {
      return m_opNotFound;
   }

   /*! See abc::environment_error::init().

   @param opNotFound
      Path that couldn’t be found.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(abc::os::path const & opNotFound, errint_t err = 0);

protected:
   //! See exception::extended_info::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Path that caused the error.
   os::path m_opNotFound;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

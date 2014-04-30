/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

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

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/str.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_path


namespace abc {

/** DOC:7101 abc::file_path

An abc::file_path instance is always either an empty path string ("") or a path that is not
necessarily normalized or absolute, but has no incorrect or redundant path separators; e.g. an
abc::file_path instance will never contain “/a//b///c”, and under Win32 it will never be “C:/a” or
“a\\\b/c”.

Under Win32, all absolute DOS-style paths (e.g. “C:\My\File”) are normalized to the Win32 File
Namespace, which means they all start with “\\?\”, forming e.g. “\\?\C:\My\File”. This prefix is
also considered the root, although trying to do anything with it other than concatenating more path
components will most likely result in exceptions being thrown. Nonetheless, this convention allows
to have a single root under Win32 just like under POSIX.

abc::file_path instances can be used with the OS’s file API by using abc::file_path::os_str(), which
will return a string suitable for such use. Under Win32, this will make the path absolute in order
to be able to use the Win32 File Namespace, which has advantages and drawbacks; os_str() mitigates
the drawbacks, allowing to overcome limitations in the Win32 file API. See abc::file_path::os_str()
for more information.

Reference for Python’s approach: “os.path — Common pathname manipulations” <http://docs.python.org/
3/library/os.path.html>
Reference for Win32: “Naming Files, Paths, and Namespaces” <http://msdn.microsoft.com/en-us/library/
windows/desktop/aa365247.aspx>
*/

// Enumerates directory entries.
#if 0
class _file_path_iterator;
#endif

/** Filesystem path.
*/
class ABCAPI file_path :
   public support_explicit_operator_bool<file_path> {

#if 0
   friend class _file_path_iterator;
#endif

public:

   /** Constructor.

   fp
      Source file path.
   s
      Source string.
   return
      *this.
   */
   file_path() {
   }
   file_path(file_path const & fp) :
      m_s(fp.m_s) {
   }
   file_path(file_path && fp) :
      m_s(std::move(fp.m_s)) {
   }
   file_path(istr const & s) :
      m_s(validate_and_adjust(s)) {
   }
   file_path(mstr && s) :
      m_s(validate_and_adjust(std::move(s))) {
   }


   /** Assignment operator.

   fp
      Source file path.
   s
      Source string.
   return
      *this.
   */
   file_path & operator=(file_path const & fp) {
      m_s = fp.m_s;
      return *this;
   }
   file_path & operator=(file_path && fp) {
      m_s = std::move(fp.m_s);
      return *this;
   }
   file_path & operator=(istr const & s) {
      m_s = validate_and_adjust(s);
      return *this;
   }
   file_path & operator=(mstr && s) {
      m_s = validate_and_adjust(std::move(s));
      return *this;
   }


   /** Returns true if the path length is greater than 0.

   return
      true if the length of the path string is greater than 0, or false otherwise.
   */
   explicit_operator_bool() const {
      return m_s.size() > 0;
   }


   /** Automatic cast to string.

   return
      An immutable, constant reference to the internal path string.
   */
   operator istr const &() const {
      return m_s;
   }


   /** Concatenation-assignment operator.

   s
      String to append.
   return
      *this.
   */
   file_path & operator+=(istr const & s) {
      m_s = validate_and_adjust(m_s + s);
      return *this;
   }


   /** Concatenation operator.

   s
      String to append.
   return
      Resulting path.
   */
   file_path operator+(istr const & s) const {
      return file_path(*this) += s;
   }


   /** Path-correct concatenation-assignment operator. Joins the current path with the provided
   string, inserting a separator if necessary.

   s
      Path component(s) to append.
   return
      *this.
   */
   file_path & operator/=(istr const & s);


   /** Path-correct concatenation operator. See operator/=() for details.

   s
      Path component(s) to append.
   return
      Resulting path.
   */
   file_path operator/(istr const & s) const {
      return file_path(*this) /= s;
   }


   /** Returns the absolute and normalized version of the path. If the path is not already absolute,
   it will be assumed to be relative to abc::file_path::current_dir(). Under Win32 there’s a
   current directory for each volume, so the base directory will be different depending on whether
   the path includes a volume designator and on which volume it identifies.

   return
      Absolute, normalized path.
   */
   file_path absolute() const;


   /** Returns the base name of (last component in) the path.

   return
      Last component in the path.
   */
   file_path base_name() const;


   /** Support for relational operators.

   s
      String to compare to.
   return
      Standard comparison result integer:
      •  > 0 if *this > s;
      •    0 if *this == s;
      •  < 0 if *this < s.
   */
   int compare_to(istr const & s) const {
      return m_s.compare_to(s);
   }


   /** Returns the current working directory (${PWD} in POSIX, %CD% in Windows).

   return
      Current directory.
   */
   static file_path current_dir();


#if ABC_HOST_API_WIN32

   /** Returns the current directory for the specified volume.

   chVolume
      Volume designator.
   return
      Current directory in chVolume.
   */
   static file_path current_dir_for_volume(char_t chVolume);

#endif //if ABC_HOST_API_WIN32


#if 0
   /** Returns an iterator over entries in the path matching the specified pattern.

   TODO: comment signature.
   */
   _file_path_iterator find(istr const & sPattern) const;
#endif


   /** Returns true if the path is in absolute form. Under Win32, this means that the path is
   prefixed with “\\?\”, e.g. “\\?\C:\my\path”.

   return
      true if the path is absolute, or false otherwise.
   */
   bool is_absolute() const {
      return is_absolute(m_s);
   }


   /** Returns true if the path represents a directory.

   return
      true if the path represents a directory, of false otherwise.
   */
   bool is_dir() const;


   /** Returns true if the path is absolute and this->parent_dir() == *this.

   return
      true if the path represents a root directory, of false otherwise.
   */
   bool is_root() const {
      return get_root_length(m_s, false) == m_s.size();
   }


   /** Returns a normalized version of the path by interpreting sequences such as “.” and “..”. The
   resulting replacements may lead to a different path if the original path includes symbolic links.

   return
      Normalized path.
   */
   file_path normalize() const;


   /** Returns a string representation of the path suitable to use with the OS’s file API.

   Under Win32, this returns the absolute (and normalized) version of the path, in order to overcome
   both the MAX_PATH limitation by using the Win32 File Namespace prefix, as well as parsing the
   path in a way that Windows won’t do for Win32 File Namespace-prefixed paths, as documented in
   “Naming Files, Paths, and Namespaces” <http://msdn.microsoft.com/en-us/library/windows/desktop/
   aa365247.aspx>:

      “[The Win32 File Namespace prefix and its UNC sub-namespace] indicate that the path should be
      passed to the system with minimal modification, which means that you cannot use forward
      slashes to represent path separators, or a period to represent the current directory, or
      double dots to represent the parent directory.”

   return
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
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


   /** Returns the directory containing the path.

   return
      Parent directory of the path.
   */
   file_path parent_dir() const;


   /** Returns the root (POSIX) or the Win32 File Namespace root (Win32).

   return
      Root directory.
   */
   static file_path root();


   /** Returns the platform-dependent path component separator.

   return
      Path component separator.
   */
   static istr separator() {
      return istr(smc_aszSeparator);
   }


   /** Returns the count of characters in the path.

   return
      Count of characters.
   */
   size_t size() const {
      return m_s.size();
   }


private:

   /** Locates the first character of the final component in the path, e.g. “a” in “a” (all), “a” in
   “/a”, “/b/a” (POSIX), “\\?\UNC\a”, “\\?\UNC\b\a”, “\\?\X:\a”, “\\?\X:\b\a”, “\a”, “\b\a”, “X:a”
   “X:b\a” (Win32).

   return
      Iterator pointing to the first character of the final component in m_s, or the beginning of
      m_s if the path does not contain a root component/prefix.
   */
   dmstr::const_iterator base_name_start() const;


   /** Returns the length of the root part of the specified path or, in other words, the index of
   the first character in the path that is not part of the root, e.g. “a” in “/a” (POSIX),
   “\\?\UNC\a”, “\\?\X:\a”, “\a”, “X:a” (Win32); the last two only if bIncludeNonAbsolute because
   they represent relative paths in Win32: “\a” is relative to the current directory’s volume
   designator, “X:a” is relative to the current directory for volume designator X.

   s
      Path to parse. Must comply with the rules set for abc::file_path’s internal representation.
   bIncludeNonRoot
      If true, non-absolute prefixes such as “\” and ”X:” (Win32) will be considered root prefixes;
      if false, they won’t.
   return
      Length of the root part in s, or 0 if s does not start with a root part.
   */
   static size_t get_root_length(dmstr const & s, bool bIncludeNonAbsolute);


   /** Returns true if the specified string represents an absolute path. Under Win32, this means
   that the path needs to be prefixed with “\\?\”, e.g. “\\?\C:\my\path”; a path starting with a
   volume designator (e.g. “C:\my\path”) is not considered absolute, as far as abc::file_path is
   concerned (and it will never be stored as-is in m_s either).

   s
      Path to parse. Must comply with the rules set for abc::file_path’s internal representation.
   return
      true if s represents an absolute path, or false otherwise.
   */
   static bool is_absolute(istr const & s);


   /** Validates and adjusts a path to make it suitable as abc::file_path’s internal representation:
   •  Collapses sequences of consecutive path separators into a single separator;
   •  Removes any trailing separators;
   •  (Win32 only) Replaces forward slashes with backslashes;
   •  (Win32 only) Prefixes absolute paths (e.g. “C:\my\path”) with the Win32 File Namespace prefix
      (e.g. “\\?\C:\my\path”).

   s
      Path to parse.
   return
      Path suitable for abc::file_path’s internal representation.
   */
   static dmstr validate_and_adjust(dmstr s);


private:

   /** Full file path, always in normalized form. */
   dmstr m_s;
   /** Platform-specific path component separator. */
   static char_t const smc_aszSeparator[1 /*"/" or "\"*/ + 1 /*NUL*/];
   /** Platform-specific root path. */
   static char_t const smc_aszRoot[
#if ABC_HOST_API_POSIX
      1 /*"/"*/ + 1 /*NUL*/
#elif ABC_HOST_API_WIN32
      4 /*"\\?\"*/ + 1 /*NUL*/
#else
   #error TODO-PORT: HOST_API
#endif
   ];
#if ABC_HOST_API_WIN32
   /** Root for UNC paths in the Win32 File Namespace. */
   static char_t const smc_aszUNCRoot[8 /*"\\?\UNC\"*/ + 1 /*NUL*/];
#endif
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

// Specialization of to_str_backend.
template <>
class ABCAPI to_str_backend<file_path> :
   public to_str_backend<istr> {
public:

   /** Constructor.

   crFormat
      Formatting options.
   */
   to_str_backend(char_range const & crFormat = char_range());


   /** Writes a string, applying the formatting options.

   fp
      File path to write.
   posOut
      Pointer to the output stream to write to.
   */
   void write(file_path const & fp, ostream * posOut);
};

} //namespace abc


namespace std {

// Specialization of std::hash.
template <>
struct hash<abc::file_path>  {

   /** See std::hash::operator()().
   */
   size_t operator()(abc::file_path const & fp) const {
      return std::hash<abc::istr>()(static_cast<abc::istr const &>(fp));
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
   _file_path_iterator(file_path const & pathDir, istr const & sPattern) :
      m_pathBaseDir(pathDir),
      m_hSearch(find_first_file((m_pathBaseDir / sPattern).data(), &m_wfd)),
      m_bEOF(m_hSearch == INVALID_HANDLE_VALUE) {
      if (!m_bEOF) {
         m_pathCurr = next_file_path();
      }
   }
   _file_path_iterator(_file_path_iterator && iter) :
      m_pathBaseDir(std::move(iter.m_pathBaseDir)),
      m_hSearch(iter.m_hSearch),
      m_bEOF(iter.m_bEOF) {
      memory::copy(&m_wfd, &iter.m_wfd);
      iter.m_hSearch = INVALID_HANDLE_VALUE;
   }


   // Destructor.
   //
   ~_file_path_iterator() {
      if (m_hSearch != INVALID_HANDLE_VALUE) {
         ::FindClose(m_hSearch);
      }
   }


   // Assignment operator.
   //
   _file_path_iterator & operator=(_file_path_iterator && iter);


   // Dereferencing operator.
   //
   file_path & operator*() {
      return m_pathCurr;
   }
   file_path const & operator*() const {
      return m_pathCurr;
   }


   // Dereferencing member access operator.
   //
   file_path & operator->() {
      return m_pathCurr;
   }
   file_path const & operator->() const {
      return m_pathCurr;
   }


   // Prefix increment operator.
   //
   _file_path_iterator & operator++() {
      if (::FindNextFileW(m_hSearch, &m_wfd)) {
         m_pathCurr = next_file_path();
      } else {
         unsigned long iErr(::GetLastError());
         if (iErr != ERROR_NO_MORE_FILES) {
            throw_os_error(iErr);
         }
         // Remember we hit the bottom.
         m_bEOF = true;
      }
      return *this;
   }


   // Returns true if there are still files to be enumerated.
   //
   operator bool() const {
      return !m_bEOF;
   }


private:

   // Wrapper for ::FindFirstFile(), to support RIIA.
   //
   static HANDLE find_first_file(char_t const * pszPattern, WIN32_FIND_DATA * pwfd) {
      HANDLE h(::FindFirstFileW(pszPattern, pwfd));
      if (h == INVALID_HANDLE_VALUE) {

         unsigned long iErr(::GetLastError());
         if (iErr != ERROR_FILE_NOT_FOUND) {
            throw_os_error(iErr);
         }
      }
      return h;
   }


   // Returns the path from the m_wfd member.
   //
   file_path next_file_path() const {
      return file_path(
         m_pathBaseDir / istr(m_wfd.cFileName, ::wcslen(m_wfd.cFileName)),
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
inline _file_path_iterator file_path::find(istr const & sPattern) const {
   return _file_path_iterator(*this, sPattern);
}
#endif

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_PATH_HXX


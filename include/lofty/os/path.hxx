/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_OS_PATH_HXX
#define _LOFTY_OS_PATH_HXX

#ifndef _LOFTY_HXX
   #error "Please #include <lofty.hxx> before this file"
#endif
#ifdef LOFTY_CXX_PRAGMA_ONCE
   #pragma once
#endif


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

// Enumerates directory entries.
#if 0
class _path_iterator;
#endif

/*! Filesystem path. An instance of this class is always either an empty path string ("") or a path that is
not necessarily normalized or absolute, but has no incorrect or redundant path separators; e.g. an
lofty::os::path instance will never contain “/a//b///c”, and under Win32 it will never be “C:/a” or “a\\\b/c”.

Under Win32, all absolute DOS-style paths (e.g. “C:\My\File”) are normalized to the Win32 File Namespace,
which means they all start with “\\?\”, forming e.g. “\\?\C:\My\File”. This prefix is also considered the
root, although trying to do anything with it other than concatenating more path components will most likely
result in exceptions being thrown. Nonetheless, this convention allows to have a single root under Win32 just
like under POSIX.

lofty::os::path instances can be used with the OS’s file API by using lofty::os::path::os_str(), which will
return a string suitable for such use. Under Win32, this will make the path absolute in order to be able to
use the Win32 File Namespace, which has advantages and drawbacks; os_str() mitigates the drawbacks, allowing
to overcome limitations in the Win32 file API. See lofty::os::path::os_str() for more information.

Reference for Python’s approach: “os.path — Common pathname manipulations” <http://docs.python.org/3/library/
os.path.html>
Reference for Win32: “Naming Files, Paths, and Namespaces” <http://msdn.microsoft.com/en-us/library/windows/
desktop/aa365247.aspx> */
class LOFTY_SYM path : public support_explicit_operator_bool<path> {

#if 0
   friend class _path_iterator;
#endif

public:
   //! Default constructor.
   path() {
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   path(path && src) :
      s_(_std::move(src.s_)) {
   }

   /*! Copy constructor.

   @param src
      Source object.
   */
   path(path const & src) :
      s_(src.s_) {
   }

   /*! Constructor from plain string.

   @param s
      Source path string.
   */
   path(str && s) :
      s_(validate_and_adjust(_std::move(s))) {
   }

   /*! Constructor from plain string.

   @param s
      Source path string.
   */
   path(str const & s) :
      s_(validate_and_adjust(s)) {
   }

   /*! Move-assignment operator.

   @param src
      Source path.
   @return
      *this.
   */
   path & operator=(path && src) {
      s_ = _std::move(src.s_);
      return *this;
   }

   /*! Copy-assignment operator.

   @param src
      Source path.
   @return
      *this.
   */
   path & operator=(path const & src) {
      s_ = src.s_;
      return *this;
   }

   /*! Assignment operator from plain string.

   @param s
      Source path string.
   @return
      *this.
   */
   path & operator=(str && s) {
      s_ = validate_and_adjust(_std::move(s));
      return *this;
   }

   /*! Assignment operator from plain string.

   @param s
      Source path string.
   @return
      *this.
   */
   path & operator=(str const & s) {
      s_ = validate_and_adjust(s);
      return *this;
   }

   /*! Boolean evaluation operator.

   @return
      true if the path string is not empty, or false otherwise.
   */
   LOFTY_EXPLICIT_OPERATOR_BOOL() const {
      return bool(s_);
   }

   /*! Automatic cast to string.

   @return
      Constant reference to the internal path string.
   */
   operator str const &() const {
      return s_;
   }

   /*! Concatenation-assignment operator.

   @param s
      String to append.
   @return
      *this.
   */
   path & operator+=(str const & s) {
      s_ = validate_and_adjust(s_ + s);
      return *this;
   }

   /*! Concatenation operator.

   @param s
      String to append.
   @return
      Resulting path.
   */
   path operator+(str const & s) const {
      return path(*this) += s;
   }

   /*! Path-correct concatenation-assignment operator. Joins the current path with the provided string,
   inserting a separator if necessary.

   @param s
      Path component(s) to append.
   @return
      *this.
   */
   path & operator/=(str const & s);

   /*! Path-correct concatenation operator. See operator/=() for details.

   @param s
      Path component(s) to append.
   @return
      Resulting path.
   */
   path operator/(str const & s) const {
      return path(*this) /= s;
   }

   /*! Returns the absolute and normalized version of the path. If the path is not already absolute, it will
   be assumed to be relative to lofty::os::path::current_dir(). Under Win32 there’s a current directory for
   each volume, so the base directory will be different depending on whether the path includes a volume
   designator and on which volume it identifies.

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

#if LOFTY_HOST_API_WIN32
   /*! Returns the current directory for the specified volume.

   @param volume
      Volume designator.
   @return
      Current directory in volume.
   */
   static path current_dir_for_volume(char_t volume);
#endif //if LOFTY_HOST_API_WIN32

#if 0
   /*! Returns an iterator over entries in the path matching the specified pattern.

   TODO: comment signature.
   */
   _path_iterator find(str const & pattern) const;
#endif

   /*! Returns true if the path is in absolute form. Under Win32, this means that the path is prefixed with
   “\\?\”, e.g. “\\?\C:\my\path”.

   @return
      true if the path is absolute, or false otherwise.
   */
   bool is_absolute() const {
      return is_absolute(s_);
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
      return get_root_length(
         s_
#if LOFTY_HOST_API_WIN32
         , false
#endif
      ) == s_.size_in_chars();
   }

   /*! Returns a normalized version of the path by interpreting sequences such as “.” and “..”. The resulting
   replacements may lead to a different path if the original path includes symbolic links.

   @return
      Normalized path.
   */
   path normalize() const;

   /*! Returns a string representation of the path suitable to use with the OS’s file API.

   Under Win32, this returns the absolute (and normalized) version of the path, in order to overcome both the
   MAX_PATH limitation by using the Win32 File Namespace prefix, as well as parsing the path in a way that
   Windows won’t do for Win32 File Namespace-prefixed paths, as documented in “Naming Files, Paths, and
   Namespaces” <http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx>:

      “[The Win32 File Namespace prefix and its UNC sub-namespace] indicate that the path should be passed to
      the system with minimal modification, which means that you cannot use forward slashes to represent path
      separators, or a period to represent the current directory, or double dots to represent the parent
      directory.”

   @return
      String representation of the path suitable for use with the OS’s file API.
   */
#if LOFTY_HOST_API_POSIX
   // Under POSIX we don’t need an intermediate string, so the return type can be str const &.
   str const & os_str() const {
      return s_;
   }
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   str os_str() const;
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else

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
   static str separator() {
      return str(separator_);
   }

   /*! Returns the count of characters in the path.

   @return
      Count of characters.
   */
   std::size_t size() const {
      return s_.size();
   }

private:
   /*! Locates the first character of the final component in the path, e.g. “a” in “a” (all), “a” in “/a”,
   “/b/a” (POSIX), “\\?\UNC\a”, “\\?\UNC\b\a”, “\\?\X:\a”, “\\?\X:\b\a”, “\a”, “\b\a”, “X:a”, “X:b\a” (Win32).

   @return
      Iterator pointing to the first character of the final component in s_, or the beginning of s_ if the
      path does not contain a root component/prefix.
   */
   str::const_iterator base_name_start() const;

   /*! Returns the length of the root part of the specified path or, in other words, the index of the first
   character in the path that is not part of the root, e.g. “a” in “/a” (POSIX), “\\?\UNC\a”, “\\?\X:\a”,
   “\a”, “X:a” (Win32); the last two only if include_non_absolute because they represent relative paths in
   Win32: “\a” is relative to the current directory’s volume designator, “X:a” is relative to the current
   directory for volume designator X.

   @param s
      Path to parse. Must comply with the rules set for lofty::os::path’s internal representation.
   @param include_non_absolute
      (Win32 only) If omitted or true, non-absolute prefixes such as “\” and ”X:” will be considered root
      prefixes; if false, they won’t.
   @return
      Length of the root part in s, or 0 if s does not start with a root part.
   */
   static std::size_t get_root_length(
      str const & s
#if LOFTY_HOST_API_WIN32
      , bool include_non_absolute = true
#endif
   );

   /*! Returns true if the specified string represents an absolute path. Under Win32, this means that the path
   needs to be prefixed with “\\?\”, e.g. “\\?\C:\my\path”; a path starting with a volume designator (e.g.
   “C:\my\path”) is not considered absolute, as far as lofty::os::path is concerned (and it will never be
   stored as-is in s_ either).

   @param s
      Path to parse. Must comply with the rules set for lofty::os::path’s internal representation.
   @return
      true if s represents an absolute path, or false otherwise.
   */
   static bool is_absolute(str const & s);

   /*! Validates and adjusts a path to make it suitable as lofty::os::path’s internal representation:
   •  Collapses sequences of consecutive path separators into a single separator;
   •  Removes any trailing separators;
   •  (Win32 only) Replaces forward slashes with backslashes;
   •  (Win32 only) Prefixes absolute paths (e.g. “C:\my\path”) with the Win32 File Namespace prefix (e.g.
      “\\?\C:\my\path”).

   @param s
      Path to parse.
   @return
      Path suitable for lofty::os::path’s internal representation.
   */
   static str validate_and_adjust(str s);

private:
   //! Full path, always in normalized form.
   str s_;
   //! Platform-specific path component separator.
   static char_t const separator_[1 /*"/" or "\"*/ + 1 /*NUL*/];
   //! Platform-specific root path.
   static char_t const root_[
#if LOFTY_HOST_API_POSIX
      1 /*"/"*/ + 1 /*NUL*/
#elif LOFTY_HOST_API_WIN32
      4 /*"\\?\"*/ + 1 /*NUL*/
#else
   #error "TODO: HOST_API"
#endif
   ];
#if LOFTY_HOST_API_WIN32
   //! Root for UNC paths in the Win32 File Namespace.
   static char_t const unc_root[8 /*"\\?\UNC\"*/ + 1 /*NUL*/];
#endif
};

// Relational operators.
#define LOFTY_RELOP_IMPL(op) \
   inline bool operator op(path const & left, path const & right) { \
      return static_cast<str const &>(left) op static_cast<str const &>(right); \
   }
LOFTY_RELOP_IMPL(==)
LOFTY_RELOP_IMPL(!=)
LOFTY_RELOP_IMPL(>)
LOFTY_RELOP_IMPL(>=)
LOFTY_RELOP_IMPL(<)
LOFTY_RELOP_IMPL(<=)
#undef LOFTY_RELOP_IMPL

}} //namespace lofty::os

//! @cond
namespace std {

template <>
struct hash<lofty::os::path> : public hash<lofty::text::str> {
   //! See std::hash::operator()().
   std::size_t operator()(lofty::os::path const & path) const {
      return hash<lofty::text::str>::operator()(path);
   }
};

} //namespace std
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty {

template <>
class LOFTY_SYM to_text_ostream<os::path> {
public:
   /*! Changes the output format.

   @param format
      Formatting options.
   */
   void set_format(str const & format);

   /*! Writes a path, applying the formatting options.

   @param src
      Path to write.
   @param dst
      Pointer to the stream to output to.
   */
   void write(os::path const & src, io::text::ostream * dst);
};

} //namespace lofty
//! @endcond

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

#if 0
class _path_iterator {
public:
   //! Constructor.
   _path_iterator(path const & dir_path, str const & pattern) :
      base_path(dir_path),
      search_handle(find_first_file((base_path / pattern).os_str().c_str(), &curr)),
      eof(search_handle == INVALID_HANDLE_VALUE) {
      if (!eof) {
         curr_path = next_path();
      }
   }

   /*! Move constructor.

   @param src
      Source object.
   */
   _path_iterator(_path_iterator && src) :
      base_path(_std::move(src.base_path)),
      search_handle(src.search_handle),
      eof(src.eof) {
      memory::copy(&curr, &src.curr);
      src.search_handle = INVALID_HANDLE_VALUE;
   }

   //! Destructor.
   ~_path_iterator() {
      if (search_handle != INVALID_HANDLE_VALUE) {
         ::FindClose(search_handle);
      }
   }

   /*! Move-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   _path_iterator & operator=(_path_iterator && src);

   //! Dereferencing operator.
   path & operator*() {
      return curr_path;
   }
   path const & operator*() const {
      return curr_path;
   }

   //! Dereferencing member access operator.
   path & operator->() {
      return curr_path;
   }
   path const & operator->() const {
      return curr_path;
   }

   //! Prefix increment operator.
   _path_iterator & operator++() {
      if (::FindNextFileW(search_handle, &curr)) {
         curr_path = next_path();
      } else {
         auto err = ::GetLastError();
         if (err != ERROR_NO_MORE_FILES) {
            exception::throw_os_error(err);
         }
         // Remember we hit the bottom.
         eof = true;
      }
      return *this;
   }

   //! Returns true if there are still files to be enumerated.
   operator bool() const {
      return !eof;
   }

private:
   //! Wrapper for ::FindFirstFile(), to support RIIA.
   static ::HANDLE find_first_file(char_t const * pattern, ::WIN32_FIND_DATA * out) {
      ::HANDLE ret = ::FindFirstFileW(pattern, out);
      if (ret == INVALID_HANDLE_VALUE) {
         auto err = ::GetLastError();
         if (err != ERROR_FILE_NOT_FOUND) {
            exception::throw_os_error(err);
         }
      }
      return ret;
   }

   //! Returns the path from the curr member.
   path next_path() const {
      return path(base_path / str(curr.cFileName, ::wcslen(curr.cFileName)), curr.dwFileAttributes);
   }

private:
   //! Directory being enumerated.
   path base_path;
   //! Search data.
   ::WIN32_FIND_DATA curr;
   //! Fake handle to the search.
   ::HANDLE search_handle;
   //! true if we run out of files.
   bool eof;
   //! Current item.
   path curr_path;
};


// Now this can be defined.

inline _path_iterator path::find(str const & pattern) const {
   return _path_iterator(*this, pattern);
}
#endif

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_OS_PATH_HXX

/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/collections/vector.hxx>
#include <lofty/exception.hxx>
#include <lofty/io/text.hxx>
#include <lofty/os/path.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text.hxx>
#include <lofty/text/str.hxx>
#include <lofty/to_text_ostream.hxx>
#if LOFTY_HOST_API_POSIX
   #include <errno.h> // errno E*
   #include <sys/stat.h> // S_*, stat()
   #include <unistd.h> // getcwd()
#elif LOFTY_HOST_API_WIN32
   #include <lofty/memory.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

#if LOFTY_HOST_API_POSIX

namespace {

//! Wrapper for a stat structure that self-loads with information on the file.
class file_stat : public ::stat {
public:
   /*! Constructor.

   path_
      Path to get statistics for.
   */
   file_stat(path const & path_) {
      if (::stat(path_.os_str().c_str(), this)) {
         exception::throw_os_error();
      }
   }
};

} //namespace

#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX

/*! Checks whether a path has the specified attribute(s) set.

path_
   Path to get attributes of.
attrs
   Combination of one or more FILE_ATTRIBUTE_* flags to check for.
return
   true if the path has all the file attributes in attrs, or false otherwise.
*/
static bool file_has_attrs(path const & path_, ::DWORD attrs) {
   ::DWORD all_attrs = ::GetFileAttributes(path_.os_str().c_str());
   if (all_attrs == INVALID_FILE_ATTRIBUTES) {
      exception::throw_os_error();
   }
   return (all_attrs & attrs) == attrs;
}

#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32


text::char_t const path::separator_[] =
#if LOFTY_HOST_API_POSIX
   LOFTY_SL("/");
#elif LOFTY_HOST_API_WIN32
   LOFTY_SL("\\");
#else
   #error "TODO: HOST_API"
#endif
text::char_t const path::root_[] =
#if LOFTY_HOST_API_POSIX
   LOFTY_SL("/");
#elif LOFTY_HOST_API_WIN32
   LOFTY_SL("\\\\?\\");
#else
   #error "TODO: HOST_API"
#endif
#if LOFTY_HOST_API_WIN32
text::char_t const path::unc_root[] = LOFTY_SL("\\\\?\\UNC\\");
#endif

path & path::operator/=(text::str const & s) {
   // Only the root already ends in a separator; everything else needs one.
   s_ = validate_and_adjust((!s_ || is_root() ? text::str(s_) : s_ + separator_) + s);
   return *this;
}

path path::absolute() const {
   path absolute_path;
   if (is_absolute()) {
      absolute_path = *this;
   } else {
#if LOFTY_HOST_API_POSIX
      // Prepend the current directory to make the path absolute, then proceed to normalize.
      absolute_path = current_dir() / *this;
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
      static std::size_t const volume_index      = 0; // “X” in “X:”.
      static std::size_t const volume_colon_index = 1; // “:” in “X:”.
      static std::size_t const leading_sep_index  = 0; // “\” in “\”.

      /* Under Win32, a path can be absolute but relative to a volume, or it can specify a volume and be
      relative to the current directory in that volume. Either way, these two formats don’t qualify as
      absolute (which is why we’re here), and can be recognized as follows. */
      std::size_t s_size = s_.size_in_chars();
      text::char_t const * s_chars = s_.data();
      if (s_size > volume_colon_index && *(s_chars + volume_colon_index) == ':') {
         /* The path is in the form “X:a”: get the current directory for that volume and prepend it to the
         path to make it absolute. */
         absolute_path = current_dir_for_volume(*(s_chars + volume_index)) /
                      s_.substr(s_.cbegin() + volume_colon_index + 1 /*“:”*/);
      } else if (s_size > leading_sep_index && *(s_chars + leading_sep_index) == '\\') {
         /* The path is in the form “\a”: make it absolute by prepending to it the volume designator of the
         current directory. */
         path curr_dir_path(current_dir());
         auto curr_dir_begin(curr_dir_path.s_.cbegin());
         absolute_path = curr_dir_path.s_.substr(
            curr_dir_begin, curr_dir_begin + LOFTY_COUNTOF(root_) - 1 /*NUL*/ + 2 /*"X:"*/
         ) + s_;
      } else {
         // None of the above patterns applies: prepend the current directory to make the path absolute.
         absolute_path = current_dir() / *this;
      }
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
   }
   // Make sure the path is normalized.
   return absolute_path.normalize();
}

path path::base_name() const {
   return s_.substr(base_name_start());
}

/*static*/ path path::current_dir() {
   text::str ret;
#if LOFTY_HOST_API_POSIX
   ret.set_from([] (text::char_t * chars, std::size_t chars_max) -> std::size_t {
      if (::getcwd(chars, chars_max)) {
         // The length will be necessarily less than chars_max, so set_from() will stop.
         return text::size_in_chars(chars);
      }
      auto err = errno;
      if (err != ERANGE) {
         exception::throw_os_error(err);
      }
      // Report that the provided buffer was too small.
      return chars_max;
   });
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   /* Since we want to prefix the result of ::GetCurrentDirectory() with root_, we’ll make str::set_from()
   allocate space for that too, by adding the size of the root to the buffer size while advancing the buffer
   pointer we pass to ::GetCurrentDirectory() in order to reserve space for the root prefix. */
   ret.set_from([] (text::char_t * chars, std::size_t chars_max) -> std::size_t {
      static std::size_t const root_size = LOFTY_COUNTOF(root_) - 1 /*NUL*/;
      if (root_size >= chars_max) {
         // If the buffer is not large enough to hold the root prefix, request a larger one.
         return chars_max;
      }
      ::DWORD cwd_size = ::GetCurrentDirectory(
         static_cast< ::DWORD>(chars_max - root_size), chars + root_size
      );
      if (cwd_size == 0) {
         exception::throw_os_error();
      }
      return cwd_size + root_size;
   });
   // Now that the current directory has been retrieved, prepend the root prefix.
   memory::copy(ret.data(), root_, LOFTY_COUNTOF(root_) - 1 /*NUL*/);
#else //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32
   #error "TODO: HOST_API"
#endif //if LOFTY_HOST_API_POSIX … elif LOFTY_HOST_API_WIN32 … else
   return _std::move(ret);
}

#if LOFTY_HOST_API_WIN32
/*static*/ path path::current_dir_for_volume(text::char_t volume) {
   // Create a dummy path for ::GetFullPathName() to expand.
   text::char_t dummy_path[4] = { volume, ':', 'a', '\0' };
   text::str ret;
   ret.set_from([&dummy_path] (text::char_t * chars, std::size_t chars_max) -> std::size_t {
      static std::size_t const root_size = LOFTY_COUNTOF(root_) - 1 /*NUL*/;
      if (root_size >= chars_max) {
         // If the buffer is not large enough to hold the root prefix, request a larger one.
         return chars_max;
      }
      ::DWORD abs_path_size = ::GetFullPathName(
         dummy_path, static_cast< ::DWORD>(chars_max - root_size), chars + root_size, nullptr
      );
      if (abs_path_size == 0) {
         exception::throw_os_error();
      }
      return abs_path_size + root_size;
   });
   // Now that the current directory has been retrieved, prepend the root prefix.
   memory::copy(ret.data(), root_, LOFTY_COUNTOF(root_) - 1 /*NUL*/);
   // Remove the last character, the “a” from dummy_path.
   ret.set_size_in_chars(ret.size_in_chars() - 1 /*“a”*/);
   return _std::move(ret);
}
#endif //if LOFTY_HOST_API_WIN32

bool path::is_dir() const {
#if LOFTY_HOST_API_POSIX
   return S_ISDIR(file_stat(*this).st_mode);
#elif LOFTY_HOST_API_WIN32
   return file_has_attrs(*this, FILE_ATTRIBUTE_DIRECTORY);
#else
   #error "TODO: HOST_API"
#endif
}

path path::normalize() const {
   text::str s(s_);
   auto begin(s.begin()), end(s.end());
   auto root_end(begin + static_cast<std::ptrdiff_t>(get_root_length(s)));

   /* Interpret “.” and “..” components, starting from root_end. Every time we encounter a separator, store
   its iterator in separator_itrs; when we encounter a “..” component, we’ll jump back to to the character
   following the last (“.”) or second-last (“..”) separator encountered, or root_end if no previous separators
   are available:
   •  Upon encountering the second “/” in “a/./”, roll back to index 2;
   •  Upon encountering the second “/” in “a/../”, roll back to index 0 (root_end);
   •  Upon encountering the second “/” in “/../a”, roll back to index 1 (root_end).
   */
   collections::vector<text::str::iterator, 5> separator_itrs;
   std::size_t dots = 0;
   auto dst(root_end);
   for (auto src(root_end); src < end; ++src) {
      char32_t ch = *src;
      if (ch == '.') {
         ++dots;
      } else {
         if (ch == text::codepoint(separator_[0])) {
            if (dots > 0 && dots <= 2) {
               /* We found “./” or “../”: go back by as many separators as the count of dots, if we
               encountered enough separators. */
               if (separator_itrs.size() >= dots) {
                  auto prev_sep_itr(separator_itrs.cend() - static_cast<std::ptrdiff_t>(dots));
                  dst = *prev_sep_itr + 1 /*“/”*/;
                  if (dots > 1) {
                     // We jumped back two separators; discard the one we jumped over (cend() - 1).
                     separator_itrs.remove_at(prev_sep_itr + 1);
                  }
               } else {
                  /* We don’t have enough separators in separator_itrs; resume from the end of the root or the
                  start of the path. */
                  dst = root_end;
                  separator_itrs.clear();
               }
               // Resume from the next character, which will be written to *dst.
               dots = 0;
               continue;
            }
            // Remember this separator.
            separator_itrs.push_back(src);
         }
         dots = 0;
      }
      // If the characer needs to be moved, move it.
      if (src != dst) {
         *dst = ch;
      }
      ++dst;
   }
   if (dots > 0 && dots <= 2) {
      /* We ended on “.” or “..”, go back by as many separators as the count of dots, if we found enough
      separators. */
      if (separator_itrs.size() >= dots) {
         // Place dst on the separator, so we don’t end up with a traling separator.
         dst = *(separator_itrs.data_end() - dots);
      } else {
         /* We don’t have enough separators in separator_itrs; resume from the end of the root or the start of
         the path. */
         dst = root_end;
      }
   } else if (dst > root_end && *(dst - 1) == separator_[0]) {
      // The last character written was a separator; rewind by 1 to avoid a trailing separator.
      --dst;
   }

   // Adjust the length based on the position of the last character written.
   s.set_size_in_chars(dst.char_index());
   return _std::move(s);
}

#if LOFTY_HOST_API_WIN32
text::str path::os_str() const {
   return _std::move(absolute().s_);
}
#endif //if LOFTY_HOST_API_WIN32

path path::parent_dir() const {
   auto begin(s_.cbegin());
   auto last_sep(base_name_start());
   if (last_sep == begin) {
      // This path only contains a base name, so there’s no parent directory part.
      return path();
   }
   /* If there’s a root separator/prefix, make sure we don’t destroy it by stripping it of a separator;
   advance the iterator instead. */
   if (last_sep - begin < static_cast<std::ptrdiff_t>(get_root_length(s_))) {
      ++last_sep;
   }
   return s_.substr(begin, last_sep);
}

/*static*/ path path::root() {
   return text::str(root_);
}

text::str::const_iterator path::base_name_start() const {
   auto ret(s_.find_last(separator_[0]));
   if (ret == s_.cend()) {
      ret = s_.cbegin();
   }
#if LOFTY_HOST_API_WIN32
   // Special case for the non-absolute “X:a”, in which case only “a” is the base name.
   static std::size_t const volume_colon_index = 1; //“:” in “X:”.

   std::size_t s_size = s_.size_in_chars();
   if (s_size > volume_colon_index) {
      auto volume_colon_itr(s_.cbegin() + volume_colon_index);
      /* If the path is in the form “X:a” and so far we considered “X” the start of the base name, reconsider
      the character after the colon as the start of the base name. */
      if (*volume_colon_itr == ':' && ret <= volume_colon_itr) {
         ret = volume_colon_itr + 1 /*“:”*/;
      }
   }
#endif
   return ret;
}

/*static*/ std::size_t path::get_root_length(
   text::str const & s
#if LOFTY_HOST_API_WIN32
   , bool include_non_absolute /*= true*/
#endif
) {
   static std::size_t const root_size = LOFTY_COUNTOF(root_) - 1 /*NUL*/;

#if LOFTY_HOST_API_POSIX
   if (s.starts_with(root_)) {
      // Return the index of “a” in “/a”.
      return root_size;
   }
#elif LOFTY_HOST_API_WIN32
   static std::size_t const unc_root_size = LOFTY_COUNTOF(unc_root) - 1 /*NUL*/;
   static std::size_t const volume_root_size = root_size + 3; //“X:\”
   static std::size_t const volume_colon_index = 1; // “:” in “X:”
   static std::size_t const leading_sep_index = 0; // “\” in “\”

   std::size_t s_size = s.size_in_chars();
   text::char_t const * s_chars = s.data();
   if (s.starts_with(root_)) {
      if (s.starts_with(unc_root)) {
         // Return the index of “a” in “\\?\UNC\a”.
         return unc_root_size;
      }
      text::char_t ch;
      LOFTY_UNUSED_ARG(ch);
      LOFTY_ASSERT(
         s_size >= volume_root_size &&
         (ch = *(s_chars + volume_root_size - 3), ch >= 'A' && ch <= 'Z') &&
         (*(s_chars + volume_root_size - 2) == ':' && *(s_chars + volume_root_size - 1) == '\\'),
         LOFTY_SL("Win32 File Namespace must continue in either \\\\?\\UNC\\ or \\\\?\\X:\\; ")
            LOFTY_SL("lofty::os::path::validate_and_adjust() needs to be fixed")
      );
      // Return the index of “a” in “\\?\X:\a”.
      return root_size;
   }
   if (include_non_absolute) {
      if (s_size > volume_colon_index && *(s_chars + volume_colon_index) == ':') {
         // Return the index of “a” in “X:a”.
         return volume_colon_index + 1 /*“:”*/;
      }
      if (s_size > leading_sep_index && *(s_chars + leading_sep_index) == '\\') {
         // Return the index of “a” in “\a”.
         return leading_sep_index + 1 /*“\”*/;
      }
   }
#else
   #error "TODO: HOST_API"
#endif
   return 0;
}

/*static*/ bool path::is_absolute(text::str const & s) {
   return s.starts_with(root_);
}

/*static*/ text::str path::validate_and_adjust(text::str s) {
#if LOFTY_HOST_API_WIN32
   // Simplify the logic below by normalizing all slashes to backslashes.
   s.replace('/', '\\');

   if (!is_absolute(s)) {
      /* lofty::os::path::is_absolute() is very strict and does not return true for DOS-style or UNC paths,
      i.e. those without the Win32 File Namespace prefix “\\?\”, such as “C:\my\path” or “\\server\share”, so
      we have to detect them here and prefix them with the Win32 File Namespace prefix. */

      if (s.starts_with(LOFTY_SL("\\\\"))) {
         // This is an UNC path; prepend to it the Win32 File Namespace prefix for UNC paths.
         s = unc_root + s.substr(s.cbegin() + 2 /*“\\”*/);
      } else {
         std::size_t s_size = s.size_in_chars();
         text::char_t * s_chars = s.data();
         if (s_size >= 2 && *(s_chars + 1) == ':') {
            text::char_t volume = *s_chars;
            // If the path is in the form “x:”, normalize the volume designator to uppercase.
            if (volume >= 'a' && volume <= 'z') {
               volume -= 'a' - 'A';
               *s_chars = volume;
            } else if (volume < 'A' || volume > 'Z') {
               // Avoid keeping a path that can’t be valid.
               // TODO: use a better exception class, or maybe just explicitly pass the invalid path.
               exception::throw_os_error(ERROR_INVALID_DRIVE);
            }
            if (s_size >= 3 /*“X:\”*/ && *(s_chars + 2) == '\\') {
               // This is a DOS-style absolute path; prepend to it the Win32 File Namespace prefix.
               s = root_ + s;
            }
         }
      }
   }
#endif //if LOFTY_HOST_API_WIN32

   auto begin(s.begin()), end(s.end());
   // Save an iterator to the end of the root prefix.
   auto root_end(begin + static_cast<std::ptrdiff_t>(get_root_length(s)));

   // Collapse sequences of one or more path separators with a single separator.
   auto dst(root_end);
   bool prev_is_separator = false;
   for (auto src(root_end); src != end; ++src) {
      char32_t ch = *src;
      bool curr_is_separator = ch == text::codepoint(separator_[0]);
      if (curr_is_separator && prev_is_separator) {
         // Collapse consecutive separators by advancing src (as part of the for loop) without advancing dst.
         continue;
      }
      // Remember whether this character is a separator.
      prev_is_separator = curr_is_separator;
      // If the characer needs to be moved, move it.
      if (dst != src) {
         *dst = ch;
      }
      ++dst;
   }
   /* If the last character written is a separator and it wouldn’t leave an empty string (other than any
   prefix), move dst back. */
   if (prev_is_separator && dst > root_end) {
      --dst;
   }

   // Adjust the length based on the position of the last character written.
   s.set_size_in_chars(dst.char_index());
   return _std::move(s);
}

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<os::path>::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<os::path>::write(os::path const & src, io::text::ostream * dst) {
   dst->write(src);
}

} //namespace lofty

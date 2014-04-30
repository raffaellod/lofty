/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#include <abc/core.hxx>
#include <abc/file_path.hxx>
#include <abc/trace.hxx>
#include <abc/vector.hxx>
#if ABC_HOST_API_POSIX
   #include <errno.h> // errno E*
   #include <sys/stat.h> // S_*, stat()
   #include <unistd.h> // getcwd()
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_path


namespace abc {

#if ABC_HOST_API_POSIX

/** Wrapper for a stat structure that self-loads with information on the file.
*/
class file_stat :
   public ::stat {
public:

   /** Constructor.

   fp
      Path to get statistics for.
   */
   file_stat(file_path const & fp) {
      if (::stat(fp.os_str().data(), this)) {
         throw_os_error();
      }
   }
};

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

/** Checks whether a path has the specified attribute(s) set.

fp
   Path to get attributes of.
fi
   Combination of one or more FILE_ATTRIBUTE_* flags to check for.
return
   true if the path has all the file attributes in fi, or false otherwise.
*/
static bool file_attrs(file_path const & fp, DWORD fi) {
   DWORD fiAttrs(::GetFileAttributes(fp.data()));
   if (fiAttrs == INVALID_FILE_ATTRIBUTES) {
      throw_os_error();
   }
   return (fiAttrs & fi) == fi;
}

#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32



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
#if ABC_HOST_API_WIN32
char_t const file_path::smc_aszUNCRoot[] = SL("\\\\?\\UNC\\");
#endif



file_path & file_path::operator/=(istr const & s) {
   ABC_TRACE_FN((this, s));

   // Only the root already ends in a separator; everything else needs one.
   m_s = validate_and_adjust((!m_s || is_root() ? dmstr(m_s) : m_s + smc_aszSeparator[0]) + s);
   return *this;
}


file_path file_path::absolute() const {
   ABC_TRACE_FN((this));

   file_path fpAbsolute;
   if (is_absolute()) {
      fpAbsolute = *this;
   } else {
#if ABC_HOST_API_POSIX
      // Prepend the current directory to make the path absolute, then proceed to normalize.
      fpAbsolute = current_dir() / *this;
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
      file_path fpCurrentDir(current_dir());
      // Under Win32, a path can be absolute but lack a volume designator. In this case, get the
      // current volume and make the path relative to that.
      // Check if the path begins with a single backslash, i.e. it’s absolute relatively to a volume
      // designator. There’s no need to check that it does not begin with two backslashes, because
      // the \\?\ prefix is already checked for by is_absolute() (above) and UNC paths are always
      // (in abc::file_path) prefixed by \\?\ too.
      if (m_s.size() >= 1 && m_s[0] == CL('\\')) {
         fpAbsolute = fpCurrentDir.m_s.substr(
            0, ABC_COUNTOF(smc_aszRoot) - 1 /*NUL*/ + 2 /*"X:"*/
         ) + m_s;
      } else {
         // Prepend the current directory to make the path absolute, then proceed to normalize.
         fpAbsolute = fpCurrentDir / *this;
      }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   }
   // Make sure the path is normalized.
   return fpAbsolute.normalize();
}


file_path file_path::base_name() const {
   ABC_TRACE_FN((this));

   return m_s.substr(base_name_start());
}


/*static*/ file_path file_path::current_dir() {
   ABC_TRACE_FN(());

   dmstr s;
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
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   // Since we want to prefix the result of ::GetCurrentDirectory() with smc_aszRoot, we’ll make
   // mstr::grow_for() allocate space for that too, by adding the size of the root to the buffer
   // size while advancing the buffer pointer we pass to ::GetCurrentDirectory() in order to
   // reserve space for the root prefix.
   size_t const c_cchRoot(ABC_COUNTOF(smc_aszRoot) - 1 /*NUL*/);
   s.grow_for([c_cchRoot] (char_t * pch, size_t cchMax) -> size_t {
      if (c_cchRoot >= cchMax) {
         // If the buffer is not large enough to hold the root prefix, request a larger one.
         return cchMax;
      }
      DWORD cch(::GetCurrentDirectory(DWORD(cchMax - c_cchRoot), pch + c_cchRoot));
      if (!cch) {
         throw_os_error();
      }
      return cch + c_cchRoot;
   });
   // Now that the current directory has been retrieved, prepend the root prefix.
   memory::copy(s.data(), smc_aszRoot, c_cchRoot);
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   return std::move(s);
}


bool file_path::is_dir() const {
   ABC_TRACE_FN((this));

#if ABC_HOST_API_POSIX
   return S_ISDIR(file_stat(*this).st_mode);
#elif ABC_HOST_API_WIN32
   return file_attrs(*this, FILE_ATTRIBUTE_DIRECTORY);
#else
   #error TODO-PORT: HOST_API
#endif
}


file_path file_path::normalize() const {
   ABC_TRACE_FN((this));

   dmstr s(m_s);
   auto itBegin(s.begin()), itEnd(s.end());
   auto itRootEnd(itBegin + ptrdiff_t(get_root_length(s, true)));

   // Interpret “.” and “..” components, starting from itRootEnd. Every time we encounter a
   // separator, store its iterator in vitSeps; when we encounter a “..” component, we’ll jump back
   // to the character following the last (“.”) or second-last (“..”) separator encountered, or
   // itRootEnd if no previous separators are available:
   // •  Upon encountering the second “/” in “a/./”, roll back to index 2;
   // •  Upon encountering the second “/” in “a/../”, roll back to index 0 (itRootEnd);
   // •  Upon encountering the second “/” in “/../a”, roll back to index 1 (itRootEnd).
   smvector<dmstr::iterator, 5> vitSeps;
   intptr_t cDots(0);
   auto itDst(itRootEnd);
   for (auto itSrc(itRootEnd); itSrc < itEnd; ++itSrc) {
      char_t ch(*itSrc);
      if (ch == CL('.')) {
         ++cDots;
      } else {
         if (ch == smc_aszSeparator[0]) {
            if (cDots > 0 && cDots <= 2) {
               // We found “./” or “../”: go back by as many separators as the count of dots.
               intptr_t iPrevSep(intptr_t(vitSeps.size()) - cDots);
               if (iPrevSep >= 0) {
                  itDst = vitSeps[iPrevSep] + 1 /*“/”*/;
                  // Remove the previous separators we used (cDots - 1).
                  vitSeps.remove_at(iPrevSep + 1, cDots - 1);
               } else {
                  // We don’t have enough separators in vitSeps; resume from the end of the root or
                  // the start of the path.
                  itDst = itRootEnd;
                  vitSeps.clear();
               }
               // Resume from the next character, which will be written to *itDst.
               cDots = 0;
               continue;
            }
            // Remember this separator.
            vitSeps.append(itSrc);
         }
         cDots = 0;
      }
      // If the characer needs to be moved, move it.
      if (itSrc != itDst) {
         *itDst = ch;
      }
      ++itDst;
   }
   if (cDots > 0 && cDots <= 2) {
      // We ended on “.” or “..”, go back by as many separators as the count of dots.
      intptr_t iPrevSep(intptr_t(vitSeps.size()) - cDots);
      if (iPrevSep >= 0) {
         // Place itDst on the separator, so we don’t end up with a traling separator.
         itDst = vitSeps[iPrevSep];
      } else {
         // We don’t have enough separators in vitSeps; resume from the end of the root or
         // the start of the path.
         itDst = itRootEnd;
      }
   } else if (itDst > itRootEnd && *(itDst - 1) == smc_aszSeparator[0]) {
      // The last character written was a separator; rewind by 1 to avoid a trailing separator.
      --itDst;
   }

   // Adjust the length based on the position of the last character written.
   s.set_size(size_t(itDst - itBegin));
   return std::move(s);
}


#if ABC_HOST_API_WIN32
istr file_path::os_str() const {
   ABC_TRACE_FN((this));

   return std::move(absolute().m_s);
}
#endif //if ABC_HOST_API_WIN32


file_path file_path::parent_dir() const {
   ABC_TRACE_FN((this));

   auto itBegin(m_s.cbegin()), itLastSep(base_name_start());
   if (itLastSep == itBegin) {
      // This path only contains a base name, so there’s no parent directory part.
      return file_path();
   }
   // If there’s a root separator/prefix, make sure we don’t destroy it by stripping it of a
   // separator; advance the iterator instead.
   if (itLastSep - itBegin < ptrdiff_t(get_root_length(m_s, true))) {
      ++itLastSep;
   }
   return m_s.substr(itBegin, itLastSep);
}


/*static*/ file_path file_path::root() {
   ABC_TRACE_FN(());

   return dmstr(smc_aszRoot);
}


dmstr::const_iterator file_path::base_name_start() const {
   ABC_TRACE_FN((this));

   auto itBaseNameStart(m_s.find_last(char32_t(smc_aszSeparator[0])));
   if (itBaseNameStart == m_s.cend()) {
      itBaseNameStart = m_s.cbegin();
   }
#if ABC_HOST_API_WIN32
   // Special case for the non-absolute “X:a”, in which case only “a” is the base name.
   static size_t const sc_ichVolumeColon(1 /*“:” in “X:”*/);

   size_t cch(s.size());
   if (cch > sc_ichVolumeColon) {
      auto itVolumeColon(m_s.cbegin() + sc_ichVolumeColon);
      // If the path is in the form “X:a” and so far we considered “X” the start of the base name,
      // reconsider the character after the colon as the start of the base name.
      if (*itVolumeColon == CL(':') && itBaseNameStart <= itVolumeColon) {
         itBaseNameStart = itVolumeColon + 1 /*“:”*/;
      }
   }
#endif
   return itBaseNameStart;
}


/*static*/ size_t file_path::get_root_length(dmstr const & s, bool bIncludeNonRoot) {
   ABC_TRACE_FN((s, bIncludeNonRoot));

   static size_t const sc_cchRoot(ABC_COUNTOF(smc_aszRoot) - 1 /*NUL*/);

#if ABC_HOST_API_POSIX
   if (s.starts_with(smc_aszRoot)) {
      // Return the index of “a” in “/a”.
      return sc_cchRoot;
   }
#elif ABC_HOST_API_WIN32
   static size_t const sc_cchUNCRoot(ABC_COUNTOF(smc_aszUNCRoot) - 1 /*NUL*/);
   static size_t const sc_cchVolumeRoot(sc_cchRoot + 3 /*“X:\”*/);
   static size_t const sc_ichVolumeColon(1 /*“:” in “X:”*/);
   static size_t const sc_ichLeadingSep(0 /*“\” in “\”*/);

   size_t cch(s.size());
   char_t const * pch(s.data());
   if (s.starts_with(smc_aszRoot)) {
      if (s.starts_with(smc_aszUNCRoot)) {
         // Return the index of “a” in “\\?\UNC\a”.
         return sc_cchUNCRoot;
      }
      ABC_ASSERT(
         cch < sc_cchVolumeRoot ||
         pch[sc_cchVolumeRoot - 3] < CL('A') || pch[sc_cchVolumeRoot - 3] > CL('Z'),
         pch[sc_cchVolumeRoot - 2] != CL(':') || pch[sc_cchVolumeRoot - 1] != CL('\\'),
         SL("Win32 File Namespace must continue in either \\\\?\\UNC\\ or \\\\?\\X:\\; ")
            SL("abc::file_path::validate_and_adjust() needs to be fixed")
      );
      // Return the index of “a” in “\\?\X:\a”.
      return sc_cchRoot;
   }
   if (bIncludeNonAbsolute) {
      if (cch > sc_ichVolumeColon && pch[sc_ichVolumeColon] == CL(':') {
         // Return the index of “a” in “X:a”.
         return sc_ichVolumeColon + 1 /*“:”*/;
      }
      if (cch > sc_ichLeadingSep && pch[sc_ichLeadingSep] == CL('\\')) {
         // Return the index of “a” in “\a”.
         return sc_ichLeadingSep + 1 /*“\”*/;
      }
   }
#else
   #error TODO-PORT: HOST_API
#endif
   return 0;
}


/*static*/ bool file_path::is_absolute(istr const & s) {
   ABC_TRACE_FN((s));

   return s.starts_with(smc_aszRoot);
}


/*static*/ dmstr file_path::validate_and_adjust(dmstr s) {
   ABC_TRACE_FN((s));

   auto itBegin(s.begin()), itEnd(s.end());

#if ABC_HOST_API_WIN32
   // Simplify the logic below by normalizing all slashes to backslashes.
   // TODO: change to use mstr::replace() when that becomes available.
   for (auto it(itBegin); it != itEnd; ++it) {
      if (*it == CL('/')) {
         *it = CL('\\');
      }
   }
#endif //if ABC_HOST_API_WIN32

   // Save an iterator to the end of the root prefix.
   auto itRootEnd(itBegin);
#if ABC_HOST_API_POSIX
   itRootEnd += ptrdiff_t(get_root_length(s, true));
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   bool bIsAbsolute(is_absolute(s));
   if (!bIsAbsolute) {
      // abc::file_path::is_absolute() is very strict and does not return true for DOS-style or UNC
      // paths, i.e. those without the Win32 File Namespace prefix “\\?\”, such as “C:\my\path” or
      // “\\server\share”, so we have to detect them here and prefix them with the Win32 File
      // Namespace prefix.

      bool bUpdateIterators(false);
      if (s.starts_with(SL("\\\\"))) {
         // This is an UNC path; prepend to it the Win32 File Namespace prefix for UNC paths.
         s = smc_aszUNCRoot + s.substring(2 /*“\\”*/);
         bIsAbsolute = true;
         bUpdateIterators = true;
      } else {
         size_t cch(s.size());
         if (cch >= 2 && s[1] == CL(':')) {
            char_t chVolume(s[0]);
            // If the path is in the form “x:”, normalize the volume designator to uppercase.
            if (chVolume >= CL('a') && chVolume <= CL('z')) {
               chVolume -= CL('a') - CL('A');
               s[0] = chVolume;
            } else if (chVolume < CL('A') || chVolume > CL('Z')) {
               // Avoid keeping a path that can’t be valid.
               throw_os_error(ERROR_INVALID_DRIVE);
            }
            if (cch >= 3 /*“X:\”*/ && s[2] == CL('\\')) {
               // This is a DOS-style absolute path; prepend to it the Win32 File Namespace prefix.
               s = smc_aszRoot + s;
               bIsAbsolute = true;
               bUpdateIterators = true;
            }
         }
      }
      if (bUpdateIterators) {
         itRootEnd = itBegin = s.begin();
         itEnd = s.end();
      }
   }
   if (bIsAbsolute) {
      // Skip the Win32 File Namespace, since we don’t want its double-backslashes to be collapsed
      // into one.
      itRootEnd += ABC_COUNTOF(smc_aszRoot) - 1 /*NUL*/;
   }
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error TODO-PORT: HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else

   // Collapse sequences of one or more path separators with a single separator.
   auto itDst(itRootEnd);
   bool bPrevIsSeparator(false);
   for (auto itSrc(itRootEnd); itSrc != itEnd; ++itSrc) {
      auto ch(*itSrc);
      bool bCurrIsSeparator(ch == smc_aszSeparator[0]);
      if (bCurrIsSeparator && bPrevIsSeparator) {
         // Collapse consecutive separators by advancing itSrc (as part of the for loop) without
         // advancing itDst.
         continue;
      }
      // Remember whether this character is a separator.
      bPrevIsSeparator = bCurrIsSeparator;
      // If the characer needs to be moved, move it.
      if (itDst != itSrc) {
         *itDst = ch;
      }
      ++itDst;
   }
   // If the last character written is a separator and it wouldn’t leave an empty string (other than
   // any prefix), move itDst back.
   if (bPrevIsSeparator && itDst > itRootEnd) {
      --itDst;
   }

   // Adjust the length based on the position of the last character written.
   s.set_size(size_t(itDst - itBegin));
   return std::move(s);
}


to_str_backend<file_path>::to_str_backend(char_range const & crFormat /*= char_range()*/) {
   ABC_TRACE_FN((this, crFormat));

   auto it(crFormat.cbegin());

   // TODO: parse the format string.

   // If we still have any characters, they are garbage.
   if (it != crFormat.cend()) {
      ABC_THROW(syntax_error, (
         SL("unexpected character"), crFormat, unsigned(it - crFormat.cbegin())
      ));
   }
}


void to_str_backend<file_path>::write(file_path const & fp, ostream * posOut) {
   ABC_TRACE_FN((this, fp, posOut));

   // TODO: apply format options.
   posOut->write(static_cast<istr const &>(fp));
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


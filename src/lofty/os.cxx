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
#include <lofty/io/text/str.hxx>
#include <lofty/os.hxx>
#include <lofty/os/path.hxx>
#if LOFTY_HOST_API_POSIX
   #include <errno.h> // _E*
#elif LOFTY_HOST_API_WIN32
   #include <lofty/text.hxx>
   #include <lofty/text/str.hxx>
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

/*explicit*/ invalid_path::invalid_path(class path const & path__, errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_WIN32
      ERROR_BAD_PATHNAME
#else
      0
#endif
   ),
   path_(path__) {
   what_ostream().print(LOFTY_SL("not a valid path=\"{}\""), path_);
}

invalid_path::invalid_path(invalid_path const & src) :
   generic_error(src),
   path_(src.path_) {
}

/*virtual*/ invalid_path::~invalid_path() LOFTY_STL_NOEXCEPT_TRUE() {
}

invalid_path & invalid_path::operator=(invalid_path const & src) {
   generic_error::operator=(src);
   path_ = src.path_;
   return *this;
}

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

/*explicit*/ path_not_found::path_not_found(class path const & path__, errint_t err_ /*= 0*/) :
   generic_error(err_ ? err_ :
#if LOFTY_HOST_API_POSIX
      ENOENT
#elif LOFTY_HOST_API_WIN32
      ERROR_PATH_NOT_FOUND
#else
      0
#endif
   ),
   path_(path__) {
   what_ostream().print(LOFTY_SL("path not found=\"{}\""), path_);
}

path_not_found::path_not_found(path_not_found const & src) :
   generic_error(src),
   path_(src.path_) {
}

/*virtual*/ path_not_found::~path_not_found() LOFTY_STL_NOEXCEPT_TRUE() {
}

path_not_found & path_not_found::operator=(path_not_found const & src) {
   generic_error::operator=(src);
   path_ = src.path_;
   return *this;
}

}} //namespace lofty::os

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace os { namespace registry {

key::key(::HKEY parent, text::str const & name) {
   // TODO: use Nt* functions to avoid the limitation of NUL termination.
   if (auto ret = ::RegOpenKeyEx(parent, name.c_str(), 0, KEY_QUERY_VALUE, &hkey)) {
      if (ret != ERROR_FILE_NOT_FOUND) {
         exception::throw_os_error(static_cast<errint_t>(ret));
      }
      hkey = nullptr;
   }
}

key::~key() {
   if (hkey) {
      ::RegCloseKey(hkey);
   }
}

bool key::get_value(text::str const & name, text::str * value) const {
   auto name_cstr(name.c_str());
   ::DWORD probed_type, probed_value_byte_size;
   if (!get_value_raw(name_cstr, &probed_type, nullptr, &probed_value_byte_size)) {
      value->clear();
      return false;
   }
   for (;;) {
      ::DWORD final_type, final_value_byte_size = probed_value_byte_size;
      std::size_t value_char_size = probed_value_byte_size / sizeof(text::char_t);
      switch (probed_type) {
         case REG_SZ:
            value->set_size_in_chars(value_char_size, false);
            if (!get_value_raw(name_cstr, &final_type, value->data(), &final_value_byte_size)) {
               // Race condition detected: somebody else deleted the value between our queries.
               value->clear();
               return false;
            }
            if (final_type != probed_type || final_value_byte_size != probed_value_byte_size) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If *value ended up including a NUL terminator because the value did, strip it; str doesn’t
            need it. */
            // TODO: use ends_with(text::char_t) when it becomes avaiable.
            if (value->ends_with(LOFTY_SL("\0"))) {
               value->set_size_in_chars(value_char_size - 1, false);
            }
            return true;
         case REG_EXPAND_SZ: {
            text::sstr<256> unexpanded;
            unexpanded.set_size_in_chars(value_char_size, false);
            if (!get_value_raw(name_cstr, &final_type, unexpanded.data(), &final_value_byte_size)) {
               // Race condition detected: somebody else deleted the value between our queries.
               value->clear();
               return false;
            }
            if (final_type != probed_type || final_value_byte_size != probed_value_byte_size) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If unexpanded ended up including a NUL terminator because the value did, strip it; str doesn’t
            need it. */
            // TODO: use ends_with(text::char_t) when it becomes avaiable.
            if (unexpanded.ends_with(LOFTY_SL("\0"))) {
               unexpanded.set_size_in_chars(value_char_size - 1, false);
            }
            // Expand any environment variables.
            auto unexpanded_cstr(unexpanded.c_str());
            value->set_from([&unexpanded_cstr] (text::char_t * chars, std::size_t chars_max) -> std::size_t {
               ::DWORD expanded_chars = ::ExpandEnvironmentStrings(
                  unexpanded_cstr, chars, static_cast< ::DWORD>(chars_max)
               );
               if (expanded_chars == 0) {
                  exception::throw_os_error();
               }
               return expanded_chars;
            });
            return true;
         }
         default:
            // TODO: use a better exception class.
            LOFTY_THROW(generic_error, ());
      }
      // Start over by switching to the new type and size.
      probed_type = final_type;
      probed_value_byte_size = final_value_byte_size;
   }
}

bool key::get_value(text::str const & name, collections::vector<text::str> * value) const {
   value->clear();
   auto name_cstr(name.c_str());
   ::DWORD probed_type, probed_value_byte_size;
   if (!get_value_raw(name_cstr, &probed_type, nullptr, &probed_value_byte_size)) {
      return false;
   }
   text::str multi_value;
   for (;;) {
      if (probed_type != REG_MULTI_SZ) {
         // TODO: use a better exception class.
         LOFTY_THROW(generic_error, ());
      }
      ::DWORD final_type, final_value_byte_size = probed_value_byte_size;
      std::size_t value_char_size = probed_value_byte_size / sizeof(text::char_t);
      multi_value.set_size_in_chars(value_char_size, false);
      if (!get_value_raw(name_cstr, &final_type, multi_value.data(), &final_value_byte_size)) {
         // Race condition detected: somebody else deleted the value between our queries.
         return false;
      }
      if (final_type == probed_type && final_value_byte_size == probed_value_byte_size) {
         break;
      }
      // Race condition detected: somebody else changed the value between our queries.
      // Start over by switching to the new type and size.
      probed_type = final_type;
      probed_value_byte_size = final_value_byte_size;
   }
   // Break up the multi-string into an array of strings.
   auto final_nul(multi_value.cend());
   // TODO: use ends_with(char_t) when it becomes avaiable.
   if (multi_value.ends_with(LOFTY_SL("\0"))) {
      --final_nul;
   }
   for (
      text::str::const_iterator prev_nul(multi_value.cbegin()), next_nul;
      (next_nul = multi_value.find('\0', prev_nul)) < final_nul;
      prev_nul = next_nul + 1 /*NUL*/
   ) {
      value->push_back(multi_value.substr(prev_nul, next_nul));
   }
   return true;
}

bool key::get_value_raw(
   text::char_t const * name, ::DWORD * type, void * value, ::DWORD * value_byte_size
) const {
   // TODO: use Nt* functions to avoid the limitation of NUL termination.
   if (auto ret = ::RegQueryValueEx(
      hkey, name, nullptr, type, static_cast< ::BYTE *>(value), value_byte_size
   )) {
      if (ret == ERROR_FILE_NOT_FOUND) {
         return false;
      } else {
         exception::throw_os_error(static_cast<errint_t>(ret));
      }
   }
   return true;
}

}}} //namespace lofty::os::registry

#endif

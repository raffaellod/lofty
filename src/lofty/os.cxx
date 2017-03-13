/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General
Public License as published by the Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License along with Lofty. If not, see
<http://www.gnu.org/licenses/>.
------------------------------------------------------------------------------------------------------------*/

#include <lofty.hxx>
#include <lofty/defer_to_scope_end.hxx>
#include <lofty/os.hxx>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace os {

/*explicit*/ invalid_path::invalid_path(os::path const & path__, errint_t err_ /*= 0*/) :
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

/*explicit*/ path_not_found::path_not_found(os::path const & path__, errint_t err_ /*= 0*/) :
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

namespace lofty { namespace os {

#if LOFTY_HOST_API_WIN32

static ::HKEY open_registry_key(::HKEY parent_hkey, str const & name) {
   ::HKEY ret_hkey;
   if (auto ret = ::RegOpenKeyEx(parent_hkey, name.c_str(), 0, KEY_QUERY_VALUE, &ret_hkey)) {
      if (ret == ERROR_FILE_NOT_FOUND) {
         return nullptr;
      }
      exception::throw_os_error(static_cast<errint_t>(ret));
   }
   return ret_hkey;
}

static bool get_registry_value_raw(
   ::HKEY hkey, char_t const * name, ::DWORD * type, void * value, ::DWORD * value_byte_size
) {
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

bool get_registry_value(::HKEY parent_hkey, str const & key_path, str const & name, str * out) {
   ::HKEY hkey = open_registry_key(parent_hkey, key_path);
   if (!hkey) {
      out->clear();
      return false;
   }
   LOFTY_DEFER_TO_SCOPE_END(::RegCloseKey(hkey));
   // TODO: use Nt* functions to avoid the limitation of NUL termination.
   auto name_cstr(name.c_str());
   ::DWORD probed_type, probed_value_byte_size;
   if (!get_registry_value_raw(hkey, name_cstr, &probed_type, nullptr, &probed_value_byte_size)) {
      out->clear();
      return false;
   }
   for (;;) {
      ::DWORD final_type, final_value_byte_size = probed_value_byte_size;
      std::size_t value_char_size = probed_value_byte_size / sizeof(char_t);
      switch (probed_type) {
         case REG_SZ:
            out->set_size_in_chars(value_char_size, false);
            if (!get_registry_value_raw(hkey, name_cstr, &final_type, out->data(), &final_value_byte_size)) {
               // Race condition detected: somebody else deleted the value between our queries.
               out->clear();
               return false;
            }
            if (final_type != probed_type || final_value_byte_size != probed_value_byte_size) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If *out ended up including a NUL terminator because the value did, strip it; str doesn’t need
            it. */
            // TODO: use ends_with(char_t) when it becomes avaiable.
            if (out->ends_with(LOFTY_SL("\0"))) {
               out->set_size_in_chars(value_char_size - 1, false);
            }
            return true;
         case REG_EXPAND_SZ: {
            text::sstr<256> unexpanded;
            unexpanded.set_size_in_chars(value_char_size, false);
            if (!get_registry_value_raw(
               hkey, name_cstr, &final_type, unexpanded.data(), &final_value_byte_size
            )) {
               // Race condition detected: somebody else deleted the value between our queries.
               out->clear();
               return false;
            }
            if (final_type != probed_type || final_value_byte_size != probed_value_byte_size) {
               // Race condition detected: somebody else changed the value between our queries.
               break;
            }
            /* If unexpanded ended up including a NUL terminator because the value did, strip it; str doesn’t
            need it. */
            // TODO: use ends_with(char_t) when it becomes avaiable.
            if (unexpanded.ends_with(LOFTY_SL("\0"))) {
               unexpanded.set_size_in_chars(value_char_size - 1, false);
            }
            // Expand any environment variables.
            auto unexpanded_cstr(unexpanded.c_str());
            out->set_from([&unexpanded_cstr] (char_t * chars, std::size_t chars_max) -> std::size_t {
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

#endif

}} //namespace lofty::os

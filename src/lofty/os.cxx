/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
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

#if LOFTY_HOST_API_WIN32

namespace lofty { namespace os { namespace registry {

key::key(::HKEY parent, str const & name) {
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

bool key::get_value(str const & name, str * value) {
   auto name_cstr(name.c_str());
   ::DWORD probed_type, probed_value_byte_size;
   if (!get_value_raw(name_cstr, &probed_type, nullptr, &probed_value_byte_size)) {
      value->clear();
      return false;
   }
   for (;;) {
      ::DWORD final_type, final_value_byte_size = probed_value_byte_size;
      std::size_t value_char_size = probed_value_byte_size / sizeof(char_t);
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
            // TODO: use ends_with(char_t) when it becomes avaiable.
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
            // TODO: use ends_with(char_t) when it becomes avaiable.
            if (unexpanded.ends_with(LOFTY_SL("\0"))) {
               unexpanded.set_size_in_chars(value_char_size - 1, false);
            }
            // Expand any environment variables.
            auto unexpanded_cstr(unexpanded.c_str());
            value->set_from([&unexpanded_cstr] (char_t * chars, std::size_t chars_max) -> std::size_t {
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

bool key::get_value_raw(char_t const * name, ::DWORD * type, void * value, ::DWORD * value_byte_size) const {
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

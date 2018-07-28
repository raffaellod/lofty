/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2014-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#include <lofty/coroutine.hxx>
#include <lofty/exception.hxx>
#include <lofty/io/text.hxx>
#include <lofty/process.hxx>
#include <lofty/_std/tuple.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text.hxx>
#include <lofty/text/str.hxx>
#include <lofty/thread.hxx>
#include <lofty/to_text_ostream.hxx>
#if LOFTY_HOST_API_POSIX
   #include <cstdlib> // std::getenv()
   #include <errno.h> // EINVAL errno
   #include <sys/types.h> // id_t pid_t
   #include <sys/wait.h> // waitid() waitpid() W*
   #include <unistd.h> // getpid()
   #if LOFTY_HOST_API_BSD
      #include <sys/signal.h> // siginfo_t
   #endif
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

process::native_handle_type const process::null_handle =
#if LOFTY_HOST_API_POSIX
   0;
#elif LOFTY_HOST_API_WIN32
   nullptr;
#else
   #error "TODO: HOST_API"
#endif

/*explicit*/ process::process(id_type pid) :
#if LOFTY_HOST_API_POSIX
   // ID == native handle.
   handle(pid) {
   static_assert(sizeof(id_type) == sizeof(::pid_t), "pid_t must be the same size as native_handle_type");
#elif LOFTY_HOST_API_WIN32
   handle(null_handle) {
   // For now, only get a minimum access level.
   handle = ::OpenProcess(SYNCHRONIZE, false, pid);
   if (!handle) {
      exception::throw_os_error();
   }
#else
   #error "TODO: HOST_API"
#endif
}

process::~process() {
   if (joinable()) {
      // TODO: std::abort() or something similar.
   }
#if LOFTY_HOST_API_WIN32
   if (handle) {
      ::CloseHandle(handle);
   }
#endif
}

void process::detach() {
#if LOFTY_HOST_API_WIN32
   if (handle) {
      ::CloseHandle(handle);
   }
#endif
   handle = null_handle;
}

process::id_type process::id() const {
#if LOFTY_HOST_API_POSIX
   // ID == native handle.
   return handle;
#elif LOFTY_HOST_API_WIN32
   ::DWORD pid = ::GetProcessId(handle);
   if (pid == 0) {
      exception::throw_os_error();
   }
   return pid;
#else
   #error "TODO: HOST_API"
#endif
}

int process::join() {
   // TODO: wait using coroutine::scheduler.
#if LOFTY_HOST_API_POSIX
   int status;
   while (::waitpid(static_cast< ::pid_t>(handle), &status, 0) != static_cast< ::pid_t>(handle)) {
      int err = errno;
      if (err != EINTR) {
         exception::throw_os_error(err);
      }
      this_coroutine::interruption_point();
   }
   this_coroutine::interruption_point();
   if (WIFEXITED(status)) {
      return WEXITSTATUS(status);
   } else if (WIFSIGNALED(status)) {
      return -WTERMSIG(status);
   } else {
      // Should never happen.
      return -1;
   }
#elif LOFTY_HOST_API_WIN32
   this_thread::interruptible_wait_for_single_object(handle, 0);
   this_coroutine::interruption_point();
   ::DWORD exit_code;
   if (!::GetExitCodeProcess(handle, &exit_code)) {
      exception::throw_os_error();
   }
   return static_cast<int>(exit_code);
#else
   #error "TODO: HOST_API"
#endif
}

bool process::joinable() const {
   if (handle == null_handle) {
      return false;
   }
#if LOFTY_HOST_API_POSIX
   ::siginfo_t si;
   /* waitid() will not touch this field if handle is not in a waitable (“joinable”) state, so we have to in
   order to check it after the call. */
   si.si_pid = 0;
   if (::waitid(P_PID, static_cast< ::id_t>(handle), &si, WEXITED | WNOHANG | WNOWAIT)) {
      exception::throw_os_error();
   }
   // waitid() sets this to handle if the child is in the requested state (WEXITED).
   return si.si_pid != 0;
#elif LOFTY_HOST_API_WIN32
   ::DWORD ret = ::WaitForSingleObject(handle, 0);
   if (ret == WAIT_FAILED) {
      exception::throw_os_error();
   }
   return ret == WAIT_TIMEOUT;
#else
   #error "TODO: HOST_API"
#endif
}

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

void to_text_ostream<process>::set_format(text::str const & format) {
   auto itr(format.cbegin());

   // Add parsing of the format string here.

   throw_on_unused_streaming_format_chars(itr, format);
}

void to_text_ostream<process>::write(process const & src, io::text::ostream * dst) {
   dst->write(LOFTY_SL("TID:"));
   if (auto id = src.id()) {
      to_text_ostream<decltype(id)>::write(id, dst);
   } else {
      dst->write(LOFTY_SL("-"));
   }
}

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace this_process { namespace _pub {

bool env_var(text::str const & name, text::str * out) {
   bool ret;
   auto name_cstr(name.c_str());
#if LOFTY_HOST_API_POSIX
   if (text::char_t const * value = std::getenv(name_cstr)) {
      /* Environment strings are to be considered stored in non-modifiable memory, so we can just adopt value
      as external buffer. */
      *out = text::str(external_buffer, value);
      ret = true;
   } else {
      out->clear();
      ret = false;
   }
#elif LOFTY_HOST_API_WIN32 //if LOFTY_HOST_API_POSIX
   out->set_from([&name_cstr, &ret] (text::char_t * chars, std::size_t chars_max) -> std::size_t {
      /* ::GetEnvironmentVariable() returns < chars_max (length without NUL) if the buffer was large enough,
      or the required size (length including NUL) otherwise. */
      ::DWORD chars_used = ::GetEnvironmentVariable(name_cstr, chars, static_cast< ::DWORD>(chars_max));
      // No other ::GetLastError() values are documented to be returned (other than 0, presumably).
      ret = (chars_used > 0 || ::GetLastError() != ERROR_ENVVAR_NOT_FOUND);
      return chars_used;
   });
#else
   #error "TODO: HOST_API"
#endif
   return ret;
}

_std::tuple<text::str, bool> env_var(text::str const & name) {
   _std::tuple<text::str, bool> ret;
   _std::get<1>(ret) = env_var(name, &_std::get<0>(ret));
   return _std::move(ret);
}

process::id_type id() {
#if LOFTY_HOST_API_POSIX
   return ::getpid();
#elif LOFTY_HOST_API_WIN32
   return ::GetCurrentProcessId();
#else
   #error "TODO: HOST_API"
#endif
}

}}} //namespace lofty::this_process::_pub

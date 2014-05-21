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
#include <abc/io/text/file.hxx>
#include <abc/io/binary/file.hxx>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text globals


namespace abc {
namespace io {
namespace text {

static std::shared_ptr<binbuf_writer> g_ptwStdErr;
static std::shared_ptr<binbuf_reader> g_ptrStdIn;
static std::shared_ptr<binbuf_writer> g_ptwStdOut;


/** Instantiates a text::base specialization appropriate for the specified binary I/O object,
returning a shared pointer to it. If the binary I/O object does not implement buffering, a buffered
I/O wrapper is instanciated as well.

pbb
   Pointer to a binary I/O object.
return
   Shared pointer to the newly created object.
*/
static std::shared_ptr<binbuf_base> _construct(
   std::shared_ptr<binary::base> pbb, abc::text::encoding enc, abc::text::line_terminator lterm
) {
   ABC_TRACE_FN((/*pbb, */enc, lterm));

   // Choose what type of text I/O object to create based on what type of binary I/O object we got.

   // Check if it’s a buffered I/O object.
   auto pbbr(std::dynamic_pointer_cast<binary::buffered_reader>(pbb));
   auto pbbw(std::dynamic_pointer_cast<binary::buffered_writer>(pbb));
   if (!pbbr && !pbbw) {
      // Not a buffered I/O object? Get one then, and try again with the casts.
      auto pbbb(binary::buffer(pbb));
      pbbr = std::dynamic_pointer_cast<binary::buffered_reader>(pbbb);
      pbbw = std::dynamic_pointer_cast<binary::buffered_writer>(pbbb);
   }

   // Now we must have a buffered reader or writer, or pbb is not something we can use.
   if (pbbr) {
      return std::make_shared<binbuf_reader>(std::move(pbbr), enc, lterm);
   }
   if (pbbw) {
      return std::make_shared<binbuf_writer>(std::move(pbbw), enc, lterm);
   }
   // TODO: use a better exception class.
   ABC_THROW(argument_error, ());
}


/** Detects the encoding to use for a standard text I/O file, with the help of an optional
environment variable.

TODO: document this behavior and the related enviroment variables.

TODO: change to use a global “environment” map object instead of this ad-hoc code.

TODO: make the below code only pick up variables meant for this PID. This should eventually be made
more general, as a way for an ABC-based parent process to communicate with an ABC-based child
process. Thought maybe a better way is to pass a command-line argument that triggers ABC-specific
behavior, so that it’s inherently PID-specific.

pszEnvVarName
   Environment variable name that, if set, specifies the encoding to be used.
return
   Encoding appropriate for the requested standard I/O file.
*/
static std::shared_ptr<binbuf_base> _construct_stdio(
   std::shared_ptr<binary::base> pbb, char_t const * pszEnvVarName
) {
   ABC_TRACE_FN((/*pbb, */pszEnvVarName));

#if ABC_HOST_API_POSIX
   istr sEnc;
   if (char_t const * pszEnvVarValue = ::getenv(pszEnvVarName)) {
      sEnc = istr(unsafe, pszEnvVarValue);
   }
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
   smstr<64> sEnc;
   sEnc.grow_for([pszEnvVarName] (char_t * pch, size_t cchMax) -> size_t {
      // ::GetEnvironmentVariable() returns < cchMax (length without NUL) if the buffer was large
      // enough, or the required size (length including NUL) otherwise.
      return ::GetEnvironmentVariable(pszEnvVarName, pch, DWORD(cchMax));
   });
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
   #error HOST_API
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
   abc::text::encoding enc(abc::text::encoding::unknown);
   if (sEnc) {
      try {
         enc = abc::text::encoding(sEnc);
      } catch (domain_error const &) {
         // Ignore this invalid encoding setting, and default to auto-detection.
         // TODO: display a warning about ABC_STDERR_ENCODING being ignored.
      }
   }
   return _construct(std::move(pbb), enc, abc::text::line_terminator::unknown);
}


std::shared_ptr<binbuf_writer> stderr() {
   ABC_TRACE_FN(());

   // TODO: mutex!
   if (!g_ptwStdErr) {
      g_ptwStdErr = std::dynamic_pointer_cast<binbuf_writer>(
         _construct_stdio(binary::stderr(), SL("ABC_STDERR_ENCODING"))
      );
   }
   return g_ptwStdErr;
}


std::shared_ptr<binbuf_reader> stdin() {
   ABC_TRACE_FN(());

   // TODO: mutex!
   if (!g_ptrStdIn) {
      g_ptrStdIn = std::dynamic_pointer_cast<binbuf_reader>(
         _construct_stdio(binary::stdin(), SL("ABC_STDIN_ENCODING"))
      );
   }
   return g_ptrStdIn;
}


std::shared_ptr<binbuf_writer> stdout() {
   ABC_TRACE_FN(());

   // TODO: mutex!
   if (!g_ptwStdOut) {
      g_ptwStdOut = std::dynamic_pointer_cast<binbuf_writer>(
         _construct_stdio(binary::stdout(), SL("ABC_STDOUT_ENCODING"))
      );
   }
   return g_ptwStdOut;
}


std::shared_ptr<binbuf_base> open(
   file_path const & fp, access_mode am, abc::text::encoding enc /*= abc::text::encoding::unknown*/,
   abc::text::line_terminator lterm /*= abc::text::line_terminator::unknown*/
) {
   ABC_TRACE_FN((fp, am, enc, lterm));

   return _construct(binary::open(fp, am), enc, lterm);
}

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


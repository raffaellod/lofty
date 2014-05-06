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

#ifndef ABC_FILE_IOSTREAM_HXX
#define ABC_FILE_IOSTREAM_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
   #pragma once
#endif

#include <abc/iostream.hxx>
#include <abc/numeric.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_stream_base


namespace abc {

namespace io {

/** Base class for file-based data streams.
*/
class ABCAPI file_stream_base :
   public virtual stream_base {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit file_stream_base(std::shared_ptr<file> pfile);
   file_stream_base(file_path const & fp, access_mode am, bool bBuffered = true);


   /** Destructor.
   */
   virtual ~file_stream_base();


protected:

   /** Releases any objects constructed by file_?stream::_construct_std_file_?stream().
   */
   static void ABC_STL_CALLCONV _release_std_file_streams();


protected:

   /** Underlying file. */
   std::shared_ptr<file> m_pfile;
   /** Maximum size_t value rounded so that will not cause immediate rejection of UTF-16 and UTF-32
   by text::guess_encoding() due to not being an integer multiple of the character size. */
   static size_t const smc_cbAlignedMax;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_istream


namespace abc {

namespace io {

/** DOC:0674 abc::io::file_istream buffering

There are a few conditions in which file_istream will be forced to use a read buffer (m_pbReadBuf),
instead of a memory map, for its read methods:

1. Reusing bytes previously pushed back with istream::unread_raw().
2. Reading from a non-disk file (pipe, socket, …).
3. Any time memmap() (or equivalent) fails.

The read buffer is maintained with these usage constraints:

1. In case of unbuffered disk access, a fixed-size, physical disk sector-aligned position in the
   buffer must be ready at all times to accept a new block of bytes from the disk: the pointer
   provided to the OS read() (or equivalent) API function must be aligned to the physical disk
   sector, and the size of the pointed buffer must be a multiple of that same size (as required by
   at least Win32).

   This can create a problem if the buffer is almost emptied, except for a few bytes that are the
   beginning of a multi-byte character, that text::transcode() will therefore not remove from the
   buffer:

   ┌─────────────────┐   The read buffer has just been filled via an OS read() API call.
   │ a b c d e f g h │
   └─────────────────┘

   ┌─────────────────┐   The client read 7 bytes, and the remaining «h» byte is part of a multi-byte
   │               h │   sequence.
   └─────────────────┘

   At this point, the only way to unlock this situation would be to temporarily take «h» out of the
   buffer, refill the buffer, take from it as many bytes as are necessary to complete the character
   it belongs to, and then resume transcoding the remaining bytes. If it sounds complicated in
   words, the code will be even worse.

   A much faster approach, also requiring far less code, consists in keeping an empty area before
   the aligned buffer, where to move any remaining bytes from a previous buffer fill:

   ┌───────────────────────────┐   The read buffer has just been filled via an OS read() API call.
   │           a b c d e f g h │
   └───────────────────────────┘

   ┌───────────────────────────┐   The client read 7 bytes, and the remaining «h» byte is part of a
   │                         h │   multi-byte sequence.
   └───────────────────────────┘  

   ┌───────────────────────────┐   To accommodate a buffer refill, the remaining bytes are moved to
   │         h                 │   the last position before the aligned pointer that will be passed
   └───────────────────────────┘   to the OS read() API call.

   ┌───────────────────────────┐   The new bytes are read. Now «h» is correctly followed by the
   │         h i j k l m n o p │   other bytes along with which it constitutes a character.
   └───────────────────────────┘

2. Issues due to a buffer refill immediately followed by an unread_raw() call:

   ┌─────────────────┐   The read buffer has just been filled via an OS read() API call.
   │ a b c d e f g h │
   └─────────────────┘

   ┌─────────────────┐   The client reads 8 bytes, therefore emptying the read buffer.
   │                 │
   └─────────────────┘

   ┌─────────────────┐   Upon a new client request, the buffer is filled again via an API call.
   │ i j k l m n o q │
   └─────────────────┘

   ┌─────────────────┐   The client actually only wanted 2 bytes; the rest is kept in the buffer.
   │     k l m n o q │
   └─────────────────┘

   At this point, what happens if the client demands to unread the last 3 bytes? It would be
   unreasonable to fail such a request, especially considering that such a small number of bytes
   could be just a single multi-byte character.

   A solution could be to use a separated unread buffer:

   ┌─────────────────┐   The bytes unread are put in the unread buffer, while the regular read
   │ h i j           │   buffer is unaffected.
   ├─────────────────┤
   │     k l m n o q │
   └─────────────────┘

   What happens now if the client wants to re-read these bytes in a different encoding, say UTF-32
   (perhaps the first few bytes of the file indicated that the encoding was different than the
   apparent one), so that the first character actually comprises the bytes «h i j k»?
   text::transcode() can only work on a single buffer, and if invoked on the unread buffer, it
   would’t transcode anything, since there is really not a single UTF-32 character in it.

   To avoid this last issue, the unread buffer is simply a portion of the read buffer. So, resuming
   from the last buffer refill:

   ┌───────────────────────────┐   Upon a new client request, the buffer is filled again via an API
   │           i j k l m n o q │   call.
   └───────────────────────────┘

   ┌───────────────────────────┐   The client actually only wanted 2 bytes; the rest is kept in the
   │               k l m n o q │   buffer.
   └───────────────────────────┘

   ┌───────────────────────────┐   The bytes unread are put back in the read buffer, immediately
   │         h i j k l m n o q │   before any bytes already there.
   └───────────────────────────┘

   This allows to make the unread bytes no special case, since they will be processed just like any
   other bytes in the read buffer.

To satisfy these constraints, the read buffer is actually only a portion of the larger memory block
allocated for it, so that there is a memory chunk immediately preceding the read buffer, ready for
use in the above scenarios.

A good way to test that the code handling the read buffer behaves correctly, is to artificially
reduce the read buffer size to ludicrous values, and check that exceptions are consciously thrown,
instead of getting a segfault or memory corruption, in file_istream tests.
*/

/** Implementation of an read-only stream based on a file.
*/
class ABCAPI file_istream :
   public virtual file_stream_base,
   public virtual istream {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit file_istream(std::shared_ptr<file> pfile);
   explicit file_istream(file_path const & fp);


   /** Destructor.
   */
   virtual ~file_istream();


   /** See istream::at_end().
   */
   virtual bool at_end() const;


   /** See istream::read_raw().
   */
   virtual size_t read_raw(void * p, size_t cbMax, text::encoding enc = text::encoding::identity);


   /** Returns the stream associated to the standard input (stdin).

   return
      Standard input stream.
   */
   static std::shared_ptr<file_istream> const & stdin();


   /** See istream::unread_raw().
   */
   virtual void unread_raw(
      void const * p, size_t cb, text::encoding enc = text::encoding::identity
   );


private:

   /** Allocates m_pbReadBuf if it hasn’t been yet, and returns it as a simple byte pointer.

   return
      Pointer to the internal read buffer, of size m_cbReadBufBulk bytes.
   */
   int8_t * _get_read_buffer();


   /** Initializes a standard input stream.

   TODO: comment signature.
   */
   static void _construct_std_file_istream(
      std::shared_ptr<file> const & pfile, std::shared_ptr<file_istream> ** pppfis
   );


   /** Code common to all constructors.
   */
   void _post_construct();


   /** See istream::_read_line().
   */
   virtual void _read_line(
      _raw_str * prs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
   );


private:

   /** Read buffer. Allocated only if necessary; see [DOC:0674 abc::io::file_istream buffering]. */
   std::unique_ptr<int8_t[]> m_pbReadBuf;
   /** Offset of the first used byte in m_pbReadBuf. */
   size_t m_ibReadBufUsed;
   /** Number of bytes used in m_pbReadBuf. */
   size_t m_cbReadBufUsed;
   /** Size of the multi-purpose area preceding the read buffer. */
   size_t m_cbReadBufLead;
   /** Size of the actual read buffer. */
   size_t m_cbReadBufBulk;
   /** Line buffer size (initial and increment). */
   size_t m_cchBufferStep;
   /** true if the last read resulted in the seek offset to be at the end of the file. */
   bool m_bAtEof;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_ostream


namespace abc {

namespace io {

/** Implementation of an write-only stream based on a file.
*/
class ABCAPI file_ostream :
   public virtual file_stream_base,
   public virtual ostream {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit file_ostream(std::shared_ptr<file> pfile);
   explicit file_ostream(file_path const & fp);


   /** Destructor.
   */
   virtual ~file_ostream();


   /** See ostream::flush().
   */
   virtual void flush();


   /** Returns the stream associated to the standard error output (stderr).

   return
      Standard error output stream.
   */
   static std::shared_ptr<file_ostream> const & stderr();


   /** Returns the stream associated to the standard output (stdout).

   return
      Standard output stream.
   */
   static std::shared_ptr<file_ostream> const & stdout();


   /** See ostream::write_raw().
   */
   virtual void write_raw(void const * p, size_t cb, text::encoding enc = text::encoding::identity);


private:

   /** Initializes a standard output stream.

   TODO: comment signature.
   */
   static void _construct_std_file_ostream(
      std::shared_ptr<file> const & pfile, std::shared_ptr<file_ostream> ** pppfos
   );


private:

   /** Write buffer. */
   std::unique_ptr<int8_t[]> m_pbWriteBuf;
   /** Maximum number of bytes the write buffer can hold. */
   static size_t const smc_cbWriteBufMax;
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::file_iostream


namespace abc {

namespace io {

/** Implementation of a read/write stream based on a file.
*/
class ABCAPI file_iostream :
   public virtual file_istream,
   public virtual file_ostream {
public:

   /** Constructor.

   TODO: comment signature.
   */
   explicit file_iostream(std::shared_ptr<file> pfile);
   explicit file_iostream(file_path const & fp);


   /** Destructor.
   */
   virtual ~file_iostream();
};

} //namespace io

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_FILE_IOSTREAM_HXX


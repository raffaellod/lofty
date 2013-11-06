/* -*- coding: utf-8; mode: c++; tab-width: 3 -*-

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

#include <abc/core.hxx>
#include <abc/file_iostream.hxx>
#include <abc/trace.hxx>
#include <algorithm>
#if ABC_HOST_API_POSIX
	#include <stdlib.h> // atexit()
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_stream_base


namespace abc {

// These should be members of file_stream_base, but that’s not possible.
static std::shared_ptr<file_ostream> * g_ppfosStdErr(NULL);
static std::shared_ptr<file_istream> * g_ppfisStdIn(NULL);
static std::shared_ptr<file_ostream> * g_ppfosStdOut(NULL);

size_t const file_stream_base::smc_cbAlignedMax =
	numeric::max<size_t>::value & ~(sizeof(char32_t) - 1);



file_stream_base::file_stream_base(std::shared_ptr<file> pfile) :
	stream_base(),
	m_pfile(std::move(pfile)) {
}
file_stream_base::file_stream_base(
	file_path const & fp, file::access_mode fam, bool bBuffered /*= true*/
) :
	stream_base(),
	m_pfile(file::open(fp, fam, bBuffered)) {
}


/*virtual*/ file_stream_base::~file_stream_base() {
}


/*static*/ void ABC_STL_CALLCONV file_stream_base::_release_std_file_streams() {
	// TODO: mutex!
	// Destruct the shared pointers, which will allow the files to be released if these were the last
	// strong references to them.
	if (g_ppfosStdErr) {
		delete g_ppfosStdErr;
		g_ppfosStdErr = NULL;
	}
	if (g_ppfisStdIn) {
		delete g_ppfisStdIn;
		g_ppfisStdIn = NULL;
	}
	if (g_ppfosStdOut) {
		delete g_ppfosStdOut;
		g_ppfosStdOut = NULL;
	}
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_istream


namespace abc {

file_istream::file_istream(std::shared_ptr<file> pfile) :
	file_stream_base(std::move(pfile)),
	istream() {
	_post_construct();
}
file_istream::file_istream(file_path const & fp) :
	file_stream_base(fp, file::access_mode::read),
	istream() {
	_post_construct();
}


/*virtual*/ file_istream::~file_istream() {
}


/*virtual*/ bool file_istream::at_end() const {
	return m_bAtEof;
}


/*virtual*/ size_t file_istream::read_raw(
	void * p, size_t cbMax, text::encoding enc /*= text::encoding::identity*/
) {
	ABC_TRACE_FN((this, p, cbMax, enc));

	if (m_enc == text::encoding::unknown) {
		// If the encoding is still undefined, try to guess it now. To have a big enough buffer, we’ll
		// just use the regular read buffer instead of the (possibly too small) provided p.
		void * pRawReadBuf(_get_read_buffer() + m_ibReadBufUsed);
		// Since nobody set m_enc yet, the buffer must have never been used.
		ABC_ASSERT(!m_cbReadBufUsed);
		m_cbReadBufUsed = m_pfile->read(
			pRawReadBuf, m_cbReadBufLead + m_cbReadBufBulk - m_ibReadBufUsed
		);
		if (!m_cbReadBufUsed) {
			m_bAtEof = true;
		}
		size_t cbBom;
		m_enc = text::guess_encoding(
			pRawReadBuf,
			m_cbReadBufUsed,
			m_pfile->has_size() ? size_t(std::min<fileint_t>(m_pfile->size(), smc_cbAlignedMax)) : 0,
			&cbBom
		);
		if (cbBom) {
			// A BOM was read: discard it.
			m_ibReadBufUsed += cbBom;
			m_cbReadBufUsed -= cbBom;
		} else if (m_enc == text::encoding::unknown) {
			// Since no encoding was detected, we’ll just do nothing to transcode the input.
			m_enc = text::encoding::identity;
		}
	}
	size_t cbTotalRead;
	if (enc == m_enc || enc == text::encoding::identity) {
		// Optimal case: no transcoding necessary.
		int8_t * pb(static_cast<int8_t *>(p));
		// Check if there are available read bytes in the read buffer; if so, use them first.
		cbTotalRead = std::min(m_cbReadBufUsed, cbMax);
		if (cbTotalRead) {
			memory::copy(pb, _get_read_buffer() + m_ibReadBufUsed, cbTotalRead);
			pb += cbTotalRead;
			cbMax -= cbTotalRead;
			m_ibReadBufUsed += cbTotalRead;
			m_cbReadBufUsed -= cbTotalRead;
		}
		// Check if we need more bytes than that.
		if (cbMax) {
			size_t cbRead(m_pfile->read(pb, cbMax));
			if (!cbRead) {
				m_bAtEof = true;
			}
			cbTotalRead += cbRead;
		}
	} else {
		size_t cbAvail(cbMax);
		int8_t * pbReadBuf(_get_read_buffer());
		int8_t * pbRawReadBuf(pbReadBuf + m_ibReadBufUsed);
		for (;;) {
			// Transcode the buffer, from the start of the bytes already in the read buffer. This also
			// allows to re-read bytes that have been unread.
			void const * pBufUsed(pbReadBuf + m_ibReadBufUsed);
			text::transcode(std::nothrow, m_enc, &pBufUsed, &m_cbReadBufUsed, enc, &p, &cbAvail);
			m_ibReadBufUsed = size_t(static_cast<int8_t const *>(pBufUsed) - pbReadBuf);
			if (!cbAvail || m_bAtEof) {
				break;
			}
			// Make sure that the beginning of the free portion of the read buffer is exactly in its
			// middle, so that we can provide m_pfile->read() an aligned buffer.
			if (m_ibReadBufUsed + m_cbReadBufUsed != m_cbReadBufLead) {
				if (m_cbReadBufUsed /*&& m_cbReadBufLead >= m_cbReadBufUsed*/) {
					memory::move(
						pbReadBuf + m_cbReadBufLead - m_cbReadBufUsed,
						pbReadBuf + m_ibReadBufUsed,
						m_cbReadBufUsed
					);
				}
				m_ibReadBufUsed = m_cbReadBufLead - m_cbReadBufUsed;
				pbRawReadBuf = pbReadBuf + m_ibReadBufUsed;
			}
			// Read as many bytes as possible into the second half of the double-size buffer.
			size_t cbRead(m_pfile->read(
				pbRawReadBuf, m_cbReadBufLead + m_cbReadBufBulk - m_ibReadBufUsed
			));
			if (!cbRead) {
				m_bAtEof = true;
			}
			m_cbReadBufUsed += cbRead;
		}
		cbTotalRead = cbMax - cbAvail;
	}
	// If we got to EOF but managed to read something first, it must be because we called
	// m_pfile->read() once more than we should have. For now, just put it off; the next 0-length
	// read by m_pfile->read() will re-set it, and since that time we’ll actually have read 0 bytes,
	// it will stay.
	if (cbTotalRead && m_bAtEof) {
		m_bAtEof = false;
	}
	return cbTotalRead;
}


/*static*/ std::shared_ptr<file_istream> const & file_istream::stdin() {
	ABC_TRACE_FN(());

	if (!g_ppfisStdIn) {
		_construct_std_file_istream(file::stdin(), &g_ppfisStdIn);
	}
	return *g_ppfisStdIn;
}


/*virtual*/ void file_istream::unread_raw(void const * p, size_t cb, text::encoding enc) {
	ABC_TRACE_FN((this, p, cb, enc));

	if (enc == text::encoding::unknown) {
		// Treat unknown as identity.
		enc = text::encoding::identity;
	}
	// This must have been set by a preceding call to read_raw().
	ABC_ASSERT(m_enc != text::encoding::unknown);
	int8_t * pbReadBuf(_get_read_buffer());
	if (enc == m_enc || enc == text::encoding::identity) {
		// Optimal case: no transcoding necessary.
		if (!m_cbReadBufUsed) {
			// No buffer space in use, so align this write to the middle of the buffer, or as close to
			// it as possible.
			m_ibReadBufUsed = size_t(std::max<ptrdiff_t>(
				ptrdiff_t(m_cbReadBufLead) - ptrdiff_t(cb), 0
			));
		} else if (cb > m_ibReadBufUsed) {
			// Trying to unread more bytes than can fit in the gap before the currently used portion of
			// the read buffer: move the used portion first.
			size_t ibNew(m_cbReadBufLead + m_cbReadBufBulk - m_cbReadBufUsed);
			memory::move(pbReadBuf + ibNew, pbReadBuf + m_ibReadBufUsed, m_cbReadBufUsed);
			m_ibReadBufUsed = ibNew;
		}
		if (cb > m_ibReadBufUsed) {
			// Can’t unread more bytes than the read buffer can take.
			ABC_THROW(buffer_error, ());
		}
		// Copy to the read buffer, before the current start.
		memory::copy<void>(pbReadBuf + m_ibReadBufUsed - cb, p, cb);
		m_ibReadBufUsed -= cb;
		m_cbReadBufUsed += cb;
	} else {
		// Transcoding necessary. This is really non-optimal, since we probably already transcoded
		// these bytes once, and now have to transcode them back, and probably we’ll need to transcode
		// them once more on the next call to read*().
		// Since it’s impossible to know beforehand how many bytes will be necessary, use the read
		// buffer as a temporary destination for transcoding; if there aren’t enough bytes before the
		// used part, move it and retry, throwing in case of a second failure (the bytes to unread
		// were simply too many). After all this, if necessary, move the transcoded bytes to precede
		// and join the used part of the buffer.
		void * pBufOffset(pbReadBuf);
		size_t cbXcodeMax;
		if (m_cbReadBufUsed) {
			cbXcodeMax = m_ibReadBufUsed;
		} else {
			cbXcodeMax = m_cbReadBufLead + m_cbReadBufBulk;
		}
		size_t cbXcodeAvail(cbXcodeMax);
		text::transcode(std::nothrow, enc, &p, &cb, m_enc, &pBufOffset, &cbXcodeAvail);
		if (cb) {
			// Still some bytes to transcode: see if there’s an used part of the buffer that can be
			// moved to make more room.
			if (m_cbReadBufUsed) {
				size_t ibNew(m_cbReadBufLead + m_cbReadBufBulk - m_cbReadBufUsed);
				memory::move(pbReadBuf + ibNew, pbReadBuf + m_ibReadBufUsed, m_cbReadBufUsed);
				// Add the gained bytes to these available for transcoding.
				cbXcodeAvail += ibNew - cbXcodeMax;
				cbXcodeMax = ibNew;
				m_ibReadBufUsed = ibNew;
				// Try again.
				text::transcode(std::nothrow, enc, &p, &cb, m_enc, &pBufOffset, &cbXcodeAvail);
			}
			if (cb) {
				// The read buffer has no more room available.
				ABC_THROW(buffer_error, ());
			}
		}
		// All bytes in the source buffer were transcoded; now make sure that they immediately precede
		// the used part of the read buffer, and update the read buffer usage data.
		size_t cbXcoded(cbXcodeMax - cbXcodeAvail);
		if (m_cbReadBufUsed && cbXcoded != m_ibReadBufUsed) {
			memory::move(pbReadBuf + m_ibReadBufUsed - cbXcoded, pbReadBuf, cbXcoded);
			m_ibReadBufUsed -= cbXcoded;
		} else {
			m_ibReadBufUsed = 0;
		}
		m_cbReadBufUsed += cbXcoded;
	}
}


int8_t * file_istream::_get_read_buffer() {
	ABC_TRACE_FN((this));

	if (!m_pbReadBuf) {
		// Create the multipurpose read buffer. See [DOC:0674 abc::file_istream buffering].
		m_pbReadBuf.reset(new int8_t[m_cbReadBufLead + m_cbReadBufBulk]);
		m_ibReadBufUsed = m_cbReadBufLead;
	}
	return m_pbReadBuf.get();
}


/*virtual*/ void file_istream::_read_line(
	_raw_str * prs, text::encoding enc, unsigned cchCodePointMax, text::str_str_fn pfnStrStr
) {
	ABC_TRACE_FN((this, /*prs, */enc, cchCodePointMax/*, pfnStrStr*/));

	size_t cbChar(text::get_encoding_size(enc));
	ABC_ASSERT(cbChar > 0);
	// Little hack to obtain an index in range 0 to 2 (1 → 0, 2 → 1, 4 → 2), for use as bit shift
	// count.
	size_t cbCharLog2((0x2010u >> (cbChar - 1) * 4) & 0xf);

	// Initial buffer total and used size, in characters.
	// TODO: set cchMax to the current capacity of the string. Don’t forget to make sure it’s
	// writable.
	size_t cchMax(0), cchFilled(0);
	for (;;) {
		size_t cchAvail(cchMax - cchFilled);
		// Ensure we can fit an (even invalidly encoded) code point in the string buffer.
		if (cchAvail < cchCodePointMax) {
			// Need to enlarge the string buffer.
			cchMax += m_cchBufferStep;
			prs->set_capacity(cbChar, cchMax, false);
			cchAvail = cchMax - cchFilled;
		}

		// Read as many characters as possible, appending to the current end of the string.
		int8_t * pbLastEnd(prs->data<int8_t>() + (cchFilled << cbCharLog2));
		size_t cbRead(read_raw(pbLastEnd, cchAvail << cbCharLog2, enc));
		if (!cbRead) {
			break;
		}

		// Now we need to search for the line terminator. Since line terminators can be more than one
		// character long, back up one character first (if we have at least one), to avoid missing the
		// line terminator due to searching for it one character beyond its start.
		int8_t const * pbBeforeLastEnd(pbLastEnd - (cchFilled ? cbChar : 0));
		size_t cchBeforeLastEnd((cchFilled ? 1 : 0) + (cbRead >> cbCharLog2));
		// If the line terminator isn’t known yet, try to detect it now.
		if (m_lterm == text::line_terminator::unknown) {
			m_lterm = text::guess_line_terminator(pbBeforeLastEnd, cchBeforeLastEnd, enc);
		}
		// If no line terminator was detected, it must be because no known one was there, so avoid
		// scanning for it, and just keep on reading more bytes.
		if (m_lterm != text::line_terminator::unknown) {
			// Obtain the line terminator for the requested encoding…
			size_t cbLTerm;
			void const * pLTerm(get_line_terminator_bytes(enc, m_lterm, &cbLTerm));
			// …and search for it.
			int8_t const * pbLineEnd(static_cast<int8_t const *>(pfnStrStr(
				pbBeforeLastEnd, pbBeforeLastEnd + cchBeforeLastEnd,
				pLTerm, static_cast<int8_t const *>(pLTerm) + (cbLTerm >> cbCharLog2)
			)));
			// Check if a match was found (remember, this is *not* C strstr(): it returns the end if no
			// matches are found).
			if (pbLineEnd != pbBeforeLastEnd + cchBeforeLastEnd) {
				// Move back to the read buffer any read bytes beyond the line terminator.
				size_t ibLineEnd(size_t(pbLineEnd - pbLastEnd));
				unread_raw(pbLineEnd + cbLTerm, cbRead - ibLineEnd - cbLTerm, enc);
				// We’re actually only filling up the characters up to the line end.
				cchFilled += ibLineEnd >> cbCharLog2;
				break;
			}
		}

		// Add the characters read as part of the line.
		cchFilled += cbRead >> cbCharLog2;
	}
	prs->set_size(cbChar, cchFilled);
}


void file_istream::_post_construct() {
	ABC_TRACE_FN((this));

	// This default is enough to read lines from an 80-column file, with a single allocation.
	m_cchBufferStep = 128;
	// As a default, this will be big enough to accept the unread of a whole line in the worst-case
	// encoding (text::max_codepoint_length).
	m_cbReadBufLead = text::max_codepoint_length * m_cchBufferStep;
	// If no specific size is imposed by unbuffered access, pick a good enough size; also impose a
	// big enough number in case the physical align is too small.
	m_cbReadBufBulk = std::max<size_t>(
		m_pfile->is_buffered() ? 0 : m_pfile->physical_alignment(), 4096
	);
	// The read buffer is created on demand.
	m_ibReadBufUsed = 0;
	m_cbReadBufUsed = 0;
	// Always give an optimistic start; if the file is actually empty, the first call to read_raw()
	// will make this true.
	m_bAtEof = false;
}


/*static*/ void file_istream::_construct_std_file_istream(
	std::shared_ptr<file> const & pfile, std::shared_ptr<file_istream> ** pppfis
) {
	ABC_TRACE_FN((/*pfile, */pppfis));

	// TODO: mutex!
	ABC_ASSERT(!*pppfis);
	// TODO: reduce the number of dynamic allocations.

	std::unique_ptr<std::shared_ptr<file_istream>> ppfis(new std::shared_ptr<file_istream>());
	*ppfis.get() = std::make_shared<file_istream>(std::move(pfile));
	// If we’re still here, everything succeeded.

	// If this is the first standard stream being constructed, register the releaser.
	if (!g_ppfosStdErr && !g_ppfisStdIn && !g_ppfosStdOut) {
		::atexit(_release_std_file_streams);
	}
	// Return the allocated shared pointer.
	*pppfis = ppfis.release();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_ostream


namespace abc {

size_t const file_ostream::smc_cbWriteBufMax = 4096;


file_ostream::file_ostream(std::shared_ptr<file> pfile) :
	file_stream_base(std::move(pfile)),
	ostream() {
}
file_ostream::file_ostream(file_path const & fp) :
	file_stream_base(fp, file::access_mode::write),
	ostream() {
}


/*virtual*/ file_ostream::~file_ostream() {
}


/*virtual*/ void file_ostream::flush() {
	m_pfile->flush();
}


/*static*/ std::shared_ptr<file_ostream> const & file_ostream::stderr() {
	ABC_TRACE_FN(());

	if (!g_ppfosStdErr) {
		_construct_std_file_ostream(file::stderr(), &g_ppfosStdErr);
	}
	return *g_ppfosStdErr;
}


/*static*/ std::shared_ptr<file_ostream> const & file_ostream::stdout() {
	ABC_TRACE_FN(());

	if (!g_ppfosStdOut) {
		_construct_std_file_ostream(file::stdout(), &g_ppfosStdOut);
	}
	return *g_ppfosStdOut;
}


/*virtual*/ void file_ostream::write_raw(
	void const * p, size_t cb, text::encoding enc /*= text::encoding::identity*/
) {
	ABC_TRACE_FN((this, p, cb, enc));

	if (enc == text::encoding::unknown) {
		// Treat unknown as identity.
		enc = text::encoding::identity;
	}
	if (m_enc == text::encoding::unknown) {
		// This is the first output, so it decides for the whole file.
		m_enc = enc;
	}
	if (enc == m_enc || enc == text::encoding::identity) {
		// Optimal case: no transcoding necessary.
		m_pfile->write(p, cb);
	} else {
		// Make sure we have a trancoding buffer.
		if (!m_pbWriteBuf) {
			m_pbWriteBuf.reset(new int8_t[smc_cbWriteBufMax]);
		}
		while (cb) {
			void * pBuf(m_pbWriteBuf.get());
			size_t cbBuf(smc_cbWriteBufMax);
			// Fill as much of the buffer as possible, and write that to the file.
			cbBuf = text::transcode(std::nothrow, enc, &p, &cb, m_enc, &pBuf, &cbBuf);
			m_pfile->write(m_pbWriteBuf.get(), cbBuf);
		}
	}
}


/*static*/ void file_ostream::_construct_std_file_ostream(
	std::shared_ptr<file> const & pfile, std::shared_ptr<file_ostream> ** pppfos
) {
	ABC_TRACE_FN((/*pfile, */pppfos));

	// TODO: mutex!
	ABC_ASSERT(!*pppfos);
	// TODO: reduce the number of dynamic allocations.

	std::unique_ptr<std::shared_ptr<file_ostream>> ppfos(new std::shared_ptr<file_ostream>());
	*ppfos.get() = std::make_shared<file_ostream>(std::move(pfile));
	// If we’re still here, everything succeeded.

	// If this is the first standard stream being constructed, register the releaser.
	if (!g_ppfosStdErr && !g_ppfisStdIn && !g_ppfosStdOut) {
		::atexit(_release_std_file_streams);
	}
	// Return the allocated shared pointer.
	*pppfos = ppfos.release();
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_iostream


namespace abc {

file_iostream::file_iostream(std::shared_ptr<file> pfile) :
	file_stream_base(pfile),
	file_istream(pfile),
	file_ostream(pfile) {
}
file_iostream::file_iostream(file_path const & fp) :
	file_stream_base(fp, file::access_mode::read_write),
	file_istream(m_pfile),
	file_ostream(m_pfile) {
}


/*virtual*/ file_iostream::~file_iostream() {
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


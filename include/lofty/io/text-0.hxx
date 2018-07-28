/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2018 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

#ifndef _LOFTY_IO_TEXT_0_HXX

#ifndef _LOFTY_NOPUB
   #define _LOFTY_NOPUB
   #define _LOFTY_IO_TEXT_0_HXX
#endif

#ifndef _LOFTY_IO_TEXT_0_HXX_NOPUB
#define _LOFTY_IO_TEXT_0_HXX_NOPUB

#include <lofty/from_text_istream.hxx>
#include <lofty/noncopyable.hxx>
#include <lofty/_std/iterator.hxx>
#include <lofty/_std/memory.hxx>
#include <lofty/_std/type_traits.hxx>
#include <lofty/_std/utility.hxx>
#include <lofty/text-1.hxx>
#include <lofty/text/str-0.hxx>
#include <lofty/to_text_ostream.hxx>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Base interface for text (character-based) streams.
class LOFTY_SYM stream {
public:
   //! Destructor.
   virtual ~stream();

   /*! Returns the encoding of the data store.

   @return
      Text encoding.
   */
   virtual lofty::text::_LOFTY_PUBNS encoding get_encoding() const = 0;

   /*! Returns the line terminator used in the data store.

   @return
      Line terminator.
   */
   lofty::text::_LOFTY_PUBNS line_terminator get_line_terminator() const {
      return lterm;
   }

   /*! Assigns a new line terminator that will be used for all following reads and/or writes.

   @param lterm_
      New line terminator.
   */
   void set_line_terminator(lofty::text::_LOFTY_PUBNS line_terminator lterm_) {
      lterm = lterm_;
   }

protected:
   //! Default constructor.
   stream();

protected:
   /*! Determines how line terminators are read and written.

   When reading, a value of line_terminator::any will cause any occurrence of “\n”, “\r”, or “\r\n” to be
   accepted as a line terminator; any other value will leave all terminators unchanged, only considering the
   corresponding line terminator for line-oriented reads.

   When writing, “\n” characters will be converted to the line terminator indicated by this variable, with
   line_terminator::any having the same meaning as line_terminator::host. */
   lofty::text::_LOFTY_PUBNS line_terminator lterm;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Interface for text (character-based) input.
class LOFTY_SYM istream : public virtual stream {
public:
   //! Proxy class that allows to iterate over lines of text.
   class LOFTY_SYM _lines_proxy {
   private:
      friend class istream;

   public:
      //! Lines iterator.
      class LOFTY_SYM iterator :
         public _std::_LOFTY_PUBNS iterator<
            _std::_LOFTY_PUBNS forward_iterator_tag, lofty::text::_LOFTY_PUBNS str
         > {
      private:
         friend class _lines_proxy;

      public:
         //! Default constructor.
         iterator() :
            istream(nullptr),
            eof(true) {
         }

         /*! Dereferencing operator.

         @return
            Reference to the current line.
         */
         lofty::text::_LOFTY_PUBNS str & operator*() const {
            return last_read_line;
         }

         /*! Dereferencing member access operator.

         @return
            Pointer to the current line.
         */
         lofty::text::_LOFTY_PUBNS str * operator->() const {
            return &last_read_line;
         }

         /*! Preincrement operator.

         @return
            *this after it’s moved to the next line in the source.
         */
         iterator & operator++() {
            eof = !istream->read_line(&last_read_line);
            return *this;
         }

         /*! Postincrement operator.

         @return
            Iterator to the line following the one read by this iterator.
         */
         iterator operator++(int) {
            iterator prev_itr(*this);
            operator++();
            return _std::_pub::move(prev_itr);
         }

         /*! Equality relational operator.

         @param other
            Object to compare to *this.
         @return
            true if *this has the same source and status as other, or false otherwise.
         */
         bool operator==(iterator const & other) const {
            return istream == other.istream && eof == other.eof;
         }

         /*! Inequality relational operator.

         @param other
            Object to compare to *this.
         @return
            true if *this has a different source or status than other, or false otherwise.
         */
         bool operator!=(iterator const & other) const {
            return !operator==(other);
         }

      private:
         /*! Constructor for use by istream::_lines_proxy. When invoked it will attempt to prefetch a line
         from the source text::istream.

         @param istream_
            See istream.
         @param eof_
            See eof.
         */
         iterator(class istream * istream_, bool eof_) :
            istream(istream_),
            // If not already at EOF, fetch a new line. This may make *this == end(), which is desirable.
            eof(eof_ || !istream->read_line(&last_read_line)) {
         }

      private:
         //! Pointer to the container from which lines are read.
         class istream * const istream;
         //! Current line, or last line read before EOF.
         lofty::text::_LOFTY_PUBNS str mutable last_read_line;
         //! If true, the iterator is at the end() of its container.
         bool eof:1;
      };

   public:
      /*! Returns an iterator to the first read line.

      @return
         Iterator to the available line.
      */
      iterator begin() const {
         // TODO: maybe istream should cache its EOF status and pass it here?
         return iterator(istream, false);
      }

      /*! Returns an iterator to the end of the source.

      @return
         Line beyond the last in the source: EOF.
      */
      iterator end() const {
         return iterator(istream, true);
      }

   private:
      /*! Constructor for use by text::istream.

      @param istream_
         See istream.
      */
      explicit _lines_proxy(class istream * istream_) :
         istream(istream_) {
      }

   private:
      //! Pointer to the container from which lines are read.
      class istream * istream;
   };

public:
   /*! Marks the specified number of characters as read, so that they won’t be presented again on the next
   peek() call.

   @param count
      Count of elements to mark as read.
   */
   virtual void consume_chars(std::size_t count) = 0;

   /*! Returns a pseudo-object that allows to iterate over lines of text.

   @return
      Accessor to the lines in the source.
   */
   _lines_proxy lines() {
      return _lines_proxy(this);
   }

   /*! Returns a view of the internal read buffer. The string may initially use an external buffer provided by
   the implementation which is potentially read-only, but it will switch to a modifiable copy if necessary,
   as all lofty::text::str instances do.

   @param count_min
      Count of characters to peek.
   @return
      View of the internal string buffer. The string may be shorter than cch characters if EOF was reached, or
      longer if there are more characters available than requested, for implementation-defined reasons. For
      non-zero values of cch, a returned empty string indicates that no more data is available (EOF).
   */
   virtual lofty::text::_LOFTY_PUBNS str peek_chars(std::size_t count_min) = 0;

   /*! Returns the entire stream, emptying it.

   @return
      Content of the stream.
   */
   lofty::text::_LOFTY_PUBNS str read_all();

   /*! Reads the entire stream into a string. Efficient when the stream is expected to be of a reasonably
   small size, and the destination string can be a lofty::text::sstr instance of sufficient size, provided by
   the caller.

   The default implementation relies on peek_chars()/consume_chars().

   @param dst
      Pointer to the string that will receive the data.
   */
   virtual void read_all(lofty::text::_LOFTY_PUBNS str * dst);

   /*! Reads a whole line into the specified string, discarding the line terminator.

   The default implementation relies on peek_chars()/consume_chars().

   @param dst
      Pointer to the string that will receive the read line, or an empty string if EOF is reached before any
      characters could be read.
   @return
      true if a line could be read, or false if the end of the stream was reached.
    */
   virtual bool read_line(lofty::text::_LOFTY_PUBNS str * dst);

   /*! Reads multiple values at once, separating them according to the specified format.

   Conceptually this is the same as matching against a group-capturing regular expression; however, unlike a
   regex, this method is able to infer the format of each capturing group from the type of the corresponding
   argument, much like ostream::print().

   The format string passed as first argument is used as a regular expression against which to match the
   input, and can contain capturing groups delimited by parentheses (‘(’ and ‘)’).

   A capturing group can specify, in the parentheses, an optional type-dependent format specification; this
   will be passed as-is to the specialization of lofty::from_text_istream for the selected argument.

   @param format
      Format string specifying the regular expression to match, including any captures.
   @param dsts
      Pointers to variables that will receive the captured values if the return value is true, or will have
      undefined contents if the return value is false.
   @return
      true if the input matched the format, or false otherwise. The input will have been consumed regardless.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   bool scan(lofty::text::_LOFTY_PUBNS str const & format, Ts *... dsts);
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   bool scan(lofty::text::_LOFTY_PUBNS str const & format);
   template <typename T0>
   bool scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0);
   template <typename T0, typename T1>
   bool scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1);
   template <typename T0, typename T1, typename T2>
   bool scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2);
   template <typename T0, typename T1, typename T2, typename T3>
   bool scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3);
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
      T5 * dst5
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
      T5 * dst5, T6 * dst6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
   >
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
      T5 * dst5, T6 * dst6, T7 * dst7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8
   >
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
      T5 * dst5, T6 * dst6, T7 * dst7, T8 * dst8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9
   >
   bool scan(
      lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
      T5 * dst5, T6 * dst6, T7 * dst7, T8 * dst8, T9 * dst9
   );
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Pushes character previously consumed with consume_chars() back in the stream, making them the next
   characters thar will be yielded by peek_chars().

   This is intended to help a parser “rewind” to a known state at the end of its execution, should it reject
   the characters it peek/consumed. This would be impossible to implement without this method, since the
   parser might have consumed multiple peek buffers, and just not calling consume_chars() for the latest peek
   buffer wouldn’t restore the previous consumed peek buffers.

   This function must be called with characters that were previously returned by peek_chars() and consumed via
   consume_chars(); implementations are allowed to enforce this by rejecting characters they didn’t previously
   yield.

   @param s
      String containing the characters to unconsume.
   */
   virtual void unconsume_chars(lofty::text::_LOFTY_PUBNS str const & s) = 0;

protected:
   //! Default constructor.
   istream();

protected:
   /*! If true, and lterm is line_terminator::any, and the next read operation encounters an initial ‘\n’,
   that character will not be considered as a line terminator; this way, even if a “\r\n” was broken into
   multiple reads, we’ll still present clients with a single ‘\n’ character instead of two, as it would happen
   without this tracker (one from the trailing ‘\r’ of the first read, one from the leading ‘\n’ of the
   second. */
   bool discard_next_lf:1;
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

//! Interface for binary (non-text) output.
class LOFTY_SYM ostream : public virtual stream {
public:
   //! Flushes the underlying backend.
   virtual void flush() = 0;

   /*! Writes multiple values combined together in the specified format.

   Designed after Python’s str.format(), this allows to combine objects together as strings using a format
   string.

   The implementation of this method is entirely contained in lofty::io::text::_pvt::ostream_print_helper,
   which accesses the individual arguments in a recursive way, from the most-derived class down to the base
   class, which also contains most of the implementation. Combined with the usage of lofty::to_text_ostream()
   (which it shares with lofty::to_str()), this enables a type-safe variadic alternative to C’s printf, and
   voids the requirement for explicit specification of the argument types (such as %d, %s), much like Python’s
   str.format().

   The format string passed as first argument can contain “replacement fields” delimited by curly braces (‘{’
   and ‘}’). Anything not contained in curly braces is considered literal text and emitted as-is; the only
   exceptions are the substrings “{{” and “}}”, which allow to print “{” and “}” respectively.

   A replacement field can specify an argument index; if omitted, the argument used will be the one following
   the last used one, or the first if no arguments have been used up to that point. After the optional
   argument index, an optional type-dependent format specification can be indicated; this will be passed as-is
   to the specialization of lofty::to_text_ostream for the selected argument.

   Grammar for a replacement field:

      @verbatim
      replacement_field : “{” index? ( “:” format_spec )? “}”
      index             : [0-9]+
      format_spec       : <type-specific format specification>
      @endverbatim

   Basic usage examples for index:

      @verbatim
      "Welcome to {0}"                 Use argument 0
      "Please see items {}, {3}, {}"   Use argument 0, skip 1 and 2, use 3 and 4
      @endverbatim

   Reference for Python’s str.format(): <http://docs.python.org/3/library/string.html#format-string-syntax> .

   @param format
      Format string to parse for replacements.
   @param ts
      Replacement values.
   */
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   void print(lofty::text::_LOFTY_PUBNS str const & format, Ts const &... ts);
#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES
   void print(lofty::text::_LOFTY_PUBNS str const & format);
   template <typename T0>
   void print(lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0);
   template <typename T0, typename T1>
   void print(lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1);
   template <typename T0, typename T1, typename T2>
   void print(lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2);
   template <typename T0, typename T1, typename T2, typename T3>
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4, T5 const & t5
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
   >
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8
   >
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
      typename T8, typename T9
   >
   void print(
      lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2,
      T3 const & t3, T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   );
#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

   /*! Writes a string.

   @param s
      String to write.
   */
   void write(lofty::text::_LOFTY_PUBNS str const & s);

   /*! Writes the contents of a memory buffer, first translating them to the text stream’s character encoding,
   if necessary.

   @param src
      Pointer to the buffer to write.
   @param src_byte_size
      Size of the buffer, in bytes.
   @param enc
      Encoding used by the buffer. If different from the stream’s encoding, a conversion will be performed on
      the fly.
   */
   virtual void write_binary(
      void const * src, std::size_t src_byte_size, lofty::text::_LOFTY_PUBNS encoding enc
   ) = 0;

   /*! Writes a string followed by a new-line.

   @param s
      String to write.
   */
   void write_line(lofty::text::_LOFTY_PUBNS str const & s = lofty::text::_LOFTY_PUBNS str::empty);

protected:
   //! Default constructor.
   ostream();
};

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty { namespace io { namespace text { namespace _pvt {

/*! Template-free implementation of istream_scan_helper; gives that class the ability to refecence parsers’
methods, without including additional header files. */
class LOFTY_SYM istream_scan_helper_impl : public lofty::_LOFTY_PUBNS noncopyable {
private:
   //! Stores members of types not defined at this point.
   struct impl;

public:
   /*! Constructor.

   @param istream_
      Source stream.
   @param format
      Format string specifying the expression to match, including any captures.
   */
   istream_scan_helper_impl(_LOFTY_PUBNS istream * istream_, lofty::text::_LOFTY_PUBNS str const & format);

   //! Destructor.
   ~istream_scan_helper_impl();

   /*! Invokes the dynamic parser configured with the states generated by the regex parser, updating the
   internal match instance in *pimpl.

   @return
      true if the input matched the expression, or false otherwise.
   */
   bool run();

protected:
   lofty::text::parsers::_LOFTY_PUBNS regex_capture_format const & curr_capture_format() const;

   /*! Wrapper for pimpl->regex.insert_capture_group().

   @param first_state
      First state of the state tree to be wrapped in a capture group.
   */
   void insert_capture_group(lofty::text::parsers::_LOFTY_PUBNS dynamic_state const * first_state);

   /*! Wrapper for pimpl->match.capture_group().

   @param index
      Capture group index.
   @return
      Refregexnce to the capture help in *pimpl.
   */
   lofty::text::parsers::_LOFTY_PUBNS dynamic_match_capture const & match_capture_group(unsigned index);

   /*! Returns a pointer to the dynamic parser instance.

   @return
      Pointer to the parser stored in *pimpl.
   */
   lofty::text::parsers::_LOFTY_PUBNS dynamic * parser_ptr();

   /*! Calls pimpl->parser.parse_up_to_next_capture(), terminating the expression once no more captures are
   found.

   @return
      An index greater than or equal to 0 if a capturing group was found, or a number less than 0 if the end
      of the expression was reached.
   */
   int parse_up_to_next_capture();

   /*! Throws an instance of lofty::collections::out_of_range, providing the invalid replacement index found
   in the format string. */
   LOFTY_FUNC_NORETURN void throw_collections_out_of_range();

private:
   //! Pointer to the source stream.
   _LOFTY_PUBNS istream * istream;
   //! Pointer to members of complex types that would require additional files to be #included.
   _std::_LOFTY_PUBNS unique_ptr<impl> pimpl;
};

//! Helper for/implementation of lofty::io::text::istream::scan().
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
class istream_scan_helper;

template <>
class istream_scan_helper<> : public istream_scan_helper_impl {
public:
   /*! Constructor.

   @param istream_
      Source stream.
   @param format
      Format string specifying the expression to match, including any captures.
   */
   istream_scan_helper(_LOFTY_PUBNS istream * istream_, lofty::text::_LOFTY_PUBNS str const & format) :
      istream_scan_helper_impl(istream_, format) {
   }

   /*! Uses from_text_istream<> instances to generate dynamic parser states for the types of the variables
   pointed to by the arguments provided to the constructor. */
   void create_parser_states() {
      int arg_index = parse_up_to_next_capture();
      if (arg_index >= 0) {
         istream_scan_helper_impl::throw_collections_out_of_range();
      }
   }

   /*! Invokes the dynamic parser configured with the states generated by the regex parser. If the input
   matched the format, it also converts all captures into the variables pointed to by the arguments provided
   to the constructor.

   @return
      true if the input matched the expression, or false otherwise.
   */
   bool run_and_convert_captures() {
      return istream_scan_helper_impl::run();
      // Don’t bother calling convert_captures().
   }

protected:
   /*! Converts all captures into the variables pointed to by the arguments provided to the constructor.

   @param arg_index
      Index of the argument associated to the capture.
   */
   void convert_captures(unsigned arg_index) {
      LOFTY_UNUSED_ARG(arg_index);
      // Nothing to do.
   }

   /*! Recursive implementation of create_parser_states().

   @param arg_index
      Index of the argument associated to the capture.
   @return
      Pointer to the first state for the capture.
   */
   LOFTY_FUNC_NORETURN lofty::text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(
      unsigned arg_index
   ) {
      /* This is the last recursion stage, with no capture destinations available, so if we got here
      istream::scan() was called with insufficient capture destinations for the given expression. */
      LOFTY_UNUSED_ARG(arg_index);
      istream_scan_helper_impl::throw_collections_out_of_range();
   }
};

template <typename T0, typename... Ts>
class istream_scan_helper<T0, Ts...> : public istream_scan_helper<Ts...> {
private:
   typedef istream_scan_helper<Ts...> helper_base;

public:
   /*! Constructor.

   @param istream_
      Source stream.
   @param format
      Regular expression to parse.
   @param dst0_
      Pointer to the Nth capture destination.
   @param dsts
      Pointer to the remaining capture destinations.
   */
   istream_scan_helper(
      _LOFTY_PUBNS istream * istream_, lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0_, Ts *... dsts
   ) :
      helper_base(istream_, format, dsts...),
      dst0(dst0_) {
   }

   //! See istream_scan_helper<>::create_parser_states().
   void create_parser_states() {
      int arg_index;
      while ((arg_index = helper_base::parse_up_to_next_capture()) >= 0) {
         // Perform and write the replacement.
         helper_base::insert_capture_group(format_to_parser_states(static_cast<unsigned>(arg_index)));
      }
   }

   //! See istream_scan_helper<>::run_and_convert_captures().
   bool run_and_convert_captures() {
      if (istream_scan_helper_impl::run()) {
         convert_captures(0);
         return true;
      } else {
         return false;
      }
   }

protected:
   //! See istream_scan_helper<>::convert_captures().
   void convert_captures(unsigned arg_index) {
      ftis.convert_capture(helper_base::match_capture_group(arg_index), dst0);
      // Recurse to the previous level.
      helper_base::convert_captures(arg_index + 1);
   }

   //! See istream_scan_helper<>::format_to_parser_states().
   lofty::text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(unsigned arg_index) {
      if (arg_index == 0) {
         return ftis.format_to_parser_states(helper_base::curr_capture_format(), helper_base::parser_ptr());
      } else {
         // Recurse to the previous level.
         return helper_base::format_to_parser_states(arg_index - 1);
      }
   }

private:
   //! Nth capture destination.
   T0 * dst0;
   //! Generates parser states for T0, and converts captured strings into T0.
   from_text_istream<T0> ftis;
};

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

// Recursion step: extract one argument, recurse with the rest.
template <
   typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void, typename T4 = void,
   typename T5 = void, typename T6 = void, typename T7 = void, typename T8 = void, typename T9 = void
>
class istream_scan_helper : public istream_scan_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> {
private:
   typedef istream_scan_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> helper_base;

public:
   /*! Constructor.

   @param istream_
      Source stream.
   @param format
      Format string specifying the expression to match, including any captures.
   @param dst0
      Pointer to the first capture destination.
   @param dst1
      Pointer to the second capture destination.
   @param dst2
      Pointer to the third capture destination.
   @param dst3
      Pointer to the fourth capture destination.
   @param dst4
      Pointer to the fifth capture destination.
   @param dst5
      Pointer to the sixth capture destination.
   @param dst6
      Pointer to the seventh capture destination.
   @param dst7
      Pointer to the eighth capture destination.
   @param dst8
      Pointer to the ninth capture destination.
   @param dst9
      Pointer to the tenth capture destination.
   */
   template <typename U0>
   istream_scan_helper(typename _std::_LOFTY_PUBNS enable_if<
      !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
   >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_) :
      helper_base(istream_, format_),
      dst0(dst0_) {
   }
   template <typename U0, typename U1>
   istream_scan_helper(typename _std::_LOFTY_PUBNS enable_if<
      !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
   >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1) :
      helper_base(istream_, format_, dst1),
      dst0(dst0_) {
   }
   template <typename U0, typename U1, typename U2>
   istream_scan_helper(typename _std::_LOFTY_PUBNS enable_if<
      !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
   >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2) :
      helper_base(istream_, format_, dst1, dst2),
      dst0(dst0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3),
      dst0(dst0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4),
      dst0(dst0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4, U5 * dst5
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4, dst5),
      dst0(dst0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4, U5 * dst5, U6 * dst6
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4, dst5, dst6),
      dst0(dst0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7
   >
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4, U5 * dst5, U6 * dst6, U7 * dst7
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4, dst5, dst6, dst7),
      dst0(dst0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8
   >
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4, U5 * dst5, U6 * dst6, U7 * dst7, U8 * dst8
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8),
      dst0(dst0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   istream_scan_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS istream *
      >::type istream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 * dst0_, U1 * dst1, U2 * dst2,
      U3 * dst3, U4 * dst4, U5 * dst5, U6 * dst6, U7 * dst7, U8 * dst8, U9 * dst9
   ) :
      helper_base(istream_, format_, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9),
      dst0(dst0_) {
   }

   //! See istream_scan_helper<>::create_parser_states().
   void create_parser_states() {
      int arg_index;
      while ((arg_index = helper_base::parse_up_to_next_capture()) >= 0) {
         // Perform and write the replacement.
         helper_base::insert_capture_group(format_to_parser_states(static_cast<unsigned>(arg_index)));
      }
   }

   //! See istream_scan_helper<>::run_and_convert_captures().
   bool run_and_convert_captures() {
      if (istream_scan_helper_impl::run()) {
         convert_captures(0);
         return true;
      } else {
         return false;
      }
   }

protected:
   //! See istream_scan_helper<>::convert_captures().
   void convert_captures(unsigned arg_index) {
      ftis.convert_capture(helper_base::match_capture_group(arg_index), dst0);
      // Recurse to the previous level.
      helper_base::convert_captures(arg_index + 1);
   }

   //! See istream_scan_helper<>::format_to_parser_states().
   lofty::text::parsers::_LOFTY_PUBNS dynamic_state const * format_to_parser_states(unsigned arg_index) {
      if (arg_index == 0) {
         return ftis.format_to_parser_states(helper_base::curr_capture_format(), helper_base::parser_ptr());
      } else {
         // Recurse to the previous level.
         return helper_base::format_to_parser_states(arg_index - 1);
      }
   }

private:
   //! Nth capture destination.
   T0 * dst0;
   //! Generates parser states for T0, and converts captured strings into T0.
   from_text_istream<T0> ftis;
};

// Base recursion step: no arguments to replace.
template <>
class istream_scan_helper<> : public istream_scan_helper_impl {
public:
   /*! Constructor.

   @param istream_
      Source stream.
   @param format
      Format string specifying the expression to match, including any captures.
   */
   istream_scan_helper(_LOFTY_PUBNS istream * istream_, lofty::text::_LOFTY_PUBNS str const & format) :
      istream_scan_helper_impl(istream_, format) {
   }

   /*! Uses from_text_istream<> instances to generate dynamic parser states for the types of the variables
   pointed to by the arguments provided to the constructor. */
   void create_parser_states() {
      int arg_index = parse_up_to_next_capture();
      if (arg_index >= 0) {
         istream_scan_helper_impl::throw_collections_out_of_range();
      }
   }

   /*! Invokes the dynamic parser configured with the states generated by the regex parser. If the input
   matched the format, it also converts all captures into the variables pointed to by the arguments provided
   to the constructor.

   @return
      true if the input matched the expression, or false otherwise.
   */
   bool run_and_convert_captures() {
      return istream_scan_helper_impl::run();
      // Don’t bother calling convert_captures().
   }

protected:
   /*! Converts all captures into the variables pointed to by the arguments provided to the constructor.

   @param arg_index
      Index of the argument associated to the capture.
   */
   void convert_captures(unsigned arg_index) {
      LOFTY_UNUSED_ARG(arg_index);
      // Nothing to do.
   }

   /*! Recursive implementation of create_parser_states().

   @param arg_index
      Index of the argument associated to the capture.
   @return
      Pointer to the first state for the capture.
   */
   LOFTY_FUNC_NORETURN lofty::text::parsers::dynamic_state const * format_to_parser_states(
      unsigned arg_index
   ) {
      /* This is the last recursion stage, with no capture destinations available, so if we got here
      istream::scan() was called with insufficient capture destinations for the given expression. */
      LOFTY_UNUSED_ARG(arg_index);
      istream_scan_helper_impl::throw_collections_out_of_range();
   }
};

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

}}}} //namespace lofty::io::text::_pvt
//! @endcond

// Now it’s possible to implement this.

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline bool istream::scan(lofty::text::_LOFTY_PUBNS str const & format, Ts *... dsts) {
   _pvt::istream_scan_helper<Ts...> helper(this, format, dsts...);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

inline bool istream::scan(lofty::text::_LOFTY_PUBNS str const & format) {
   _pvt::istream_scan_helper<> helper(this, format);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0>
inline bool istream::scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0) {
   _pvt::istream_scan_helper<T0> helper(this, format, dst0);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1>
inline bool istream::scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1) {
   _pvt::istream_scan_helper<T0, T1> helper(this, format, dst0, dst1);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1, typename T2>
inline bool istream::scan(lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2) {
   _pvt::istream_scan_helper<T0, T1, T2> helper(this, format, dst0, dst1, dst2);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1, typename T2, typename T3>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3> helper(this, format, dst0, dst1, dst2, dst3);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4> helper(this, format, dst0, dst1, dst2, dst3, dst4);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
   T5 * dst5
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4, T5> helper(this, format, dst0, dst1, dst2, dst3, dst4, dst5);
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
   T5 * dst5, T6 * dst6
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4, T5, T6> helper(
      this, format, dst0, dst1, dst2, dst3, dst4, dst5, dst6
   );
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
   T5 * dst5, T6 * dst6, T7 * dst7
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4, T5, T6, T7> helper(
      this, format, dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7
   );
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
   T5 * dst5, T6 * dst6, T7 * dst7, T8 * dst8
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8> helper(
      this, format, dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8
   );
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline bool istream::scan(
   lofty::text::_LOFTY_PUBNS str const & format, T0 * dst0, T1 * dst1, T2 * dst2, T3 * dst3, T4 * dst4,
   T5 * dst5, T6 * dst6, T7 * dst7, T8 * dst8, T9 * dst9
) {
   _pvt::istream_scan_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> helper(
      this, format, dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, dst8, dst9
   );
   helper.create_parser_states();
   return helper.run_and_convert_captures();
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

//! @cond
namespace lofty { namespace io { namespace text { namespace _pvt {

//! Template-free implementation of lofty::io::text::_pvt::ostream_print_helper.
class LOFTY_SYM ostream_print_helper_impl : public lofty::_LOFTY_PUBNS noncopyable {
public:
   /*! Constructor.

   @param ostream
      Text stream to output to.
   @param format
      Format string to parse for replacements.
   */
   ostream_print_helper_impl(
      _LOFTY_PUBNS ostream * ostream, lofty::text::_LOFTY_PUBNS str const & format
   );

   //! Writes the provided arguments to the target text stream, performing replacements as necessary.
   void run();

protected:
   /*! Throws an instance of lofty::collections::out_of_range, providing the invalid replacement index found
   in the format string. */
   LOFTY_FUNC_NORETURN void throw_collections_out_of_range();

   /*! Writes the portion of format string between format_to_write_begin_itr and the next replacement and
   returns true, or writes the remaining characters of the format string and returns false if no more
   replacement are found.

   @return
      true if another replacement was found and should be printed, or false otherwise.
   */
   bool write_format_up_to_next_repl();

private:
   /*! Throws an instance of lofty::syntax_error(), providing accurate context information.

   @param description
      Error description.
   @param itr
      Position of the offending character in format.
   */
   LOFTY_FUNC_NORETURN void throw_syntax_error(
      lofty::text::_LOFTY_PUBNS str const & description, lofty::text::_LOFTY_PUBNS str::const_iterator itr
   ) const;

   /*! Writes the portion of format string between the first character to be written
   (format_to_write_begin_itr) and the specified one, and updates format_to_write_begin_itr.

   @param up_to
      First character not to be written.
   */
   void write_format_up_to(lofty::text::_LOFTY_PUBNS str::const_iterator up_to);

protected:
   //! Target text output stream.
   _LOFTY_PUBNS ostream * ostream;
   // TODO: use iterators for the following two member variables.
   //! Start of the format specification of the current replacement.
   lofty::text::_LOFTY_PUBNS char_t const * repl_format_spec_begin;
   //! End of the format specification of the current replacement.
   lofty::text::_LOFTY_PUBNS char_t const * repl_format_spec_end;
   //! 0-based index of the argument to replace the next replacement.
   unsigned last_used_arg_index;

private:
   //! Format string.
   lofty::text::_LOFTY_PUBNS str const & format;
   //! First format string character to be written yet.
   lofty::text::_LOFTY_PUBNS str::const_iterator format_to_write_begin_itr;
};

//! Helper for/implementation of lofty::io::text::ostream::print().
#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
class ostream_print_helper;

// Base recursion step: no arguments to replace.
template <>
class ostream_print_helper<> : public ostream_print_helper_impl {
public:
   /*! Constructor.

   @param ostream_
      Text stream to output to.
   @param format_
      Format string to parse for replacements.
   */
   ostream_print_helper(_LOFTY_PUBNS ostream * ostream_, lofty::text::_LOFTY_PUBNS str const & format_) :
      ostream_print_helper_impl(ostream_, format_) {
   }

protected:
   /*! Writes T0 if arg_index == 0, or forwards the call to the previous recursion level.

   @param arg_index
      0-based index of the template argument to write.
   */
   LOFTY_FUNC_NORETURN void write_repl(unsigned arg_index) {
      /* This is the last recursion stage, with no replacements available, so if we got here ostream::print()
      was called with insufficient replacements for the given format string. */
      LOFTY_UNUSED_ARG(arg_index);
      ostream_print_helper_impl::throw_collections_out_of_range();
   }
};

// Recursion step: extract one argument, recurse with the rest.
template <typename T0, typename... Ts>
class ostream_print_helper<T0, Ts ...> : public ostream_print_helper<Ts ...> {
private:
   typedef ostream_print_helper<Ts ...> helper_base;

public:
   /*! Constructor.

   @param ostream_
      Text stream to output to.
   @param format_
      Format string to parse for replacements.
   @param t0
      First replacement value.
   @param ts
      Remaining replacement values.
   */
   ostream_print_helper(
      _LOFTY_PUBNS ostream * ostream_, lofty::text::_LOFTY_PUBNS str const & format_,
      T0 const & t0_, Ts const &... ts
   ) :
      helper_base(ostream_, format_, ts ...),
      t0(t0_) {
   }

   //! See ostream_print_helper<>::run().
   void run() {
      while (helper_base::write_format_up_to_next_repl()) {
         // Perform and write the replacement.
         write_repl(helper_base::last_used_arg_index);
      }
   }

protected:
   //! See ostream_print_helper<>::write_repl().
   void write_repl(unsigned arg_index) {
      if (arg_index == 0) {
         to_text_ostream<T0> ttos;
         ttos.set_format(lofty::text::_pub::str(
            lofty::_pub::external_buffer, helper_base::repl_format_spec_begin, static_cast<std::size_t>(
               helper_base::repl_format_spec_end - helper_base::repl_format_spec_begin
            )
         ));
         ttos.write(t0, helper_base::ostream);
      } else {
         // Recurse to the previous level.
         helper_base::write_repl(arg_index - 1);
      }
   }

private:
   //! Nth replacement.
   T0 const & t0;
};

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

// Recursion step: extract one argument, recurse with the rest.
template <
   typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void, typename T4 = void,
   typename T5 = void, typename T6 = void, typename T7 = void, typename T8 = void, typename T9 = void
>
class ostream_print_helper : public ostream_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> {
private:
   typedef ostream_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> helper_base;

public:
   /*! Constructor.

   @param ostream_
      Text stream to output to.
   @param format_
      Format string to parse for replacements.
   @param t0
      First replacement value.
   @param t1
      Second replacement value.
   @param t2
      Third replacement value.
   @param t3
      Fourth replacement value.
   @param t4
      Fifth replacement value.
   @param t5
      Sixth replacement value.
   @param t6
      Seventh replacement value.
   @param t7
      Eighth replacement value.
   @param t8
      Ninth replacement value.
   @param t9
      Tenth replacement value.
   */
   template <typename U0>
   ostream_print_helper(typename _std::_LOFTY_PUBNS enable_if<
      !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
   >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_) :
      helper_base(ostream_, format_),
      t0(t0_) {
   }
   template <typename U0, typename U1>
   ostream_print_helper(typename _std::_LOFTY_PUBNS enable_if<
      !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
   >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1) :
      helper_base(ostream_, format_, t1),
      t0(t0_) {
   }
   template <typename U0, typename U1, typename U2>
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2
   ) :
      helper_base(ostream_, format_, t1, t2),
      t0(t0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3
   ) :
      helper_base(ostream_, format_, t1, t2, t3),
      t0(t0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4),
      t0(t0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4, t5),
      t0(t0_) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6>
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5, U6 const & t6
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4, t5, t6),
      t0(t0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7
   >
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5, U6 const & t6, U7 const & t7
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4, t5, t6, t7),
      t0(t0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8
   >
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5, U6 const & t6, U7 const & t7, U8 const & t8
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4, t5, t6, t7, t8),
      t0(t0_) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6, typename U7,
      typename U8, typename U9
   >
   ostream_print_helper(
      typename _std::_LOFTY_PUBNS enable_if<
         !_std::_LOFTY_PUBNS is_void<U0>::value, _LOFTY_PUBNS ostream *
      >::type ostream_, lofty::text::_LOFTY_PUBNS str const & format_, U0 const & t0_, U1 const & t1,
      U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5, U6 const & t6, U7 const & t7, U8 const & t8,
      U9 const & t9
   ) :
      helper_base(ostream_, format_, t1, t2, t3, t4, t5, t6, t7, t8, t9),
      t0(t0_) {
   }

   //! See ostream_print_helper<>::run().
   void run() {
      while (helper_base::write_format_up_to_next_repl()) {
         // Perform and write the replacement.
         write_repl(helper_base::last_used_arg_index);
      }
   }

protected:
   //! See ostream_print_helper<>::write_repl().
   void write_repl(unsigned arg_index) {
      if (arg_index == 0) {
         to_text_ostream<T0> ttos;
         ttos.set_format(lofty::text::_pub::str(
            lofty::_pub::external_buffer, helper_base::repl_format_spec_begin, static_cast<std::size_t>(
               helper_base::repl_format_spec_end - helper_base::repl_format_spec_begin
            )
         ));
         ttos.write(t0, helper_base::ostream);
      } else {
         // Recurse to the previous level.
         helper_base::write_repl(arg_index - 1);
      }
   }

private:
   //! Nth replacement.
   T0 const & t0;
};

// Base recursion step: no arguments to replace.
template <>
class ostream_print_helper<> : public ostream_print_helper_impl {
public:
   /*! Constructor.

   @param ostream_
      Text stream to output to.
   @param format_
      Format string to parse for replacements.
   */
   ostream_print_helper(_LOFTY_PUBNS ostream * ostream_, lofty::text::_LOFTY_PUBNS str const & format_) :
      ostream_print_helper_impl(ostream_, format_) {
   }

protected:
   /*! Writes T0 if arg_index == 0, or fowards the call to the previous recursion level.

   @param arg_index
      0-based index of the template argument to write.
   */
   LOFTY_FUNC_NORETURN void write_repl(unsigned arg_index) {
      /* This is the last recursion stage, with no replacements available, so if we got here ostream::print()
      was called with insufficient replacements for the given format string. */
      LOFTY_UNUSED_ARG(arg_index);
      ostream_print_helper_impl::throw_collections_out_of_range();
   }
};

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

}}}} //namespace lofty::io::text::_pvt
//! @endcond

// Now it’s possible to implement this.

namespace lofty { namespace io { namespace text {
_LOFTY_PUBNS_BEGIN

#ifdef LOFTY_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline void ostream::print(lofty::text::_LOFTY_PUBNS str const & format, Ts const &... ts) {
   _pvt::ostream_print_helper<Ts ...> helper(this, format, ts ...);
   helper.run();
}

#else //ifdef LOFTY_CXX_VARIADIC_TEMPLATES

inline void ostream::print(lofty::text::_LOFTY_PUBNS str const & format) {
   _pvt::ostream_print_helper<> helper(this, format);
   helper.run();
}
template <typename T0>
inline void ostream::print(lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0) {
   _pvt::ostream_print_helper<T0> helper(this, format, t0);
   helper.run();
}
template <typename T0, typename T1>
inline void ostream::print(lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1) {
   _pvt::ostream_print_helper<T0, T1> helper(this, format, t0, t1);
   helper.run();
}
template <typename T0, typename T1, typename T2>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2
) {
   _pvt::ostream_print_helper<T0, T1, T2> helper(this, format, t0, t1, t2);
   helper.run();
}
template <typename T0, typename T1, typename T2, typename T3>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3> helper(this, format, t0, t1, t2, t3);
   helper.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4> helper(this, format, t0, t1, t2, t3, t4);
   helper.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4, T5 const & t5
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4, T5> helper(this, format, t0, t1, t2, t3, t4, t5);
   helper.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4, T5 const & t5, T6 const & t6
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4, T5, T6> helper(this, format, t0, t1, t2, t3, t4, t5, t6);
   helper.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7
>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7> helper(
      this, format, t0, t1, t2, t3, t4, t5, t6, t7
   );
   helper.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8
>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8> helper(
      this, format, t0, t1, t2, t3, t4, t5, t6, t7, t8
   );
   helper.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7,
   typename T8, typename T9
>
inline void ostream::print(
   lofty::text::_LOFTY_PUBNS str const & format, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
   T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   _pvt::ostream_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> helper(
      this, format, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
   helper.run();
}

#endif //ifdef LOFTY_CXX_VARIADIC_TEMPLATES … else

_LOFTY_PUBNS_END
}}} //namespace lofty::io::text

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //ifndef _LOFTY_IO_TEXT_0_HXX_NOPUB

#ifdef _LOFTY_IO_TEXT_0_HXX
   #undef _LOFTY_NOPUB

   namespace lofty { namespace io { namespace text {

   using _pub::istream;
   using _pub::ostream;
   using _pub::stream;

   }}}

   #ifdef LOFTY_CXX_PRAGMA_ONCE
      #pragma once
   #endif
#endif

#endif //ifndef _LOFTY_IO_TEXT_0_HXX

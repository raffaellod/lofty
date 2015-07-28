/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014, 2015
Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
Public License for more details.

You should have received a copy of the GNU General Public License along with Abaclade. If not, see
<http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

// Forward declarations.
class reader;
class writer;
class binbuf_base;
class binbuf_reader;
class binbuf_writer;

//! Text writer associated to the standard error output file.
extern ABACLADE_SYM std::shared_ptr<writer> stderr;
//! Text reader associated to the standard input file.
extern ABACLADE_SYM std::shared_ptr<reader> stdin;
//! Text writer associated to the standard output file.
extern ABACLADE_SYM std::shared_ptr<writer> stdout;

/*! Creates and returns a text reader for the specified binary reader.

@param pbr
   Pointer to a binary reader.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader operating on top of the specified binary reader.
*/
ABACLADE_SYM std::shared_ptr<reader> make_reader(
   std::shared_ptr<binary::reader> pbr, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Creates and returns a text writer for the specified binary writer.

@param pbw
   Pointer to a binary writer.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer operating on top of the specified binary writer.
*/
ABACLADE_SYM std::shared_ptr<writer> make_writer(
   std::shared_ptr<binary::writer> pbw, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode access.

@param op
   Path to the file.
@param am
   Desired access mode.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text I/O object for the file.
*/
ABACLADE_SYM std::shared_ptr<binbuf_base> open(
   os::path const & op, access_mode am, abc::text::encoding enc = abc::text::encoding::unknown
);

/*! Opens a file for text-mode reading.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text reader for the file.
*/
inline std::shared_ptr<binbuf_reader> open_reader(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::unknown
) {
   return std::dynamic_pointer_cast<binbuf_reader>(open(op, access_mode::read, enc));
}

/*! Opens a file for text-mode writing.

@param op
   Path to the file.
@param enc
   Encoding to be used the the text.
@return
   Pointer to a text writer for the file.
*/
inline std::shared_ptr<binbuf_writer> open_writer(
   os::path const & op, abc::text::encoding enc = abc::text::encoding::utf8
) {
   return std::dynamic_pointer_cast<binbuf_writer>(open(op, access_mode::write, enc));
}

}}} //namespace abc::io::text

namespace abc { namespace io { namespace text { namespace detail {

/*! Creates and returns a text writer associated to the standard error output file (stderr).

@return
   Standard error file.
*/
ABACLADE_SYM std::shared_ptr<writer> make_stderr();

/*! Creates and returns a text reader associated to the standard input file (stdin).

@return
   Standard input file.
*/
ABACLADE_SYM std::shared_ptr<reader> make_stdin();

/*! Creates and returns a text writer associated to the standard output file (stdout).

@return
   Standard output file.
*/
ABACLADE_SYM std::shared_ptr<writer> make_stdout();

}}}} //namespace abc::io::text::detail

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Base interface for text (character-based) I/O.
class ABACLADE_SYM base {
public:
   //! Destructor.
   virtual ~base();

   /*! Returns the encoding of the data store.

   @return
      Text encoding.
   */
   virtual abc::text::encoding get_encoding() const = 0;

   /*! Returns the line terminator used in the data store.

   @return
      Line terminator.
   */
   abc::text::line_terminator get_line_terminator() const {
      return m_lterm;
   }

   /*! Assigns a new line terminator that will be used for all following writes.

   @param lterm
      New line terminator.
   */
   void set_line_terminator(abc::text::line_terminator lterm) {
      m_lterm = lterm;
   }

protected:
   //! Constructor.
   base();

protected:
   /*! Determines how line terminators are read and written.

   When reading, a value of line_terminator::any or line_terminator::convert_any_to_lf will cause
   any occurrence of “\n”, “\r”, or “\r\n” to be accepted as a line terminator, and
   line_terminator::convert_any_to_lf will additionally cause them to be returned to the reader as
   “\n”; any other value will leave all terminators unchanged, only considering the corresponding
   line terminator for line-oriented reads.

   When writing, “\n” characters will be converted to the line terminator indicated by this
   variable, with line_terminator::any and line_terminator::convert_any_to_lf having the same
   meaning as line_terminator::host. */
   abc::text::line_terminator m_lterm;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Interface for text (character-based) input.
class ABACLADE_SYM reader : public virtual base {
public:
   //! Proxy class that allows to iterate over lines of text.
   class ABACLADE_SYM _lines_proxy {
   private:
      friend class reader;

   public:
      //! Lines iterator.
      class ABACLADE_SYM iterator :
         public std::iterator<std::forward_iterator_tag, dmstr> {
      private:
         friend class _lines_proxy;

      public:
         //! Constructor.
         iterator() :
            mc_ptr(nullptr),
            m_bEOF(true) {
         }

         /*! Dereferencing operator.

         @return
            Reference to the current line.
         */
         dmstr & operator*() const {
            return m_s;
         }

         /*! Dereferencing member access operator.

         @return
            Pointer to the current line.
         */
         dmstr * operator->() const {
            return &m_s;
         }

         /*! Preincrement operator.

         @return
            *this after it’s moved to the next line in the source.
         */
         iterator & operator++() {
            m_bEOF = !mc_ptr->read_line(&m_s);
            return *this;
         }

         /*! Postincrement operator.

         @return
            Iterator to the line following the one read by this iterator.
         */
         iterator operator++(int) const {
            return ++iterator(*this);
         }

         /*! Equality relational operator.

         @param it
            Object to compare to *this.
         @return
            true if *this has the same source and status as it, or false otherwise.
         */
         bool operator==(iterator const & it) const {
            return mc_ptr == it.mc_ptr && m_bEOF == it.m_bEOF;
         }

         /*! Inequality relational operator.

         @param it
            Object to compare to *this.
         @return
            true if *this has a different source or status than it, or false otherwise.
         */
         bool operator!=(iterator const & it) const {
            return !operator==(it);
         }

      private:
         /*! Constructor for use by reader::_lines_proxy. When invoked it will attempt to prefetch a
         line from the source text::reader.

         @param ptr
            See mc_ptr.
         @param bEOF
            See m_bEOF.
         */
         iterator(reader * ptr, bool bEOF) :
            mc_ptr(ptr),
            /* If not already at EOF, fetch a new line. This may make *this == end(), which is
            desirable. */
            m_bEOF(bEOF || !ptr->read_line(&m_s)) {
         }

      private:
         //! Pointer to the container from which lines are read.
         reader * const mc_ptr;
         //! Last line read.
         dmstr mutable m_s;
         //! If true, the iterator is at the end() of its container.
         bool m_bEOF:1;
      };

   public:
      /*! Returns an iterator to the first read line.

      @return
         Iterator to the available line.
      */
      iterator begin() const {
         // TODO: maybe m_ptr should cache its EOF status and pass it here?
         return iterator(m_ptr, false);
      }

      /*! Returns an iterator to the end of the source.

      @return
         Line beyond the last in the source: EOF.
      */
      iterator end() const {
         return iterator(m_ptr, true);
      }

   private:
      /*! Constructor for use by text::reader.

      @param ptr
         See m_ptr.
      */
      explicit _lines_proxy(reader * ptr) :
         m_ptr(ptr) {
      }

   private:
      //! Pointer to the container from which lines are read.
      reader * m_ptr;
   };

public:
   /*! Returns a pseudo-object that allows to iterate over lines of text.

   @return
      Accessor to the lines in the source.
   */
   _lines_proxy lines() {
      return _lines_proxy(this);
   }

   /*! Reads the entire source into the specified mutable string.

   @param psDst
      Pointer to the string that will receive the data.
   @return
      Content of the source, if psDst is omitted.
   */
   dmstr read_all();
   void read_all(mstr * psDst);

   /*! Reads a whole line into the specified mutable string, discarding the line terminator.

   @param psDst
      Pointer to the string that will receive the read line.
   @return
      true if a line could be read, or false if the end of the data was reached, in which case
      *psDst is left in an undetermined state.
   */
   bool read_line(mstr * psDst);

protected:
   //! See base::base().
   reader();

   /*! Reads data into the specified mutable string, invoking a callback function to determine how
   much of the read data should be consumed.

   @param psDst
      Pointer to the string that will receive the read data.
   @param bOneLine
      If true, reading will stop at the first line terminator character.
   @return
      true if a string could be read, or false if the reader was at EOF.
   */
   virtual bool read_line_or_all(mstr * psDst, bool bOneLine) = 0;
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text {

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer : public virtual base {
public:
   /*! Finalizes the underlying backend, ensuring that no error conditions remain possible in the
   destructor. */
   virtual void finalize() = 0;

   //! Flushes the underlying backend.
   virtual void flush() = 0;

   /*! Writes multiple values combined together in the specified format.

   Designed after Python’s str.format(), this allows to combine objects together as strings using a
   format string.

   The implementation of print() is entirely contained in
   abc::io::text::detail::writer_print_helper, which accesses the individual arguments in a
   recursive way, from the most-derived class down to the base class, which also contains most of
   the implementation. Combined with the usage of abc::to_str_backend() (that it shares with
   abc::to_str()), this enables a type-safe variadic alternative to C’s printf, and voids the
   requirement for explicit specification of the argument types (such as %d, %s), much like Python’s
   str.format().

   Because of its type-safety, print() is also the core of @ref stack-tracing, as it allows to print
   a variable by automatically deducing its type.

   The format string passed as first argument to abc::io::text::writer::print() can contain
   “replacement fields” delimited by curly braces (‘{’ and ‘}’). Anything not contained in curly
   braces is considered literal text and emitted as-is; the only exceptions are the substrings “{{”
   and “}}”, which allow to print “{” and “}” respectively.

   A replacement field can specify an argument index; if omitted, the argument used will be the one
   following the last used one, or the first if no arguments have been used up to that point. After
   the optional argument index, an optional type-dependent format specification can be indicated;
   this will be passed as-is to the specialization of abc::to_str_backend for the selected argument.

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

   Reference for Python’s str.format(): <http://docs.python.org/3/library/string.html#format-string-
   syntax>

   @param sFormat
      Format string to parse for replacements.
   @param ts
      Replacement values.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename... Ts>
   void print(istr const & sFormat, Ts const &... ts);
#else //ifdef ABC_CXX_VARIADIC_TEMPLATES
   void print(istr const & sFormat);
   template <typename T0>
   void print(istr const & sFormat, T0 const & t0);
   template <typename T0, typename T1>
   void print(istr const & sFormat, T0 const & t0, T1 const & t1);
   template <typename T0, typename T1, typename T2>
   void print(istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2);
   template <typename T0, typename T1, typename T2, typename T3>
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4>
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4
   );
   template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4, T5 const & t5
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6
   >
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4, T5 const & t5, T6 const & t6
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7
   >
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8
   >
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
   );
   template <
      typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
      typename T7, typename T8, typename T9
   >
   void print(
      istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3,
      T4 const & t4, T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
   );
#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

   /*! Writes a value using the default formatting for abc::to_str_backend().

   @param t
      Value to write.
   */
   template <typename T>
   void write(T const & t) {
      to_str_backend<T> tsb;
      tsb.write(t, this);
   }

   /*! Writes the contents of a memory buffer, first translating them to the text writer’s character
   encoding, if necessary.

   @param pSrc
      Pointer to the buffer to write.
   @param cbSrc
      Size of the buffer, in bytes.
   @param enc
      Encoding used by the buffer. If different from the writer’s encoding, a conversion will be
      performed on the fly.
   */
   virtual void write_binary(void const * pSrc, std::size_t cbSrc, abc::text::encoding enc) = 0;

   /*! Writes a string followed by a new-line.

   @param s
      String to write.
   */
   void write_line(istr const & s = istr::empty);

protected:
   //! See base::base().
   writer();
};

}}} //namespace abc::io::text

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace io { namespace text { namespace detail {

//! Template-free implementation of abc::io::text::detail::writer_print_helper.
class ABACLADE_SYM writer_print_helper_impl : public noncopyable {
public:
   /*! Constructor.

   @param ptw
      Text writer to use.
   @param sFormat
      Format string to parse for replacements.
   */
   writer_print_helper_impl(writer * ptw, istr const & sFormat);

   /*! Writes the provided arguments to the target text writer, performing replacements as
   necessary. */
   void run();

protected:
   /*! Throws an instance of abc::index_error(), providing the invalid replacement index found in
   the format string. */
   ABC_FUNC_NORETURN void throw_index_error();

   /*! Writes the portion of format string between m_itFormatToWriteBegin and the next replacement
   and returns true, or writes the remaining characters of the format string and returns false if no
   more replacement are found.

   @return
      true if another replacement was found and should be printed, or false otherwise.
   */
   bool write_format_up_to_next_repl();

private:
   /*! Throws an instance of abc::syntax_error(), providing accurate context information.

   @param sDescription
      Error description.
   @param it
      Position of the offending character in m_sFormat.
   */
   ABC_FUNC_NORETURN void throw_syntax_error(
      istr const & sDescription, istr::const_iterator it
   ) const;

   /*! Writes the portion of format string between the first character to be written
   (m_itFormatToWriteBegin) and the specified one, and updates m_itFormatToWriteBegin.

   @param itUpTo
      First character not to be written.
   */
   void write_format_up_to(istr::const_iterator itUpTo);

protected:
   //! Target text writer.
   writer * m_ptw;
   // TODO: use iterators for the following two member variables.
   //! Start of the format specification of the current replacement.
   char_t const * m_pchReplFormatSpecBegin;
   //! End of the format specification of the current replacement.
   char_t const * m_pchReplFormatSpecEnd;
   //! 0-based index of the argument to replace the next replacement.
   unsigned m_iSubstArg;

private:
   //! Format string.
   istr const & m_sFormat;
   //! First format string character to be written yet.
   istr::const_iterator m_itFormatToWriteBegin;
};

//! Helper for/implementation of abc::io::text::writer::print().
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
class writer_print_helper;

// Base recursion step: no arguments to replace.
template <>
class writer_print_helper<> : public writer_print_helper_impl {
public:
   /*! Constructor.

   @param ptw
      Text writer to use.
   @param sFormat
      Format string to parse for replacements.
   */
   writer_print_helper(writer * ptw, istr const & sFormat) :
      writer_print_helper_impl(ptw, sFormat) {
   }

protected:
   /*! Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

   @param iArg
      0-based index of the template argument to write.
   */
   ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
      /* This is the last recursion stage, with no replacements available, so if we got here
      writer::print() was called with insufficient replacements for the given format string. */
      ABC_UNUSED_ARG(iArg);
      writer_print_helper_impl::throw_index_error();
   }
};

// Recursion step: extract one argument, recurse with the rest.
template <typename T0, typename... Ts>
class writer_print_helper<T0, Ts ...> : public writer_print_helper<Ts ...> {
private:
   typedef writer_print_helper<Ts ...> wph_base;

public:
   /*! Constructor.

   @param ptw
      Text writer to use.
   @param sFormat
      Format string to parse for replacements.
   @param t0
      First replacement value.
   @param ts
      Remaining replacement values.
   */
   writer_print_helper(writer * ptw, istr const & sFormat, T0 const & t0, Ts const &... ts) :
      wph_base(ptw, sFormat, ts ...),
      m_t0(t0) {
   }

   //! See writer_print_helper<>::run().
   void run() {
      while (wph_base::write_format_up_to_next_repl()) {
         // Perform and write the replacement.
         write_repl(wph_base::m_iSubstArg);
      }
   }

protected:
   //! See writer_print_helper<>::write_repl().
   void write_repl(unsigned iArg) {
      if (iArg == 0) {
         to_str_backend<T0> tsb;
         tsb.set_format(istr(
            external_buffer, wph_base::m_pchReplFormatSpecBegin, static_cast<std::size_t>(
               wph_base::m_pchReplFormatSpecEnd - wph_base::m_pchReplFormatSpecBegin
            )
         ));
         tsb.write(m_t0, wph_base::m_ptw);
      } else {
         // Recurse to the previous level.
         wph_base::write_repl(iArg - 1);
      }
   }

private:
   //! Nth replacement.
   T0 const & m_t0;
};

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

// Recursion step: extract one argument, recurse with the rest.
template <
   typename T0 = void, typename T1 = void, typename T2 = void, typename T3 = void,
   typename T4 = void, typename T5 = void, typename T6 = void, typename T7 = void,
   typename T8 = void, typename T9 = void
>
class writer_print_helper : public writer_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> {
private:
   typedef writer_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> wph_base;

public:
   /*! Constructor.

   @param ptw
      Text writer to write to.
   @param sFormat
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
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0
   ) :
      wph_base(ptw, sFormat),
      m_t0(t0) {
   }
   template <typename U0, typename U1>
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1
   ) :
      wph_base(ptw, sFormat, t1),
      m_t0(t0) {
   }
   template <typename U0, typename U1, typename U2>
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2
   ) :
      wph_base(ptw, sFormat, t1, t2),
      m_t0(t0) {
   }
   template <typename U0, typename U1, typename U2, typename U3>
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3
   ) :
      wph_base(ptw, sFormat, t1, t2, t3),
      m_t0(t0) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4>
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4),
      m_t0(t0) {
   }
   template <typename U0, typename U1, typename U2, typename U3, typename U4, typename U5>
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4, t5),
      m_t0(t0) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6
   >
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
      U6 const & t6
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4, t5, t6),
      m_t0(t0) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7
   >
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
      U6 const & t6, U7 const & t7
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4, t5, t6, t7),
      m_t0(t0) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8
   >
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
      U6 const & t6, U7 const & t7, U8 const & t8
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4, t5, t6, t7, t8),
      m_t0(t0) {
   }
   template <
      typename U0, typename U1, typename U2, typename U3, typename U4, typename U5, typename U6,
      typename U7, typename U8, typename U9
   >
   writer_print_helper(
      typename std::enable_if<!std::is_void<U0>::value, writer *>::type ptw, istr const & sFormat,
      U0 const & t0, U1 const & t1, U2 const & t2, U3 const & t3, U4 const & t4, U5 const & t5,
      U6 const & t6, U7 const & t7, U8 const & t8, U9 const & t9
   ) :
      wph_base(ptw, sFormat, t1, t2, t3, t4, t5, t6, t7, t8, t9),
      m_t0(t0) {
   }

   //! See writer_print_helper<>::run().
   void run() {
      while (wph_base::write_format_up_to_next_repl()) {
         // Perform and write the replacement.
         write_repl(wph_base::m_iSubstArg);
      }
   }

protected:
   //! See writer_print_helper<>::write_repl().
   void write_repl(unsigned iArg) {
      if (iArg == 0) {
         to_str_backend<T0> tsb;
         tsb.set_format(istr(
            external_buffer, wph_base::m_pchReplFormatSpecBegin, static_cast<std::size_t>(
               wph_base::m_pchReplFormatSpecEnd - wph_base::m_pchReplFormatSpecBegin
            )
         ));
         tsb.write(m_t0, wph_base::m_ptw);
      } else {
         // Recurse to the previous level.
         wph_base::write_repl(iArg - 1);
      }
   }

private:
   //! Nth replacement.
   T0 const & m_t0;
};

// Base recursion step: no arguments to replace.
template <>
class writer_print_helper<> : public writer_print_helper_impl {
public:
   /*! Constructor.

   @param ptw
      Text writer to write to.
   @param sFormat
      Format string to parse for replacements.
   */
   writer_print_helper(writer * ptw, istr const & sFormat) :
      writer_print_helper_impl(ptw, sFormat) {
   }

protected:
   /*! Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

   @param iArg
      0-based index of the template argument to write.
   */
   ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
      /* This is the last recursion stage, with no replacements available, so if we got here
      writer::print() was called with insufficient replacements for the given format string. */
      ABC_UNUSED_ARG(iArg);
      writer_print_helper_impl::throw_index_error();
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}}}} //namespace abc::io::text::detail

// Now it’s possible to implement this.

namespace abc { namespace io { namespace text {

#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename... Ts>
inline void writer::print(istr const & sFormat, Ts const &... ts) {
   detail::writer_print_helper<Ts ...> wph(this, sFormat, ts ...);
   wph.run();
}

#else //ifdef ABC_CXX_VARIADIC_TEMPLATES

inline void writer::print(istr const & sFormat) {
   detail::writer_print_helper<> wph(this, sFormat);
   wph.run();
}
template <typename T0>
inline void writer::print(istr const & sFormat, T0 const & t0) {
   detail::writer_print_helper<T0> wph(this, sFormat, t0);
   wph.run();
}
template <typename T0, typename T1>
inline void writer::print(istr const & sFormat, T0 const & t0, T1 const & t1) {
   detail::writer_print_helper<T0, T1> wph(this, sFormat, t0, t1);
   wph.run();
}
template <typename T0, typename T1, typename T2>
inline void writer::print(istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2) {
   detail::writer_print_helper<T0, T1, T2> wph(this, sFormat, t0, t1, t2);
   wph.run();
}
template <typename T0, typename T1, typename T2, typename T3>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3
) {
   detail::writer_print_helper<T0, T1, T2, T3> wph(this, sFormat, t0, t1, t2, t3);
   wph.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4> wph(this, sFormat, t0, t1, t2, t3, t4);
   wph.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4, T5> wph(this, sFormat, t0, t1, t2, t3, t4, t5);
   wph.run();
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4, T5, T6> wph(
      this, sFormat, t0, t1, t2, t3, t4, t5, t6
   );
   wph.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7
>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4, T5, T6, T7> wph(
      this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7
   );
   wph.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8
>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8> wph(
      this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8
   );
   wph.run();
}
template <
   typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6,
   typename T7, typename T8, typename T9
>
inline void writer::print(
   istr const & sFormat, T0 const & t0, T1 const & t1, T2 const & t2, T3 const & t3, T4 const & t4,
   T5 const & t5, T6 const & t6, T7 const & t7, T8 const & t8, T9 const & t9
) {
   detail::writer_print_helper<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> wph(
      this, sFormat, t0, t1, t2, t3, t4, t5, t6, t7, t8, t9
   );
   wph.run();
}

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

}}} //namespace abc::io::text

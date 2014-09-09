/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010, 2011, 2012, 2013, 2014
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

#ifndef _ABACLADE_HXX
   #error Please #include <abaclade.hxx> instead of this file
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::base


namespace abc {
namespace io {
namespace text {

//! Base interface for text (character-based) I/O.
class ABACLADE_SYM base {
public:

   //! Destructor.
   virtual ~base();


   /*! Returns the encoding of the data store.

   return
      Text encoding.
   */
   virtual abc::text::encoding get_encoding() const = 0;


   /*! Returns the line terminator used in the data store.

   return
      Line terminator.
   */
   abc::text::line_terminator get_line_terminator() const {
      return m_lterm;
   }


   /*! Assigns a new line terminator that will be used for all following writes.

   lterm
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
   meaning as line_terminator::host.
   */
   abc::text::line_terminator m_lterm;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::reader


namespace abc {
namespace io {
namespace text {

//! Interface for text (character-based) input.
class ABACLADE_SYM reader :
   public virtual base {
public:

   /*! Reads the entire source into the specified mutable string.

   psDst
      Pointer to the string that will receive the data.
   */
   void read_all(mstr * psDst);


   /*! Reads a whole line into the specified mutable string, discarding the line terminator.

   psDst
      Pointer to the string that will receive the read line.
   return
      true if a line could be read, or false if the end of the data was reached, in which case
      *psDst is left in an undetermined state.
   */
   bool read_line(mstr * psDst);


   /*! Reads data into the specified mutable string, invoking a callback function to determine how
   much of the read data should be consumed.

   psDst
      Pointer to the string that will receive the read data.
   fnGetConsumeEnd
      If non-null, this callback is invoked at least once for each internal read.
      sRead
         String containing every character read by this invocation of read_while() up to this point.
      itLastReadBegin
         Iterator to the start of the last read portion of sRead. The callback can use this to avoid
         re-scanning portions of the string that it has seen before.
      return
         Iterator to beyond the last character to be consumed, i.e. that the callback wants
         read_while() to include in the destination string (*psDst).
   return
      true if a string could be read, or false if the end of the data was reached, in which case
      *psDst is left in an undetermined state.
   */
   virtual bool read_while(mstr * psDst, std::function<
      istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
   > const & fnGetConsumeEnd) = 0;


protected:

   //! See base::base().
   reader();
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::writer


namespace abc {
namespace io {
namespace text {

/*! DOC:7103 abc::io::text::writer::print()

Designed after Python’s str.format(), abc::io::text::writer::print() allows to combine objects
together as strings using a format string.

The implementation of print() is entirely contained in abc::io::text::detail::writer_print_helper,
which accesses the individual arguments in a recursive way, from the most-derived class down to the
base class, which also contains most of the implementation. Combined with the usage of [DOC:3984
abc::to_str() and abc::to_str_backend()], this enables a type-safe variadic alternative to C’s
printf, and voids the requirement for explicit specification of the argumment types (such as %d,
%s), much like Python’s str.format().

Because of its type-safety, print() is also the core of [DOC:8503 Stack tracing], because it allows
to print a variable by automatically deducing its type.

The format string passed as first argument to abc::io::text::writer::print() can contain
“replacement fields” delimited by curly braces (‘{’ and ‘}’). Anything not contained in curly braces
is considered literal text and emitted as-is; the only exceptions are the substrings “{{” and “}}”,
which allow to print “{” and “}” respectively.

A replacement field can specify an argument index; if omitted, the argument used will be the one
following the last used one, or the first if no arguments have been used up to that point. After the
optional argument index, an optional type-dependent format specification can be indicated; this will
be passed as-is to the specialization of abc::to_str_backend for the selected argument.

Grammar for a replacement field:

   replacement_field : “{” index? ( “:” format_spec )? “}”
   index             : [0-9]+
   format_spec       : <type-specific format specification>

Basic usage examples for index:

   "Welcome to {0}"                 Use argument 0
   "Please see items {}, {3}, {}"   Use argument 0, skip 1 and 2, use 3 and 4

Reference for Python’s str.format(): <http://docs.python.org/3/library/string.html#format-string-
syntax>
*/

//! Interface for binary (non-text) output.
class ABACLADE_SYM writer :
   public virtual base {
public:

   /*! Writes multiple values combined together in the specified format.

   sFormat
      Format string to parse for replacements.
   ts
      Replacement values.
   */
#ifdef ABC_CXX_VARIADIC_TEMPLATES
   template <typename ... Ts>
   void print(istr const & sFormat, Ts const & ... ts);
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

   t
      Value to write.
   */
   template <typename T>
   void write(T const & t) {
      to_str_backend<T> tsb;
      tsb.write(t, this);
   }


   /*! Writes the contents of a memory buffer, first translating them to the text writer’s character
   encoding, if necessary.

   pSrc
      Pointer to the buffer to write.
   cbSrc
      Size of the buffer, in bytes.
   enc
      Encoding used by the buffer. If different from the writer’s encoding, a conversion will be
      performed on the fly.
   */
   virtual void write_binary(void const * pSrc, std::size_t cbSrc, abc::text::encoding enc) = 0;


   /*! Writes a string followed by a new-line.

   s
      String to write.
   */
   void write_line(istr const & s);


protected:

   //! See base::base().
   writer();
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::detail::writer_print_helper


namespace abc {
namespace io {
namespace text {
namespace detail {

//! Template-free implementation of abc::io::text::detail::writer_print_helper.
class ABACLADE_SYM writer_print_helper_impl :
   public noncopyable {
public:

   /*! Constructor.

   ptw
      Text writer to use.
   sFormat
      Format string to parse for replacements.
   */
   writer_print_helper_impl(writer * ptw, istr const & sFormat);


   /*! Writes the provided arguments to the target text writer, performing replacements as
   necessary.
   */
   void run();


protected:

   /*! Throws an instance of abc::index_error(), providing the invalid replacement index found in
   the format string.
   */
   ABC_FUNC_NORETURN void throw_index_error();


   /*! Writes the portion of format string between m_itFormatToWriteBegin and the next replacement
   and returns true, or writes the remaining characters of the format string and returns false if no
   more replacement are found.

   return
      true if another replacement was found and should be printed, or false otherwise.
   */
   bool write_format_up_to_next_repl();


private:

   /*! Throws an instance of abc::syntax_error(), providing accurate context information.

   sDescription
      Error description.
   it
      Position of the offending character in m_sFormat.
   */
   ABC_FUNC_NORETURN void throw_syntax_error(
      istr const & sDescription, istr::const_iterator it
   ) const;


   /*! Writes the portion of format string between the first character to be written
   (m_itFormatToWriteBegin) and the specified one, and updates m_itFormatToWriteBegin.

   itUpTo
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

template <typename ... Ts>
class writer_print_helper;

// Base recursion step: no arguments to replace.
template <>
class writer_print_helper<> :
   public writer_print_helper_impl {
public:

   /*! Constructor.

   ptw
      Text writer to use.
   sFormat
      Format string to parse for replacements.
   */
   writer_print_helper(writer * ptw, istr const & sFormat) :
      writer_print_helper_impl(ptw, sFormat) {
   }


protected:

   /*! Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

   iArg
      0-based index of the template argument to write.
   */
   ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
      // This is the last recursion stage, with no replacements available, so if we got here
      // writer::print() was called with insufficient replacements for the given format string.
      ABC_UNUSED_ARG(iArg);
      writer_print_helper_impl::throw_index_error();
   }
};

// Recursion step: extract one argument, recurse with the rest.
template <typename T0, typename ... Ts>
class writer_print_helper<T0, Ts ...> :
   public writer_print_helper<Ts ...> {

   typedef writer_print_helper<Ts ...> wph_base;

public:

   /*! Constructor.

   ptw
      Text writer to use.
   sFormat
      Format string to parse for replacements.
   t0
      First replacement value.
   ts
      Remaining replacement values.
   */
   writer_print_helper(writer * ptw, istr const & sFormat, T0 const & t0, Ts const & ... ts) :
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
         tsb.set_format(istr(unsafe, wph_base::m_pchReplFormatSpecBegin, static_cast<std::size_t>(
            wph_base::m_pchReplFormatSpecEnd - wph_base::m_pchReplFormatSpecBegin
         )));
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
class writer_print_helper :
   public writer_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> {

   typedef writer_print_helper<T1, T2, T3, T4, T5, T6, T7, T8, T9> wph_base;

public:

   /*! Constructor.

   ptw
      Text writer to write to.
   sFormat
      Format string to parse for replacements.
   t0
      First replacement value.
   t1
      Second replacement value.
   t2
      Third replacement value.
   t3
      Fourth replacement value.
   t4
      Fifth replacement value.
   t5
      Sixth replacement value.
   t6
      Seventh replacement value.
   t7
      Eighth replacement value.
   t8
      Ninth replacement value.
   t9
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
         tsb.set_format(istr(unsafe, wph_base::m_pchReplFormatSpecBegin, static_cast<std::size_t>(
            wph_base::m_pchReplFormatSpecEnd - wph_base::m_pchReplFormatSpecBegin
         )));
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
class writer_print_helper<> :
   public writer_print_helper_impl {
public:

   /*! Constructor.

   ptw
      Text writer to write to.
   sFormat
      Format string to parse for replacements.
   */
   writer_print_helper(writer * ptw, istr const & sFormat) :
      writer_print_helper_impl(ptw, sFormat) {
   }


protected:

   /*! Writes T0 if iArg == 0, or fowards the call to the previous recursion level.

   iArg
      0-based index of the template argument to write.
   */
   ABC_FUNC_NORETURN void write_repl(unsigned iArg) {
      // This is the last recursion stage, with no replacements available, so if we got here
      // writer::print() was called with insufficient replacements for the given format string.
      ABC_UNUSED_ARG(iArg);
      writer_print_helper_impl::throw_index_error();
   }
};

#endif //ifdef ABC_CXX_VARIADIC_TEMPLATES … else

} //namespace detail


// Now it’s possible to implement this.
#ifdef ABC_CXX_VARIADIC_TEMPLATES

template <typename ... Ts>
inline void writer::print(istr const & sFormat, Ts const & ... ts) {
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

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_base


namespace abc {
namespace io {
namespace text {

//! Base for text I/O classes built on top of a binary::buffered_base instance.
class ABACLADE_SYM binbuf_base :
   public virtual base {
public:

   //! Destructor.
   virtual ~binbuf_base() /*override*/;

   //! Returns a pointer to the underlying buffered binary I/O object.
   virtual std::shared_ptr<binary::buffered_base> buffered_base() const = 0;

   //! See base::get_encoding().
   virtual abc::text::encoding get_encoding() const /*override*/;


protected:

   /*! Constructor.

   enc
      Initial value for get_encoding().
   */
   binbuf_base(abc::text::encoding enc);


protected:

   /*! Encoding used for I/O to/from the underlying buffered_base. If not explicitly set, it will be
   automatically determined and assigned on the first read or write.
   */
   abc::text::encoding m_enc;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_reader


namespace abc {
namespace io {
namespace text {

//! Implementation of a text (character-based) reader on top of a binary::buffered_reader instance.
class ABACLADE_SYM binbuf_reader :
   public virtual binbuf_base,
   public virtual reader {
public:

   /*! Constructor.

   pbbr
      Pointer to a binary buffered reader to work with.
   enc
      Initial value for get_encoding(). If omitted, an encoding will be automatically detected
      (guessed) on the first read from the underlying binary reader.
   */
   explicit binbuf_reader(
      std::shared_ptr<binary::buffered_reader> pbbr,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_reader() /*override*/;

   //! See binbuf_base::buffered_base().
   virtual std::shared_ptr<binary::buffered_base> buffered_base() const /*override*/;

   //! See reader::read_while().
   virtual bool read_while(mstr * psDst, std::function<
      istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
   > const & fnGetConsumeEnd) /*override*/;


private:

   /*! Invokes the consumer function translating its arguments and translating back and validating
   its return value.

   pchBegin
      Pointer to the buffer from which characters should be consumed.
   pchOffset
      Pointer to the end in *pchBegin of any previously consumed characters.
   cch
      Size of the buffer pointed to by pchBegin, in characters.
   fnGetConsumeEnd
      See read_while()’s fnGetConsumeEnd argument.
   return
      Pointer returned by fnGetConsumeEnd.
   */
   static char_t const * call_get_consume_end(
      char_t const * pchBegin, char_t const * pchOffset, std::size_t cch, std::function<
         istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
      > const & fnGetConsumeEnd
   );


   /*! Detects the encoding used in the source.

   pb
      Pointer to a buffer with the initial contents of the file.
   cb
      Size of the buffer pointed to by pb.
   return
      Size of the BOM, if found in the source. If non-zero, the caller should discard this many
      bytes from the provided buffer.
   */
   std::size_t detect_encoding(std::int8_t const * pb, std::size_t cb);


   /*! Implementation of read_while() for the source encoding == host encoding.

   pb
      Pointer to a buffer with the initial contents of the file.
   pcb
      Pointer to the size of the buffer pointed to by pb; on return it will contain the count of
      bytes remaining in the last peek buffer.
   psDst
      Pointer to the string that will receive the read data.
   fnGetConsumeEnd
      See read_while()’s fnGetConsumeEnd argument.
   return
      Count of characters read into *psDst.
   */
   std::size_t read_while_host_encoding(
      std::int8_t const * pbSrc, std::size_t * pcbSrc, mstr * psDst, std::function<
         istr::const_iterator (istr const & sRead, istr::const_iterator itLastReadBegin)
      > const & fnGetConsumeEnd
   );


protected:

   //! Underlying binary buffered reader.
   std::shared_ptr<binary::buffered_reader> m_pbbr;


private:

   /*! If true and m_lterm is line_terminator::any or line_terminator::convert_any_to_lf, and the
   next read operation encounters a leading ‘\n’, that character will not be considered as a line
   terminator; this way, even if a “\r\n” was broken into multiple reads, we’ll still present
   clients with a single ‘\n’ character instead of two, as it would happen without this tracker (one
   from the trailing ‘\r’ of the first read, one from the leading ‘\n’ of the second.
   */
   bool m_bDiscardNextLF:1;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io::text::binbuf_writer


namespace abc {
namespace io {
namespace text {

//! Implementation of a text (character-based) writer on top of a binary::buffered_writer instance.
class ABACLADE_SYM binbuf_writer :
   public virtual binbuf_base,
   public virtual writer {
public:

   /*! Constructor.

   pbbw
      Pointer to a binary buffered writer to work with.
   enc
      Initial value for get_encoding(). If omitted and never explicitly set, on the first write it
      will default to abc::text::encoding::utf8.
   */
   explicit binbuf_writer(
      std::shared_ptr<binary::buffered_writer> pbbw,
      abc::text::encoding enc = abc::text::encoding::unknown
   );

   //! Destructor.
   virtual ~binbuf_writer() /*override*/;

   //! See binbuf_base::buffered_base().
   virtual std::shared_ptr<binary::buffered_base> buffered_base() const /*override*/;

   //! See writer::write_binary().
   virtual void write_binary(
      void const * pSrc, std::size_t cbSrc, abc::text::encoding enc
   ) /*override*/;


protected:

   //! Underlying binary buffered writer.
   std::shared_ptr<binary::buffered_writer> m_pbbw;
};

} //namespace text
} //namespace io
} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


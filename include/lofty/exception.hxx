/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2017 Raffaello D. Di Napoli

This file is part of Lofty.

Lofty is free software: you can redistribute it and/or modify it under the terms of version 2.1 of the GNU
Lesser General Public License as published by the Free Software Foundation.

Lofty is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for
more details.
------------------------------------------------------------------------------------------------------------*/

/*! @file
Basic exception classes and related macros. */

#ifndef _LOFTY_HXX_INTERNAL
   #error "Please #include <lofty.hxx> instead of this file"
#endif

/*! @page exception-classes Exception classes
Lofty’s exception class hierarchy. These classes provide diverse and semantically-rich types that complement
the hirearchy provided by the STL. See doc/Exception_class_hierarchy.fodg for a diagram of the entire Lofty
exception class hierarchy.

See also LOFTY_THROW() for more information on throwing Lofty exceptions.

Reference for Python’s exception class hierarchy: <http://docs.python.org/3.2/library/exceptions.html>. */


//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Integer type used by the OS to represent error numbers.
#if LOFTY_HOST_API_POSIX
   typedef int errint_t;
#elif LOFTY_HOST_API_WIN32
   typedef ::DWORD errint_t;
#else
   #error "TODO: HOST_API"
#endif

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty { namespace _pvt {

//! Stores the source code location for a scope_trace instance.
struct source_file_address_data {
   //! Function name.
   char_t const * function;
   //! Address in the file.
   text::_pvt::file_address_data file_addr_data;
};

}} //namespace lofty::_pvt

namespace lofty {

//! Stores the source code location for a scope_trace instance.
class source_file_address : protected _pvt::source_file_address_data {
public:
   //! Default constructor.
   source_file_address() {
      _pvt::source_file_address_data::function = nullptr;
      file_addr_data.file_path_ = nullptr;
      file_addr_data.line_number_ = 0;
   }

   /*! Constructor.

   @param function_
      Name of the function.
   @param file_path_
      Path to the source file.
   @param line_number_
      Line number in *file_path.
   */
   source_file_address(char_t const * function_, char_t const * file_path_, unsigned line_number_) {
      _pvt::source_file_address_data::function = function_;
      file_addr_data.file_path_ = file_path_;
      file_addr_data.line_number_ = line_number_;
   }

   /*! Returns a pointer to the contained data-only struct.

   @return
      Pointer to the contained _pvt::source_file_address_data.
   */
   _pvt::source_file_address_data const * data() const {
      return this;
   }

   /*! Returns the contained file address, identifying file path and line number.

   @return
      Contained file address.
   */
   text::file_address const & file_address() const {
      return *text::file_address::from_data(&file_addr_data);
   }

   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return file_addr_data.file_path_;
   }

   /*! Returns a pointer to an instance of this class from a pointer to the data-only struct.

   @param source_file_addr_data
      Pointer to a data-only struct.
   @return
      Pointer to the equivalent source_file_address instance.
   */
   static source_file_address const * from_data(
      _pvt::source_file_address_data const * source_file_addr_data
   ) {
      return static_cast<source_file_address const *>(source_file_addr_data);
   }

   /*! Returns the function name.

   @return
      Function name.
   */
   char_t const * function() const {
      return _pvt::source_file_address_data::function;
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return file_addr_data.line_number_;
   }
};

} //namespace lofty

//! Pretty-printed name of the current function.
#if LOFTY_HOST_CXX_CLANG || LOFTY_HOST_CXX_GCC
   /* Can’t use LOFTY_SL(__PRETTY_FUNCTION__) because __PRETTY_FUNCTION__ is expanded by the compiler, not the
   preprocessor; this causes LOFTY_SL(__PRETTY_FUNCTION__) to incorrectly expand to u8__PRETTY_FUNCTION__.
   However these compilers will encode __PRETTY_FUNCTION__ using UTF-8, which makes LOFTY_SL() unnecessary, so
   just avoid using it here. */
   #define LOFTY_THIS_FUNC \
      __PRETTY_FUNCTION__
#elif LOFTY_HOST_CXX_MSC
   /* __FUNCSIG__ is expanded after preprocessing like __PRETTY_FUNCTION__, but for some reason this works
   just fine. */
   #define LOFTY_THIS_FUNC \
      LOFTY_SL(__FUNCSIG__)
#else
   #define LOFTY_THIS_FUNC \
      nullptr
#endif

/*! Expands into a lofty::text::file_address x-value referencing the location in which it’s used.

@return
   lofty::text::file_address instance.
*/
#define LOFTY_THIS_FILE_ADDRESS() \
   (::lofty::text::file_address(LOFTY_SL(__FILE__), __LINE__))

/*! Expands into a lofty::source_file_address x-value referencing the location in which it’s used.

@return
   lofty::source_file_address instance.
*/
#define LOFTY_THIS_SOURCE_FILE_ADDRESS() \
   (::lofty::source_file_address(LOFTY_THIS_FUNC, LOFTY_SL(__FILE__), __LINE__))

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Implementation of LOFTY_THROW(); can be used directly to customize the source of the exception.

@param source_file_addr
   Location at which the exception is being thrown.
@param x
   Exception type to be thrown.
@param args
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by the
   constructor of x.
*/
#define LOFTY_THROW_FROM(source_file_addr, x, args) \
   do { \
      x __x LOFTY_CPP_IF(LOFTY_CPP_LIST_COUNT args, args, ); \
      __x._before_throw(source_file_addr); \
      throw ::lofty::_std::move(__x); \
   } while (false)

/*! Instantiates the specified exception class, fills it up with context information and the remaining
arguments, and then throws it.

This is the recommended way of throwing an exception within code using Lofty. See the documentation for
lofty::exception. Combined with @ref stack-tracing, the use of LOFTY_THROW() augments the stack trace with the
exact line where the throw statement occurred.

Only instances of lofty::exception (or a derived class) can be thrown using LOFTY_THROW(), because of the
additional members that the latter expects to be able to set in the former.

The class lofty::exception implements the actual stack trace printing for lofty::_pvt::scope_trace because
it’s the only class involved that’s not in a _pvt namespace.

@param x
   Exception type to be thrown.
@param args
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by the constructor
   of x.
*/
#define LOFTY_THROW(x, args) \
   LOFTY_THROW_FROM(LOFTY_THIS_SOURCE_FILE_ADDRESS(), x, args)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Base for all of Lofty’s exceptions classes. See @see exception-classes.
class LOFTY_SYM exception : public _std::exception {
public:
   //! List of common exception types, used by several static methods.
   LOFTY_ENUM_AUTO_VALUES(common_type,
      none,

      execution_interruption,
      process_exit,
      process_interruption,
      user_forced_interruption,

      math_arithmetic_error,
      math_division_by_zero,
      math_floating_point_error,
      math_overflow,
      memory_bad_pointer,
      memory_bad_pointer_alignment
   );

public:
   //! Default Constructor.
   exception();

   /*! Copy constructor.

   @param src
      Source object.
   */
   exception(exception const & src);

   //! Destructor.
   virtual ~exception() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   exception & operator=(exception const & src);

   /*! Prepares the exception for throwing.

   @param source_file_addr
      Location at which the exception is being thrown.
   */
   void _before_throw(source_file_address const & source_file_addr);

#if LOFTY_HOST_API_MACH || LOFTY_HOST_API_POSIX
   /*! Injects the requested type of exception in the specified context.

   @param x_type
      Type of exception to inject.
   @param arg0
      First argument to the exception constructor, if applicable.
   @param arg1
      Second argument to the exception constructor, if applicable.
   @param os_context
      Pointer to an OS-specific context struct.
   */
   static void inject_in_context(
      common_type x_type, std::intptr_t arg0, std::intptr_t arg1, void * os_context
   );
#endif

   /*! Throws an exception of the specified type.

   @param x_type
      Type of exception to be throw.
   @param arg0
      Exception type-specific argument 0.
   @param arg1
      Exception type-specific argument 1.
   */
   static void throw_common_type(common_type::enum_type x_type, std::intptr_t arg0, std::intptr_t arg1);

#if LOFTY_HOST_API_POSIX || LOFTY_HOST_API_WIN32
   //! Throws an exception matching the last error reported by the OS.
   static LOFTY_FUNC_NORETURN void throw_os_error();

   /*! Throws an exception matching a specified OS-defined error.

   @param err
      OS-defined error number.
   */
   static LOFTY_FUNC_NORETURN void throw_os_error(errint_t err);
#endif

   /*! Returns the common_type value that best matches the type of the specified exception, which may or may
   not be an execution_interruption instance.

   @param x
      Pointer to an exception whose type shall be inspected to determine which common_type matches it most
      closely. If omitted, the default value of common_type::execution_interruption is returned.
   @return
      Exception type. May be nullptr to indicate that the caught exception is not an std::exception instance.
   */
   static common_type execution_interruption_to_common_type(_std::exception const * x = nullptr);

   /*! See std::exception::what().

   @return
      Name of the exception class.
   */
   virtual const char * what() const LOFTY_STL_NOEXCEPT_TRUE() override;

   /*! Writes detailed information about an exception, as well as any scope/stack trace generated up to the
   point of the call to this function.

   @param dst
      Pointer to the stream to output to. If omitted, the scope/stack trace will be written to stderr.
   @param std_x
      Caught exception.
   */
   static void write_with_scope_trace(
      io::text::ostream * dst = nullptr, _std::exception const * std_x = nullptr
   );

protected:
   /*! Returns a stream that can be used to add information to the exception. The space available for writing
   in the stream is limited, and should be used intelligently.

   @return
      Output stream instance.
   */
   io::text::char_ptr_ostream what_ostream();

private:
   //! Source location.
   source_file_address source_file_addr;
   //! true if *this is an in-flight exception (it has been thrown) or is a copy of one.
   bool in_flight;
   //! Characters available in what_buf.
   std::size_t what_buf_available;
   /*! Buffer for the string returned by what(); used as the buffer for the string backing the stream returned
   by what_ostream(). */
   char what_buf[256];
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Verifies a condition at runtime, throwing a assertion_error exception if the assertion turns out to be
incorrect.

@param expr
   Expression to be validated.
@param msg
   Message to be displayed if the assertion fails.
*/
#ifdef DEBUG
   #define LOFTY_ASSERT(expr, msg) \
      do { \
         if (!(expr)) { \
            lofty::assertion_error::_assertion_failed( \
               LOFTY_THIS_SOURCE_FILE_ADDRESS(), LOFTY_SL(#expr), msg \
            ); \
         } \
      } while (false)
#else
   #define LOFTY_ASSERT(expr, msg) \
      static_cast<void>(0)
#endif

//! An assertion failed.
class LOFTY_SYM assertion_error : public exception {
public:
   //! Throws an exception of type ab::assertion_error due to an expression failing validation.
   static LOFTY_FUNC_NORETURN void _assertion_failed(
      source_file_address const & source_file_addr, str const & expr, str const & msg
   );

protected:
   /*! Set to true for the duration of the execution of _assertion_failed(). If another assertion fails due to
   code executed during the call to _assertion_failed(), the latter will just throw, without printing
   anything; otherwise we’ll most likely get stuck in an infinite recursion. */
   static coroutine_local_value<bool> reentering;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Execution interruption. May affect a single thread/coroutine or the whole program.
class LOFTY_SYM execution_interruption : public exception {
public:
   //! Default constructor.
   execution_interruption(/*source?*/);

   /*! Copy constructor.

   @param src
      Source object.
   */
   execution_interruption(execution_interruption const & src);

   //! Destructor.
   virtual ~execution_interruption() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   execution_interruption & operator=(execution_interruption const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Thrown in coroutines and threads that are still running when lofty::app:main() returns causing the process
to terminate (exit), to force them to return as well. */
class LOFTY_SYM process_exit : public execution_interruption {
public:
   //! Default constructor.
   process_exit();

   /*! Copy constructor.

   @param src
      Source object.
   */
   process_exit(process_exit const & src);

   //! Destructor.
   virtual ~process_exit() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   process_exit & operator=(process_exit const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Interruption in the execution of the whole process, typically requested by the user. Raised simultaneously
in every coroutine and thread. */
class LOFTY_SYM process_interruption : public execution_interruption {
public:
   //! Default constructor.
   process_interruption();

   /*! Copy constructor.

   @param src
      Source object.
   */
   process_interruption(process_interruption const & src);

   //! Destructor.
   virtual ~process_interruption() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   process_interruption & operator=(process_interruption const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

/*! Execution interruption requested by the user, resulting in the termination of all coroutines and threads
in the process. */
class LOFTY_SYM user_forced_interruption : public process_interruption {
public:
   //! Default constructor.
   user_forced_interruption();

   /*! Copy constructor.

   @param src
      Source object.
   */
   user_forced_interruption(user_forced_interruption const & src);

   //! Destructor.
   virtual ~user_forced_interruption() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   user_forced_interruption & operator=(user_forced_interruption const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Base for all error-related exceptions classes.
class LOFTY_SYM generic_error : public exception {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit generic_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   generic_error(generic_error const & src);

   //! Destructor.
   virtual ~generic_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   generic_error & operator=(generic_error const & src);

   /*! Returns the OS-defined error number, if any.

   @return
      OS-defined error number.
   */
   errint_t os_error() const {
      return err;
   }

private:
   //! OS-specific error wrapped by this exception.
   errint_t err;
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! A function/method received an argument that had an inappropriate value.
class LOFTY_SYM argument_error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit argument_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   argument_error(argument_error const & src);

   //! Destructor.
   virtual ~argument_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   argument_error & operator=(argument_error const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! Invalid value provided for a variable/argument.
class LOFTY_SYM domain_error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit domain_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   domain_error(domain_error const & src);

   //! Destructor.
   virtual ~domain_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   domain_error & operator=(domain_error const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! A network-related error occurred.
class LOFTY_SYM network_error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit network_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   network_error(network_error const & src);

   //! Destructor.
   virtual ~network_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   network_error & operator=(network_error const & src);
};

} //namespace lofty

//////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace lofty {

//! An operation failed to prevent a security hazard.
class LOFTY_SYM security_error : public generic_error {
public:
   /*! Constructor.

   @param err
      OS-defined error number associated to the exception.
   */
   explicit security_error(errint_t err = 0);

   /*! Copy constructor.

   @param src
      Source object.
   */
   security_error(security_error const & src);

   //! Destructor.
   virtual ~security_error() LOFTY_STL_NOEXCEPT_TRUE();

   /*! Copy-assignment operator.

   @param src
      Source object.
   @return
      *this.
   */
   security_error & operator=(security_error const & src);
};

} //namespace lofty

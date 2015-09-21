/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2010-2015 Raffaello D. Di Napoli

This file is part of Abaclade.

Abaclade is free software: you can redistribute it and/or modify it under the terms of the GNU
Lesser General Public License as published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.

Abaclade is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser
General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with Abaclade. If
not, see <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------------------------------------*/

/*! @file
Basic exception classes and related macros. */

#ifndef _ABACLADE_HXX_INTERNAL
   #error "Please #include <abaclade.hxx> instead of this file"
#endif

/*! @page exception-classes Exception classes
Abaclade’s exception class hierarchy. These classes provide diverse and semantically-rich types that
complement the hirearchy provided by the STL. See doc/Exception_class_hierarchy.fodg for a diagram
of the entire Abaclade exception class hierarchy.

See also ABC_THROW() for more information on throwing Abaclade exceptions.

Reference for Python’s exception class hierarchy: <http://docs.python.org/3.2/library/
exceptions.html>. */


////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Integer type used by the OS to represent error numbers.
#if ABC_HOST_API_POSIX
   typedef int errint_t;
#elif ABC_HOST_API_WIN32
   typedef ::DWORD errint_t;
#else
   #error "TODO: HOST_API"
#endif

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

//! Stores the source code location for a scope_trace instance.
struct source_file_address_data {
   //! Function name.
   char_t const * m_pszFunction;
   //! Address in the file.
   text::detail::file_address_data m_tfad;
};

}} //namespace abc::detail

namespace abc {

//! Stores the source code location for a scope_trace instance.
class source_file_address : protected detail::source_file_address_data {
public:
   //! Default constructor.
   source_file_address() {
      m_pszFunction = nullptr;
      m_tfad.m_pszFilePath = nullptr;
      m_tfad.m_iLine = 0;
   }

   /*! Constructor.

   @param pszFunction
      Name of the function.
   @param pszFilePath
      Path to the source file.
   @param iLine
      Line number in *pszFilePath.
   */
   source_file_address(char_t const * pszFunction, char_t const * pszFilePath, unsigned iLine) {
      m_pszFunction = pszFunction;
      m_tfad.m_pszFilePath = pszFilePath;
      m_tfad.m_iLine = iLine;
   }

   /*! Returns a pointer to the contained data-only struct.

   @return
      Pointer to the contained detail::source_file_address_data.
   */
   detail::source_file_address_data const * data() const {
      return this;
   }

   /*! Returns the contained file address, identifying file path and line number.

   @return
      Contained file address.
   */
   text::file_address const & file_address() const {
      return *text::file_address::from_data(&m_tfad);
   }

   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return m_tfad.m_pszFilePath;
   }

   /*! Returns a pointer to an instance of this class from a pointer to the data-only struct.

   @param psfad
      Pointer to a data-only struct.
   @return
      Pointer to the equivalent source_file_address instance.
   */
   static source_file_address const * from_data(detail::source_file_address_data const * psfad) {
      return static_cast<source_file_address const *>(psfad);
   }

   /*! Returns the function name.

   @return
      Function name.
   */
   char_t const * function() const {
      return m_pszFunction;
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return m_tfad.m_iLine;
   }
};

} //namespace abc

//! Pretty-printed name of the current function.
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   /* Can’t use ABC_SL(__PRETTY_FUNCTION__) because __PRETTY_FUNCTION__ is expanded by the compiler,
   not the preprocessor; this causes ABC_SL(__PRETTY_FUNCTION__) to incorrectly expand to
   u8__PRETTY_FUNCTION__. However these compilers will encode __PRETTY_FUNCTION__ using UTF-8, which
   makes ABC_SL() unnecessary, so just avoid using it here. */
   #define ABC_THIS_FUNC \
      __PRETTY_FUNCTION__
#elif ABC_HOST_CXX_MSC
   /* __FUNCSIG__ is expanded after preprocessing like __PRETTY_FUNCTION__, but for some reason this
   works just fine. */
   #define ABC_THIS_FUNC \
      ABC_SL(__FUNCSIG__)
#else
   #define ABC_THIS_FUNC \
      nullptr
#endif

/*! Expands into an abc::text::file_address x-value referencing the location in which it’s used.

@return
   abc::text::file_address instance.
*/
#define ABC_THIS_FILE_ADDRESS() \
   (::abc::text::file_address(ABC_SL(__FILE__), __LINE__))

/*! Expands into an abc::source_file_address x-value referencing the location in which it’s used.

@return
   abc::source_file_address instance.
*/
#define ABC_THIS_SOURCE_FILE_ADDRESS() \
   (::abc::source_file_address(ABC_THIS_FUNC, ABC_SL(__FILE__), __LINE__))

////////////////////////////////////////////////////////////////////////////////////////////////////

/*! Implementation of ABC_THROW(); can be used directly to customize the source of the exception.

@param sfa
   Location at which the exception is being thrown.
@param pszFunction
   Function that is throwing the exception.
@param x
   Exception type to be thrown.
@param info
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by
   x::init().
*/
#define ABC_THROW_FROM(sfa, x, info) \
   do { \
      x __x; \
      __x.init info; \
      __x._before_throw(sfa); \
      throw __x; \
   } while (false)

/*! Instantiates the specified exception class, fills it up with context information and the
remaining arguments, and then throws it.

This is the recommended way of throwing an exception within code using Abaclade. See the
documentation for abc::exception. Combined with @ref stack-tracing, the use of ABC_THROW() augments
the stack trace with the exact line where the throw statement occurred.

Only instances of abc::exception (or a derived class) can be thrown using ABC_THROW(), because of
the additional members that the latter expects to be able to set in the former.

The class abc::exception implements the actual stack trace printing for abc::detail::scope_trace
because it’s the only class involved that’s not in a detail namespace.

@param x
   Exception type to be thrown.
@param info
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by
   x::init().
*/
#define ABC_THROW(x, info) \
   ABC_THROW_FROM(ABC_THIS_SOURCE_FILE_ADDRESS(), x, info)

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Base for all of Abaclade’s exceptions classes. See @see exception-classes.
class ABACLADE_SYM exception : public _std::exception {
public:
   //! List of common exception types, used by several static methods.
   ABC_ENUM_AUTO_VALUES(common_type,
      none,

      app_execution_interruption,
      app_exit_interruption,
      execution_interruption,
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

   @param x
      Source object.
   */
   exception(exception const & x);

   //! Destructor.
   virtual ~exception();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   exception & operator=(exception const & x);

   /*! Stores context information to be displayed if the exception is not caught.

   @param sfa
      Location at which the exception is being thrown.
   */
   void _before_throw(source_file_address const & sfa);

   //! Initializes the information associated to the exception.
   void init() {
   }

#if ABC_HOST_API_MACH || ABC_HOST_API_POSIX
   /*! Injects the requested type of exception in the specified context.

   @param xct
      Type of exception to inject.
   @param iArg0
      First argument to the exception constructor, if applicable.
   @param iArg1
      Second argument to the exception constructor, if applicable.
   @param pvctx
      Pointer to an OS-specific context struct.
   */
   static void inject_in_context(
      common_type xct, std::intptr_t iArg0, std::intptr_t iArg1, void * pvctx
   );
#endif

   /*! Throws an exception of the specified type.

   @param xct
      Type of exception to be throw.
   @param iArg0
      Exception type-specific argument 0.
   @param iArg1
      Exception type-specific argument 1.
   */
   static void throw_common_type(
      common_type::enum_type xct, std::intptr_t iArg0, std::intptr_t iArg1
   );

#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   //! Throws an exception matching the last error reported by the OS.
   static ABC_FUNC_NORETURN void throw_os_error();

   /*! Throws an exception matching a specified OS-defined error.

   @param err
      OS-defined error number.
   */
   static ABC_FUNC_NORETURN void throw_os_error(errint_t err);
#endif

   /*! Returns the common_type value that best matches the type of the specified exception, which
   may or may not be an execution_interruption instance.

   @param px
      Pointer to an exception whose type shall be inspected to determine which common_type matches
      it most closely. If omitted, the default value of common_type::execution_interruption is
      returned.
   @return
      Exception type. May be nullptr to indicate that the caught exception is not an std::exception
      instance.
   */
   static common_type execution_interruption_to_common_type(_std::exception const * px = nullptr);

   /*! See std::exception::what().

   @return
      Name of the exception class.
   */
   virtual const char * what() const ABC_STL_NOEXCEPT_TRUE() override;

   /*! Writes detailed information about an exception, as well as any scope/stack trace generated up
   to the point of the call to this function.

   @param ptwOut
      Pointer to the writer to output to. If omitted, the scope/stack trace will be written to
      stderr.
   @param pstdx
      Caught exception.
   */
   static void write_with_scope_trace(
      io::text::writer * ptwOut = nullptr, _std::exception const * pstdx = nullptr
   );

protected:
   /*! Writes extended information for the exception to the specified text writer.

   @param ptwOut
      Pointer to the writer to output to.
   */
   virtual void write_extended_info(io::text::writer * ptwOut) const;

protected:
   /*! String to be returned by what(). Derived classes can overwrite this instead of overriding the
   entire std::exception::what() method. */
   char const * m_pszWhat;

private:
   //! Source location.
   source_file_address m_sfa;
   //! true if *this is an in-flight exception (it has been thrown) or is a copy of one.
   bool m_bInFlight;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Verifies a condition at runtime, throwing a assertion_error exception if the assertion turns out
to be incorrect.

@param expr
   Expression to be validated.
@param sMsg
   Message to be displayed if the assertion fails.
*/
#ifdef DEBUG
   #define ABC_ASSERT(expr, sMsg) \
      do { \
         if (!(expr)) { \
            abc::assertion_error::_assertion_failed( \
               ABC_THIS_SOURCE_FILE_ADDRESS(), ABC_SL(#expr), sMsg \
            ); \
         } \
      } while (false)
#else
   #define ABC_ASSERT(expr, sMsg) \
      static_cast<void>(0)
#endif

//! An assertion failed.
class ABACLADE_SYM assertion_error : public exception {
public:
   //! Throws an exception of type ab::assertion_error due to an expression failing validation.
   static ABC_FUNC_NORETURN void _assertion_failed(
      source_file_address const & sfa, str const & sExpr, str const & sMsg
   );

protected:
   /*! Set to true for the duration of the execution of _assertion_failed(). If another assertion
   fails due to code executed during the call to _assertion_failed(), the latter will just throw,
   without printing anything; otherwise we’ll most likely get stuck in an infinite recursion. */
   static coroutine_local_value<bool> sm_bReentering;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Execution interruption. May affect a single thread/coroutine or the whole program.
class ABACLADE_SYM execution_interruption : public exception {
public:
   //! Default constructor.
   execution_interruption(/*source?*/);

   /*! Copy constructor.

   @param x
      Source object.
   */
   execution_interruption(execution_interruption const & x);

   //! Destructor.
   virtual ~execution_interruption();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   execution_interruption & operator=(execution_interruption const & x);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Interruption in the execution of the whole application, typically requested by the user. Raised
simultaneously in every coroutine and thread. */
class ABACLADE_SYM app_execution_interruption : public execution_interruption {
public:
   //! Default constructor.
   app_execution_interruption();

   /*! Copy constructor.

   @param x
      Source object.
   */
   app_execution_interruption(app_execution_interruption const & x);

   //! Destructor.
   virtual ~app_execution_interruption();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   app_execution_interruption & operator=(app_execution_interruption const & x);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Thrown in coroutines and threads that are still running when abc::app:main() returns, to force
them to return as well. */
class ABACLADE_SYM app_exit_interruption : public execution_interruption {
public:
   //! Default constructor.
   app_exit_interruption();

   /*! Copy constructor.

   @param x
      Source object.
   */
   app_exit_interruption(app_exit_interruption const & x);

   //! Destructor.
   virtual ~app_exit_interruption();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   app_exit_interruption & operator=(app_exit_interruption const & x);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

/*! Execution interruption requested by the user, resulting in the termination of all coroutines and
threads in the process. */
class ABACLADE_SYM user_forced_interruption : public app_execution_interruption {
public:
   //! Default constructor.
   user_forced_interruption();

   /*! Copy constructor.

   @param x
      Source object.
   */
   user_forced_interruption(user_forced_interruption const & x);

   //! Destructor.
   virtual ~user_forced_interruption();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   user_forced_interruption & operator=(user_forced_interruption const & x);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Base for all error-related exceptions classes.
class ABACLADE_SYM generic_error : public exception {
public:
   //! Default constructor.
   generic_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   generic_error(generic_error const & x);

   //! Destructor.
   virtual ~generic_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   generic_error & operator=(generic_error const & x);

   /*! See abc::exception::init().

   @param err
      OS-defined error number associated to the exception.
   */
   void init(errint_t err = 0) {
      exception::init();
      m_err = err;
   }

   /*! Returns the OS-defined error number, if any.

   @return
      OS-defined error number.
   */
   errint_t os_error() const {
      return m_err;
   }

protected:
   //! See exception::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

protected:
   //! OS-specific error wrapped by this exception.
   errint_t m_err;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! A function/method received an argument that had an inappropriate value.
class ABACLADE_SYM argument_error : public generic_error {
public:
   //! Default constructor.
   argument_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   argument_error(argument_error const & x);

   //! Destructor.
   virtual ~argument_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   argument_error & operator=(argument_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Invalid value provided for a variable/argument.
class ABACLADE_SYM domain_error : public generic_error {
public:
   //! Default constructor.
   domain_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   domain_error(domain_error const & x);

   //! Destructor.
   virtual ~domain_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   domain_error & operator=(domain_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! A network-related error occurred.
class ABACLADE_SYM network_error : public generic_error {
public:
   //! Default constructor.
   network_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   network_error(network_error const & x);

   //! Destructor.
   virtual ~network_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   network_error & operator=(network_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! An operation failed to prevent a security hazard.
class ABACLADE_SYM security_error : public generic_error {
public:
   //! Default constructor.
   security_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   security_error(security_error const & x);

   //! Destructor.
   virtual ~security_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   security_error & operator=(security_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

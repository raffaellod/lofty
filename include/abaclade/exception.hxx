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

/*! @file
Facilities to create smart enumerations, as described in @ref enumeration-classes
*/

/*! @page exception-classes Exception classes
Abaclade’s exception class hierarchy. These classes provide diverse and semantically-rich types that
complement the hirearchy provided by the STL.

Classes in Abaclade’s exception class hierarchy inherit from different STL exception classes, to
facilitate interoperability between the two hierarchies.

The STL hierarchy does not use virtual inheritance. In earlier versions of Abaclade, this would mean
that, since some Abaclade exception classes have an as-is relationship with an STL exception class
that’s not std::exception, in some cases this inheritance scenario would occur:

   @code
   class abc::exception :          class abc::domain_error :
      public std::exception {         public abc::exception,
   };                                 public std::domain_error {
                                   };

   throw abc::exception();         throw abc::domain_error();
   @endcode
   @verbatim
   ┌────────────────┐              ┌────────────────────┐
   │ std::exception │              │┌──────────────────┐│
   └────────────────┘              ││ abc::exception   ││
                                   ││┌────────────────┐││
                                   │││ std::exception │││
                                   ││└────────────────┘││
                                   │└──────────────────┘│
                                   │ std::domain_error  │
                                   │┌──────────────────┐│
                                   ││ std::exception   ││
                                   │└──────────────────┘│
                                   └────────────────────┘
   @endverbatim

As visible on the right side of the diagram, an abc::domain_error would include two distinct copies
of std::exception, which leads to ambiguity: for example, abc::domain_error may be cast as both
abc::exception → std::exception or as std::domain_error → std::exception. While this does not
trigger any warnings in GCC, MSC16 warns that the resulting object (e.g. an abc::domain_error
instance) will not be caught by a std::exception catch block, arguably due to said casting ambiguity
– the MSC exception handling implementation might not know which of the two casts to favor.

Instead, in the current implementation of the exception class hierarchy, the Abaclade and the STL
hierarchies are kept completely separated; they are only combined when an exception is thrown, by
instantiating the class template abc::detail::exception_aggregator, specializations of which create
the leaf classes mentioned earlier.

The instantiation of a specialization of abc::detail::exception_aggregator is conveniently handled
by the ABC_THROW() statement; see this example, based on the previous one:

   @code
   class abc::exception {                      class abc::domain_error :
   public:                                        public abc::exception {
      typedef std::exception related_std;      public:
   };                                             typedef std::domain_error related_std;
                                               };

   ABC_THROW(abc::exception, ())               ABC_THROW(abc::domain_error, ())
   @endcode
   @verbatim
   ┌────────────────┐                          ┌───────────────────┐
   │ std::exception │                          │ std::domain_error │
   ├────────────────┤                          │┌─────────────────┐│
   │ abc::exception │                          ││ std::exception  ││
   └────────────────┘                          │└─────────────────┘│
                                               ├───────────────────┤
                                               │ abc::domain_error │
                                               │┌─────────────────┐│
                                               ││ abc::exception  ││
                                               │└─────────────────┘│
                                               └───────────────────┘
   @endverbatim

Note: multiple vtables (and therefore typeid and identifiers) can and will be generated for
abc::detail::exception_aggregator (with identical template arguments) across all binaries, because
no exported definition of it is available; this could be a problem if any code were to catch
instances of abc::detail::exception_aggregator, because exceptions thrown in one library wouldn’t be
caught by a catch block in another. However, this is not an issue because no code should be catching
abc::detail::exception_aggregator instance; clients will instead catch the appropriate Abaclade or
STL exception class, and these are indeed defined once for all binaries, and are therefore unique.

See also ABC_THROW() for more information.

See [IMG:8190 Exception class hierarchy] for a diagram of the entire Abaclade exception class
hierarchy, including the relations with the STL hierarchy.

Reference for Python’s exception class hierarchy: <http://docs.python.org/3.2/library/
exceptions.html>.
*/

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

namespace abc {

//! Source code location.
class source_location {
public:
   //! Default constructor.
   source_location() :
      m_pszFilePath(nullptr),
      m_iLine(0) {
   }

   /*! Constructor.

   @param pszFilePath
      Path to the source file.
   @param iLine
      Line number in pszFilePath.
   */
   source_location(char_t const * pszFilePath, unsigned iLine) :
      m_pszFilePath(pszFilePath),
      m_iLine(static_cast<std::uint16_t>(iLine)) {
   }

   /*! Returns the file path.

   @return
      File path.
   */
   char_t const * file_path() const {
      return m_pszFilePath;
   }

   /*! Returns the line number.

   @return
      Line number.
   */
   unsigned line_number() const {
      return m_iLine;
   }

protected:
   //! Path to the source file.
   char_t const * m_pszFilePath;
   //! Line number in m_pszFilePath.
   std::uint16_t m_iLine;
};

} //namespace abc

/*! Expands into the instantiation of a temporary abc::source_location object referencing the
location in which it’s used.

@return
   abc::source_location instance.
*/
#define ABC_SOURCE_LOCATION() \
   (::abc::source_location(ABC_SL(__FILE__), __LINE__))

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc { namespace detail {

/*! Combines a std::exception-derived class with an abc::exception-derived class, to form objects
that can be caught from code written for either framework. */
template <class TAbc, class TStd = typename TAbc::related_std>
class exception_aggregator : public TStd, public TAbc {
public:
   //! Default constructor.
   exception_aggregator() :
      TStd(),
      TAbc() {
   }

   //! Destructor.
   virtual ~exception_aggregator() ABC_STL_NOEXCEPT_TRUE() {
   }

   //! See std::exception::what().
   virtual const char * what() const ABC_STL_NOEXCEPT_TRUE() override {
      return TAbc::what();
   }
};

}} //namespace abc::detail

/*! Implementation of ABC_THROW(); can be used directly to customize the source of the exception.

@param srcloc
   Location at which the exception is being thrown.
@param pszFunction
   Function that is throwing the exception.
@param x
   Exception type to be thrown.
@param info
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by
   x::init().
*/
#define _ABC_THROW_FROM(srcloc, pszFunction, x, info) \
   do { \
      ::abc::detail::exception_aggregator<x> _x; \
      _x.init info; \
      _x._before_throw(srcloc, pszFunction); \
      throw _x; \
   } while (false)

//! Pretty-printed name of the current function.
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   /* Can’t use ABC_SL(__PRETTY_FUNCTION__) because __PRETTY_FUNCTION__ is expanded by the compiler,
   not the preprocessor; this causes ABC_SL(__PRETTY_FUNCTION__) to incorrectly expand to
   u8__PRETTY_FUNCTION__. However these compilers will encode __PRETTY_FUNCTION__ using UTF-8, which
   makes ABC_SL() unnecessary, so just avoid using it here. */
   #define _ABC_THIS_FUNC \
      __PRETTY_FUNCTION__
#elif ABC_HOST_CXX_MSC
   /* __FUNCSIG__ is expanded after preprocessing like __PRETTY_FUNCTION__, but for some reason this
   works just fine. */
   #define _ABC_THIS_FUNC \
      ABC_SL(__FUNCSIG__)
#else
   #define _ABC_THIS_FUNC \
      nullptr
#endif

/*! Instantiates a specialization of the class template abc::detail::exception_aggregator, fills it
up with context information and the remaining arguments, and then throws it.

This is the recommended way of throwing an exception within code using Abaclade. See the
documentation for abc::exception for more information on abc::detail::exception_aggregator and why
it exists. Combined with @ref stack-tracing, the use of ABC_THROW() augments the stack trace with
the exact line where the throw statement occurred.

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
   _ABC_THROW_FROM(ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC, x, info)

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Base for all of Abaclade’s exceptions classes. See @see exception-classes.
class ABACLADE_SYM exception {
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
      memory_access_error,
      memory_address_error
   );

   //! Related STL exception class.
   typedef _std::exception related_std;

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

   @param srcloc
      Location at which the exception is being thrown.
   @param pszFunction
      Function that is throwing the exception.
   */
   void _before_throw(source_location const & srcloc, char_t const * pszFunction);

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

   @return
      Exception type. May be nullptr to indicate that the caught exception is not an std::exception
      instance.
   */
   static common_type execution_interruption_to_common_type(_std::exception const * px = nullptr);

   /*! See std::exception::what(). Note that this is not virtual, because derived classes don’t need
   to override it; only abc::detail::exception_aggregator will define this as a virtual, to override
   std::exception::what() with this implementation.

   @return
      Name of the exception class.
   */
   char const * what() const;

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
   //! Source function name.
   char_t const * m_pszSourceFunction;
   //! Source location.
   source_location m_srcloc;
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
*/
#ifdef DEBUG
   #define ABC_ASSERT(expr, sMsg) \
      do { \
         if (!(expr)) { \
            abc::assertion_error::_assertion_failed( \
               ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC, ABC_SL(#expr), sMsg \
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
      source_location const & srcloc, str const & sFunction, str const & sExpr, str const & sMsg
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
#if 0
   // TODO: make abc::detail::exception_aggregator able to construct <stdexcept> classes.

   //! See abc::generic_error::related_std.
   typedef _std::invalid_argument related_std;
#endif

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
#if 0
   // TODO: make abc::detail::exception_aggregator able to construct <stdexcept> classes.

   //! See abc::generic_error::related_std.
   typedef _std::domain_error related_std;
#endif

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

//! Invalid iterator operation, such as moving an iterator to outside the container’s range.
class ABACLADE_SYM iterator_error : public generic_error {
public:
   //! Default constructor.
   iterator_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   iterator_error(iterator_error const & x);

   //! Destructor.
   virtual ~iterator_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   iterator_error & operator=(iterator_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Base for errors due to an invalid key or index being used on a mapping or sequence.
class ABACLADE_SYM lookup_error : public generic_error {
public:
   //! Default constructor.
   lookup_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   lookup_error(lookup_error const & x);

   //! Destructor.
   virtual ~lookup_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   lookup_error & operator=(lookup_error const & x);

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Sequence subscript out of range.
class ABACLADE_SYM index_error : public lookup_error {
public:
#if 0
   // TODO: make abc::detail::exception_aggregator able to construct <stdexcept> classes.

   //! See abc::lookup_error::related_std.
   typedef _std::out_of_range related_std;
#endif

public:
   //! Default constructor.
   index_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   index_error(index_error const & x);

   //! Destructor.
   virtual ~index_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   index_error & operator=(index_error const & x);

   /*! Returns the invalid index.

   @return
      Index that was not valid in the context in which it was used.
   */
   std::ptrdiff_t index() const {
      return m_iInvalid;
   }

   /*! See abc::lookup_error::init().

   @param iInvalid
      Index that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(std::ptrdiff_t iInvalid, errint_t err = 0);

   /*! See abc::lookup_error::init().

   @param iInvalid
      Index that caused the error.
   @param iMin
      Minimum allowed index value.
   @param iMax
      Maximum allowed index value.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(std::ptrdiff_t iInvalid, std::ptrdiff_t iMin, std::ptrdiff_t iMax, errint_t err = 0);

protected:
   //! See lookup_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Index that caused the error.
   std::ptrdiff_t m_iInvalid;
   //! Minimum allowed index value.
   std::ptrdiff_t m_iMin;
   //! Maximum allowed index value.
   std::ptrdiff_t m_iMax;
   //! true if m_iMin and m_iMax have been provided.
   bool m_bMinMaxProvided;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace abc {

//! Mapping (dictionary) key not found in the set of existing keys.
class ABACLADE_SYM key_error : public lookup_error {
public:
   //! Default constructor.
   key_error();

   /*! Copy constructor.

   @param x
      Source object.
   */
   key_error(key_error const & x);

   //! Destructor.
   virtual ~key_error();

   /*! Copy-assignment operator.

   @param x
      Source object.
   @return
      *this.
   */
   key_error & operator=(key_error const & x);

   //! See abc::lookup_error::init().
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

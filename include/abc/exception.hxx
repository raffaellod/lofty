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

#ifndef _ABC_CORE_HXX
   #error Please #include <abc/core.hxx> instead of this file
#endif

#include <exception>



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::source_location


/** Expands into the instantiation of a temporary abc::source_location object referencing the
location in which it’s used.

return
   abc::source_location instance.
*/
#define ABC_SOURCE_LOCATION() \
   (::abc::source_location(SL(__FILE__), __LINE__))


namespace abc {

/** Source code location.
*/
class source_location {
public:

   /** Constructor.

   pszFilePath
      Path to the source file.
   iLine
      Line number in pszFilePath.
   */
   source_location() :
      m_pszFilePath(nullptr),
      m_iLine(0) {
   }
   source_location(char_t const * pszFilePath, unsigned iLine) :
      m_pszFilePath(pszFilePath),
      m_iLine(uint16_t(iLine)) {
   }


   /** Returns the file path.

   return
      File path.
   */
   char_t const * file_path() const {
      return m_pszFilePath;
   }


   /** Returns the line number.

   return
      Line number.
   */
   unsigned line_number() const {
      return m_iLine;
   }


protected:

   /** Path to the source file. */
   char_t const * m_pszFilePath;
   /** Line number in m_pszFilePath. */
   uint16_t m_iLine;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception


namespace abc {

/** DOC:8190 Exception class hierarchy

ABC provides a diverse and semantically-rich exception class hierarchy that parallels and extends
that provided by the STL.

Due to the fact that both class hierarchies extend in both width and depth, and since the STL
hierarchy does not use virtual base classes, they have to be kept completely separated until the
most-derived classes, which is the only way the guarantee can be provided that no leaf class will
derive twice from std::exception, yielding to instances with two separate, ambiguous, sets of
std::exception members. See for example this fictional hierarchy, displaying an early tentative
ABC design having a single class hierarchy where each class would derive individually from a
std::exception-derived class:

   class abc::exception :                  abc::exception
      public std::exception {             ┌────────────────┐
      …                                   │ std::exception │
   };                                     └────────────────┘

   class abc::network_error :              abc::network_error
      public virtual abc::exception {     ┌──────────────────┐
      …                                   │ abc::exception   │
   };                                     │┌────────────────┐│
                                          ││ std::exception ││
                                          │└────────────────┘│
                                          └──────────────────┘

   class abc::io_error :                   abc::io_error
      public virtual abc::exception,      ┌────────────────────────┐
      public std::ios_base::failure {     │ abc::exception         │
      …                                   │┌──────────────────────┐│
   };                                     ││ std::exception       ││
                                          │└──────────────────────┘│
                                          ├────────────────────────┤
                                          │ std::ios_base::failure │
                                          │┌──────────────────────┐│
                                          ││ std::exception       ││
                                          │└──────────────────────┘│
                                          └────────────────────────┘

   class abc::network_io_error :           abc::network_io_error
      public virtual abc::network_error,  ┌────────────────────┬──────────────────────────┐
      public virtual abc::io_error {      │ abc::network_error │ abc::io_error            │
      …                                   │┌───────────────────┴─────────────────────────┐│
   };                                     ││ abc::exception                              ││
                                          ││┌───────────────────────────────────────────┐││
                                          │││ std::exception                            │││
                                          ││└───────────────────────────────────────────┘││
                                          │└───────────────────┬─────────────────────────┘│
                                          │                    │┌────────────────────────┐│
                                          │                    ││ std::ios_base::failure ││
                                          │                    ││┌──────────────────────┐││
                                          │                    │││ std::exception       │││
                                          │                    ││└──────────────────────┘││
                                          │                    │└────────────────────────┘│
                                          └────────────────────┴──────────────────────────┘

As visible in the last two class data representations, objects can include multiple distinct copies
of std::exception, which leads to ambiguity: for example, abc::io_error may be cast as both
abc::exception → std:exception or as std::ios_base::failure → std::exception. While this does not
trigger any warnings in GCC, MSC16 warns that the resulting object (e.g. an abc::io_error instance)
will not be caught by a std::exception catch block, arguably due to said casting ambiguity - the
MSVCRT might not know which of the two casts to favor.

In the current implementation of the exception class hierarchy instead, the ABC and the STL
hierarchies are kept completely separated; they are only combined when an exception is thrown, by
instantiating the class template abc::_exception_aggregator, specializations of which create the
leaf classes mentioned earlier; this is conveniently handled in the ABC_THROW() statement. See this
example based on the previous one:

   class abc::exception {                           ABC_THROW(abc::exception, ())
      typedef std::exception related_std;          ┌────────────────┐
      …                                            │ std::exception │
   };                                              ├────────────────┤
                                                   │ abc::exception │
                                                   └────────────────┘

   class abc::network_error :                       ABC_THROW(abc::network_error, ())
      public virtual abc::exception {              ┌────────────────────┐
      …                                            │ std::exception     │
   };                                              ├────────────────────┤
                                                   │ abc::network_error │
                                                   │┌──────────────────┐│
                                                   ││ abc::exception   ││
                                                   │└──────────────────┘│
                                                   └────────────────────┘

   class abc::io_error :                            ABC_THROW(abc::io_error, ())
      public virtual abc::exception {              ┌────────────────────────┐
      typedef std::ios_base::failure related_std;  │ std::ios_base::failure │
      …                                            │┌──────────────────────┐│
   };                                              ││ std::exception       ││
                                                   │└──────────────────────┘│
                                                   ├────────────────────────┤
                                                   │ abc::io_error          │
                                                   │┌──────────────────────┐│
                                                   ││ abc::exception       ││
                                                   │└──────────────────────┘│
                                                   └────────────────────────┘

   class abc::network_io_error :                    ABC_THROW(abc::network_io_error, ())
      public virtual abc::network_error,           ┌──────────────────────────────────────┐
      public virtual abc::io_error {               │ std::ios_base::failure               │
      typedef std::ios_base::failure related_std;  │┌────────────────────────────────────┐│
      …                                            ││ std::exception                     ││
   };                                              │└────────────────────────────────────┘│
                                                   ├──────────────────────────────────────┤
                                                   │ abc::network_io_error                │
                                                   │┌────────────────────┬───────────────┐│
                                                   ││ abc::network_error │ abc::io_error ││
                                                   ││┌───────────────────┴──────────────┐││
                                                   │││ abc::exception                   │││
                                                   ││└───────────────────┬──────────────┘││
                                                   │└────────────────────┴───────────────┘│
                                                   └──────────────────────────────────────┘

Note: multiple vtables (and therefore typeid and identifiers) can and will be generated for
abc::_exception_aggregator (with identical template arguments) across all binaries, because no
exported definition of it is available; this could be a problem if any code were to catch instances
of abc::_exception_aggregator, because exceptions thrown in one library wouldn’t be caught by a
catch block in another. However, this is not an issue because no code should be catching
abc::_exception_aggregator instance; clients will instead catch the appropriate ABC or STL exception
class, and these are indeed defined once for all binaries, and are therefore unique.

See [DOC:8191 Throwing exceptions] for more information on ABC_THROW().

Most of the exception class hierarchy is based on Python’s, which was chosen as model because of its
breadth and depth.

See related diagram [DIA:8190 Exception class hierarchy] for a diagram of the entire ABC exception
class hierarchy, including the relations with the STL hierarchy.

Reference for Python’s exception class hierarchy: <http://docs.python.org/3.2/library/
exceptions.html>.
*/

/** DOC:8191 Throwing exceptions

ABC_THROW() instantiates a specialization of the class template abc::_exception_aggregator, fills it
up with context information and the remaining arguments, and then throws it. This is the suggested
way of throwing an exception within code using ABC. See [DOC:8190 Exception class hierarchy] for
more information on abc::_exception_aggregator and why it exists.

Combined with [DOC:8503 Stack tracing], the use of ABC_THROW() augments the stack trace with the
exact line where the throw statement occurred.

Only instances of abc::exception (or a derived class) can be thrown using ABC_THROW(), because of
the additional members that the latter expects to be able to set in the former.

The class abc::exception also implements the actual stack trace printing for abc::_stack_trace,
since this file is included in virtually every file whereas trace.hxx is not.
*/

/** Pretty-printed name of the current function. */
#if ABC_HOST_GCC
   // GCC chokes on SL(__PRETTY_FUNCTION__), claiming it expands to u8__PRETTY_FUNCTION__. This is
   // inconsistent with the successful expansion of SL(__FILE__), but since it’s not really
   // necessary, we just avoid using SL() here.
   #define _ABC_THIS_FUNC \
      __PRETTY_FUNCTION__
#elif ABC_HOST_MSC
   #define _ABC_THIS_FUNC \
      SL(__FUNCTION__)
#else
   #define _ABC_THIS_FUNC \
      nullptr
#endif


/** Combines a std::exception-derived class with an abc::exception-derived class, to form objects
that can be caught from code written for either framework.
*/
template <class TAbc, class TStd = typename TAbc::related_std>
class _exception_aggregator :
   public TStd,
   public TAbc {
public:

   /** Constructor.
   */
   _exception_aggregator() :
      TStd(),
      TAbc() {
   }


   /** Destructor.
   */
   virtual ~_exception_aggregator() ABC_STL_NOEXCEPT_TRUE() {
   }


   /** See std::exception::what().
   */
   virtual const char * what() const ABC_STL_NOEXCEPT_TRUE() {
      return TAbc::what();
   }
};


/** Throws the specified object, after providing it with debug information.

x
   Exception instance to be thrown.
info
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by
   x::init().
*/
#define ABC_THROW(x, info) \
   do { \
      ::abc::_exception_aggregator<x> _x; \
      _x.init info; \
      _x._before_throw(ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC); \
      throw _x; \
   } while (false)


/** Verifies an expression at compile time; failure is reported as a compiler error. See C++11 § 7
“Declarations” point 4.

expr
   bool-convertible constant expression to be evaluated.
msg
   Diagnostic message to be output in case expr evaluates to false.
*/
#if !ABC_HOST_GCC && !ABC_HOST_MSC
   #define static_assert(expr, msg) \
      extern char _static_assert_failed[(expr) ? 1 : -1]
#endif

namespace io {

// Forward declaration.
class ostream;
// Methods here need to use io::ostream * instead of io::ostream & because at this point io::ostream
// has only been forward-declared above, but not defined yet (a pointer to a forward-declared type
// is legal, but a reference to it is not).

}


/** Base for all abc exceptions classes.
*/
class ABCAPI exception {
public:

   /** Related STL exception class. */
   typedef std::exception related_std;


   /** Constructor.

   x
      Source error.
   */
   exception();
   exception(exception const & x);


   /** Destructor.
   */
   virtual ~exception();


   /** Assignment operator. See std::exception::operator=().
   */
   exception & operator=(exception const & x);


   /** Stores context information to be displayed if the exception is not caught.

   srcloc
      Location at which the exception is being thrown.
   pszFunction
      Function that is throwing the exception.
   */
   void _before_throw(source_location const & srcloc, char_t const * pszFunction);


   /** Initializes the information associated to the exception.
   */
   void init() {
   }


   /** See std::exception::what(). Note that this is not virtual, because derived classes don’t need
   to override it; only abc::_exception_aggregator will define this as a virtual, to override
   std::exception::what() with this implementation.

   return
      Name of the exception class.
   */
   char const * what() const;


   /** Writes detailed information about an exception, as well as any scope/stack trace generated up
   to the point of the call to this function.

   pos
      Stream to write to. If omitted, the stack trace will be written to stderr.
   pstdx
      Caught exception.
   */
   static void write_with_scope_trace(
      io::ostream * pos = nullptr, std::exception const * pstdx = nullptr
   );


protected:

   /** Prints extended information for the exception.

   pos
      Pointer to a stream to write to.
   */
   virtual void _print_extended_info(io::ostream * pos) const;


public:

   /** Establishes, and restores upon destruction, special-case handlers to convert non-C++
   asynchronous error events (POSIX signals, Win32 Structured Exceptions) into C++ exceptions.

   For unsupported OSes, this class is empty. A little silly, but avoids conditional code in other
   files that shouldn’t care whether the target OS is supported in this respect or not.

   Note: this class uses global or thread-local variables (OS-dependent) for all its member
   variables, since their types cannot be specified without #including a lot of files into this one.
   */
   class ABCAPI async_handler_manager {
#if ABC_HOST_API_LINUX || ABC_HOST_API_WIN32
   public:

      /** Constructor.
      */
      async_handler_manager();

      /** Destructor.
      */
      ~async_handler_manager();
#endif
   };


protected:

   /** String to be returned by what(). Derived classes can overwrite this instead of overriding the
   entire std::exception::what() method. */
   char const * m_pszWhat;


private:

   /** Source function name. */
   char_t const * m_pszSourceFunction;
   /** Source location. */
   source_location m_srcloc;
   /** true if *this is an in-flight exception (it has been thrown) or is a copy of one. */
   bool m_bInFlight;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::assertion_error


namespace abc {

/** Verifies a condition at runtime, throwing a assertion_error exception if the assertion turns out
to be incorrect.

expr
   Expression to be validated.
*/
#ifdef DEBUG
   #define ABC_ASSERT(expr, sMsg) \
      do { \
         if (!(expr)) { \
            abc::assertion_error::_assertion_failed( \
               ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC, SL(#expr), sMsg \
            ); \
         } \
      } while (0)
#else
   #define ABC_ASSERT(expr) \
      static_cast<void>(0)
#endif

class istr;

/** An assertion failed.
*/
class ABCAPI assertion_error :
   public exception {
public:

   /** Throws an exception of type ab::assertion_error due to an expression failing validation.
   */
   static ABC_FUNC_NORETURN void _assertion_failed(
      source_location const & srcloc, istr const & sFunction, istr const & sExpr, istr const & sMsg
   );


protected:

   /** Set to true for the duration of the execution of _assertion_failed(). If another assertion
   fails due to code executed during the call to _assertion_failed(), the latter will just throw,
   without printing anything; otherwise we’ll most likely get stuck in an infinite recursion. */
   static /*tls*/ bool sm_bReentering;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::user_interrupt


namespace abc {

/** The user hit an interrupt key (usually Ctrl-C or Del).
*/
class ABCAPI user_interrupt :
   public exception {
public:
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error


namespace abc {

/** Integer type used by the OS to represent error numbers. */
#if ABC_HOST_API_POSIX
   typedef int errint_t;
#elif ABC_HOST_API_WIN32
   typedef DWORD errint_t;
#else
   #error HOST_API
#endif


#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32

/** Throws an exception matching a specified OS-defined error, or the last reported by the OS.

err
   OS-defined error number.
*/
ABCAPI ABC_FUNC_NORETURN void throw_os_error();
ABCAPI ABC_FUNC_NORETURN void throw_os_error(errint_t err);

#endif


/** Base for all error-related exceptions classes.
*/
class ABCAPI generic_error :
   public exception {
public:

   /** Constructor.

   x
      Source error.
   */
   generic_error();
   generic_error(generic_error const & x);


   /** Assignment operator. See abc::exception::operator=().
   */
   generic_error & operator=(generic_error const & x);


   /** See abc::exception::init().

   err
      OS-defined error number associated to the exception.
   */
   void init(errint_t err = 0) {
      exception::init();
      m_err = err;
   }


   /** Returns the OS-defined error number, if any.

   return
      OS-defined error number.
   */
   errint_t os_error() const {
      return m_err;
   }


protected:

   /** OS-specific error wrapped by this exception. */
   errint_t m_err;
};

} //namespace abc


// Relational operators.
#define ABC_RELOP_IMPL(op) \
   inline bool operator op(abc::generic_error const & ge1, abc::generic_error const & ge2) { \
      return ge1.os_error() op ge2.os_error(); \
   } \
   inline bool operator op(abc::generic_error const & ge, abc::errint_t err) { \
      return ge.os_error() op err; \
   } \
   inline bool operator op(abc::errint_t err, abc::generic_error const & ge) { \
      return err op ge.os_error(); \
   }
ABC_RELOP_IMPL(==)
ABC_RELOP_IMPL(!=)
#undef ABC_RELOP_IMPL


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::os_error_mapping


namespace abc {

/** Defines a member mapped_error to the default OS-specific error code associated to an exception
class.
*/
template <class TError>
struct os_error_mapping {

   /** Default error code the class errclass maps from. */
   static errint_t const mapped_error = 0;
};


/** Defines an OS-specific error code to be the default for an exception class.
*/
#define ABC_MAP_ERROR_CLASS_TO_ERRINT(errclass, err) \
   template <> \
   class os_error_mapping<errclass> { \
   public: \
   \
      static errint_t const mapped_error = err; \
   }

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error


namespace abc {

/** A function/method received an argument that had an inappropriate value.
*/
class ABCAPI argument_error :
   public virtual generic_error {
public:

   /** Constructor.

   TODO: add arguments name/value, to be passed by macro ABC_THROW_ARGUMENT_ERROR(argname).
   */
   argument_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error


namespace abc {

/** Base for arithmetic errors.
*/
class ABCAPI arithmetic_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   arithmetic_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::buffer_error


namespace abc {

/** A buffer operation could not be performed.
*/
class ABCAPI buffer_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   buffer_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error


namespace abc {

/** The divisor of a division or modulo operation was zero.
*/
class ABCAPI division_by_zero_error :
   public virtual arithmetic_error {
public:

   /** Constructor.
   */
   division_by_zero_error();


   /** See abc::arithmetic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error


namespace abc {

class ABCAPI domain_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   domain_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::environment_error


namespace abc {

/** Base for errors that occur in the outer system.
*/
class ABCAPI environment_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   environment_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_not_found_error


namespace abc {

/** A file could not be found.
*/
class ABCAPI file_not_found_error :
   public virtual environment_error {
public:

   /** Constructor.
   */
   file_not_found_error();


   /** See abc::environment_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error


namespace abc {

/** A floating point operation failed.
*/
class ABCAPI floating_point_error :
   public virtual arithmetic_error {
public:

   /** Constructor.
   */
   floating_point_error();


   /** See abc::arithmetic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error


namespace abc {

/** Base for errors due to an invalid key or index being used on a mapping or sequence.
*/
class ABCAPI lookup_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   lookup_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error


namespace abc {

/** Sequence subscript out of range.
*/
class ABCAPI index_error :
   public virtual lookup_error {
public:

   /** Constructor.

   x
      Source error.
   */
   index_error();
   index_error(index_error const & x);


   /** Assignment operator. See abc::lookup_error::operator=().
   */
   index_error & operator=(index_error const & x);


   /** Returns the invalid index.

   return
      Index that was not valid in the context in which it was used.
   */
   intptr_t index() const {
      return m_iInvalid;
   }


   /** See abc::lookup_error::init().

   iInvalid
      Index that caused the error.
   err
      OS-defined error number associated to the exception.
   */
   void init(intptr_t iInvalid, errint_t err = 0);


protected:

   /** See exception::_print_extended_info().
   */
   virtual void _print_extended_info(io::ostream * pos) const;


private:

   /** Index that caused the error. */
   intptr_t m_iInvalid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::key_error


namespace abc {

/** Mapping (dictionary) key not found in the set of existing keys.
*/
class ABCAPI key_error :
   public virtual lookup_error {
public:

   /** Constructor.
   */
   key_error();


   /** See abc::lookup_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::invalid_path_error


namespace abc {

/** The specified file path is not a valid path.
*/
class ABCAPI invalid_path_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   invalid_path_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io_error


namespace abc {

/** An I/O operation failed for an I/O-related reason.
*/
class ABCAPI io_error :
   public virtual environment_error {
public:

   /** Constructor.
   */
   io_error();


   /** See abc::environment_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error


namespace abc {

/** An attempt was made to access an invalid memory location.
*/
class ABCAPI memory_address_error :
   public virtual generic_error {
public:

   /** Constructor.

   x
      Source error.
   */
   memory_address_error();
   memory_address_error(memory_address_error const & x);


   /** Assignment operator. See abc::generic_error::operator=().
   */
   memory_address_error & operator=(memory_address_error const & x);


   /** Returns the faulty address.

   return
      Value of the pointer that was dereferenced.
   */
   void const * address() const {
      return m_pInvalid;
   }


   /** See abc::generic_error::init().

   pInvalid
      Pointer that could not be dereferenced.
   err
      OS-defined error number associated to the error.
   */
   void init(errint_t err = 0) {
      init(smc_achUnknownAddress, err);
   }
   void init(void const * pInvalid, errint_t err = 0);


protected:

   /** See exception::_print_extended_info().
   */
   virtual void _print_extended_info(io::ostream * pos) const;


private:

   /** Address that could not be dereferenced. */
   void const * m_pInvalid;
   /** String used as special value for when the address is not available. */
   static char_t const smc_achUnknownAddress[];
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error


namespace abc {

/** An invalid memory access (e.g. misaligned pointer) was detected.
*/
class ABCAPI memory_access_error :
   public virtual memory_address_error {
public:

   /** Constructor.
   */
   memory_access_error();


   /** See abc::memory_address_error::init().
   */
   void init(void const * pInvalid, errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error


namespace abc {

/** A memory allocation request could not be satisfied.
*/
class ABCAPI memory_allocation_error :
   public virtual generic_error {
public:

   /** See abc::generic_error::related_std. */
   typedef std::bad_alloc related_std;


   /** Constructor.
   */
   memory_allocation_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_error


namespace abc {

/** A network-related error occurred.
*/
class ABCAPI network_error :
   public virtual environment_error {
public:

   /** Constructor.
   */
   network_error();


   /** See abc::environment_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_io_error


namespace abc {

/** An I/O operation failed for a network-related reason.
*/
class ABCAPI network_io_error :
   public virtual io_error,
   public virtual network_error {
public:

   /** Constructor.
   */
   network_io_error();


   /** See abc::io_error::init() and abc::network_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::not_implemented_error


namespace abc {

/** Method not implemented for this class. Usually thrown when a class is not able to provide a full
implementation of an interface; in practice, this should be avoided.
*/
class ABCAPI not_implemented_error :
   public virtual generic_error {
public:

   /** Constructor.
   */
   not_implemented_error();


   /** See abc::generic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error


namespace abc {

/** An attempt was made to access the memory location 0 (nullptr).
*/
class ABCAPI null_pointer_error :
   public virtual memory_address_error {
public:

   /** Constructor.
   */
   null_pointer_error();


   /** See abc::memory_address_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error


namespace abc {

/** Result of an arithmetic operation too large to be represented. Because of the lack of
standardization of floating point exception handling in C, most floating point operations are also
not checked.
*/
class ABCAPI overflow_error :
   public virtual arithmetic_error {
public:

   /** Constructor.
   */
   overflow_error();


   /** See abc::arithmetic_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::security_error


namespace abc {

/** An operation failed to prevent a security hazard.
*/
class ABCAPI security_error :
   public virtual environment_error {
public:

   /** Constructor.
   */
   security_error();


   /** See abc::environment_error::init().
   */
   void init(errint_t err = 0);
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


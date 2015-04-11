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


/*! DOC:8190 Exception class hierarchy

Abaclade provides a diverse and semantically-rich exception class hierarchy that parallels and
extends that provided by the STL.

Due to the fact that both class hierarchies extend in both width and depth, and since the STL
hierarchy does not use virtual base classes, they have to be kept completely separated until the
most-derived classes, which is the only way the guarantee can be provided that no leaf class will
derive twice from std::exception, yielding to instances with two separate, ambiguous, sets of
std::exception members. See for example this fictional hierarchy, displaying an early tentative
Abaclade design having a single class hierarchy where each class would derive individually from a
std::exception-derived class:

   class abc::exception : public std::exception {};

       abc::exception
      ┌────────────────┐
      │ std::exception │
      └────────────────┘

   class abc::network_error : public virtual abc::exception {};

       abc::network_error
      ┌──────────────────┐
      │ abc::exception   │
      │┌────────────────┐│
      ││ std::exception ││
      │└────────────────┘│
      └──────────────────┘

   class abc::io_error : public virtual abc::exception, public std::ios_base::failure {};

       abc::io_error
      ┌────────────────────────┐
      │ abc::exception         │
      │┌──────────────────────┐│
      ││ std::exception       ││
      │└──────────────────────┘│
      ├────────────────────────┤
      │ std::ios_base::failure │
      │┌──────────────────────┐│
      ││ std::exception       ││
      │└──────────────────────┘│
      └────────────────────────┘

   class abc::network_io_error : public virtual abc::network_error, public virtual abc::io_error {};

       abc::network_io_error
      ┌────────────────────┬──────────────────────────┐
      │ abc::network_error │ abc::io_error            │
      │┌───────────────────┴─────────────────────────┐│
      ││ abc::exception                              ││
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
will not be caught by a std::exception catch block, arguably due to said casting ambiguity – the
MSVCRT might not know which of the two casts to favor.

In the current implementation of the exception class hierarchy instead, the Abaclade and the STL
hierarchies are kept completely separated; they are only combined when an exception is thrown, by
instantiating the class template abc::detail::exception_aggregator, specializations of which create
the leaf classes mentioned earlier; this is conveniently handled in the ABC_THROW() statement. See
this example based on the previous one:

   class abc::exception {
      typedef std::exception related_std;
   };

       ABC_THROW(abc::exception, ())
      ┌────────────────┐
      │ std::exception │
      ├────────────────┤
      │ abc::exception │
      └────────────────┘

   class abc::network_error : public virtual abc::exception {};

       ABC_THROW(abc::network_error, ())
      ┌────────────────────┐
      │ std::exception     │
      ├────────────────────┤
      │ abc::network_error │
      │┌──────────────────┐│
      ││ abc::exception   ││
      │└──────────────────┘│
      └────────────────────┘

   class abc::io_error : public virtual abc::exception {
      typedef std::ios_base::failure related_std;
   };

       ABC_THROW(abc::io_error, ())
      ┌────────────────────────┐
      │ std::ios_base::failure │
      │┌──────────────────────┐│
      ││ std::exception       ││
      │└──────────────────────┘│
      ├────────────────────────┤
      │ abc::io_error          │
      │┌──────────────────────┐│
      ││ abc::exception       ││
      │└──────────────────────┘│
      └────────────────────────┘

   class abc::network_io_error : public virtual abc::network_error, public virtual abc::io_error {
      typedef std::ios_base::failure related_std;
   };

       ABC_THROW(abc::network_io_error, ())
      ┌──────────────────────────────────────┐
      │ std::ios_base::failure               │
      │┌────────────────────────────────────┐│
      ││ std::exception                     ││
      │└────────────────────────────────────┘│
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
abc::detail::exception_aggregator (with identical template arguments) across all binaries, because
no exported definition of it is available; this could be a problem if any code were to catch
instances of abc::detail::exception_aggregator, because exceptions thrown in one library wouldn’t be
caught by a catch block in another. However, this is not an issue because no code should be catching
abc::detail::exception_aggregator instance; clients will instead catch the appropriate Abaclade or
STL exception class, and these are indeed defined once for all binaries, and are therefore unique.

See also ABC_THROW() for more information.

Most of the exception class hierarchy is based on Python’s, which was chosen as model because of its
breadth and depth.

See [IMG:8190 Exception class hierarchy] for a diagram of the entire Abaclade exception class
hierarchy, including the relations with the STL hierarchy.

Reference for Python’s exception class hierarchy: <http://docs.python.org/3.2/library/
exceptions.html>.
*/

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

//! Integer type used by the OS to represent error numbers.
#if ABC_HOST_API_POSIX
   typedef int errint_t;
#elif ABC_HOST_API_WIN32
   typedef DWORD errint_t;
#else
   #error "TODO: HOST_API"
#endif

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals – ABC_THROW()

/*! Instantiates a specialization of the class template abc::detail::exception_aggregator, fills it
up with context information and the remaining arguments, and then throws it.

This is the suggested way of throwing an exception within code using Abaclade. See [DOC:8190
Exception class hierarchy] for more information on abc::detail::exception_aggregator and why it
exists. Combined with [DOC:8503 Stack tracing], the use of ABC_THROW() augments the stack trace with
the exact line where the throw statement occurred.

Only instances of abc::exception (or a derived class) can be thrown using ABC_THROW(), because of
the additional members that the latter expects to be able to set in the former.

The class abc::exception implements the actual stack trace printing for abc::detail::scope_trace
because it’s the only class involved that’s not in a detail namespace.

@param x
   Exception instance to be thrown.
@param info
   Parentheses-enclosed list of data that will be associated to the exception, as accepted by
   x::init().
*/
#define ABC_THROW(x, info) \
   do { \
      ::abc::detail::exception_aggregator<x> _x; \
      _x.init info; \
      _x._before_throw(ABC_SOURCE_LOCATION(), _ABC_THIS_FUNC); \
      throw _x; \
   } while (false)

//! Pretty-printed name of the current function.
#if ABC_HOST_CXX_CLANG || ABC_HOST_CXX_GCC
   /* With GCC we cannot use ABC_SL(__PRETTY_FUNCTION__) because __PRETTY_FUNCTION__ is expanded by
   the compiler, not the preprocessor, which makes sense as the preprocessor doesn’t know what scope
   even means; this causes ABC_SL(__PRETTY_FUNCTION__) to expand to u8__PRETTY_FUNCTION__. However,
   since GCC will encode __PRETTY_FUNCTION__ using UTF-8, it’s not really necessary, so we just
   avoid using ABC_SL() here. */
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

namespace abc {
namespace detail {

/*! Combines a std::exception-derived class with an abc::exception-derived class, to form objects
that can be caught from code written for either framework. */
template <class TAbc, class TStd = typename TAbc::related_std>
class exception_aggregator : public TStd, public TAbc {
public:
   //! Constructor.
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

} //namespace detail
} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::source_location

/*! Expands into the instantiation of a temporary abc::source_location object referencing the
location in which it’s used.

@return
   abc::source_location instance.
*/
#define ABC_SOURCE_LOCATION() \
   (::abc::source_location(ABC_SL(__FILE__), __LINE__))

namespace abc {

//! Source code location.
class source_location {
public:
   /*! Constructor.

   @param pszFilePath
      Path to the source file.
   @param iLine
      Line number in pszFilePath.
   */
   source_location() :
      m_pszFilePath(nullptr),
      m_iLine(0) {
   }
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

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception

namespace abc {

//! Base for all abc exceptions classes.
class ABACLADE_SYM exception {
public:
   /*! Establishes, and restores upon destruction, special-case handlers to convert non-C++
   synchronous error events (POSIX signals, Win32 Structured Exceptions) into C++ exceptions.

   Note: this class uses global or thread-local variables for all its member variables, since their
   types cannot be specified without #including a lot of files into this one. */
   class ABACLADE_SYM fault_converter {
   public:
      //! Constructor.
      fault_converter();

      //! Destructor.
      ~fault_converter();
   };

   //! Possible exception types injectable by inject_in_context().
   ABC_ENUM_AUTO_VALUES(injectable,
      arithmetic_error,
      division_by_zero_error,
      floating_point_error,
      memory_access_error,
      memory_address_error,
      null_pointer_error,
      overflow_error
   );

   //! Related STL exception class.
   typedef std::exception related_std;

public:
   /*! Constructor.

   @param x
      Source error.
   */
   exception();
   exception(exception const & x);

   //! Destructor.
   virtual ~exception();

   //! Assignment operator. See std::exception::operator=().
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

   /*! Injects the requested type of exception in the specified context.
   @param inj
      Type of exception to inject.
   @param iArg0
      First argument to the exception constructor, if applicable.
   @param iArg1
      Second argument to the exception constructor, if applicable.
   */
   static void inject_in_context(
      exception::injectable inj, std::intptr_t iArg0, std::intptr_t iArg1, void * pctx
   );

   /*! Throws an exception of the specified type.

   @param inj
      Type of exception to be throw.
   @param iArg0
      Exception type-specific argument 0.
   @param iArg1
      Exception type-specific argument 1.
   */
   static void throw_injected_exception(
      exception::injectable::enum_type inj, std::intptr_t iArg0, std::intptr_t iArg1
   );

#if ABC_HOST_API_POSIX || ABC_HOST_API_WIN32
   /*! Throws an exception matching a specified OS-defined error, or the last reported by the OS.

   @param err
      OS-defined error number.
   */
   static ABC_FUNC_NORETURN void throw_os_error();
   static ABC_FUNC_NORETURN void throw_os_error(errint_t err);
#endif

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
      io::text::writer * ptwOut = nullptr, std::exception const * pstdx = nullptr
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
// abc::assertion_error

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
      source_location const & srcloc, istr const & sFunction, istr const & sExpr, istr const & sMsg
   );

protected:
   /*! Set to true for the duration of the execution of _assertion_failed(). If another assertion
   fails due to code executed during the call to _assertion_failed(), the latter will just throw,
   without printing anything; otherwise we’ll most likely get stuck in an infinite recursion. */
   static coroutine_local_value<bool> sm_bReentering;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::user_interrupt

namespace abc {

//! The user hit an interrupt key (usually Ctrl-C or Del).
class ABACLADE_SYM user_interrupt : public exception {
public:
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error

namespace abc {

//! Base for all error-related exceptions classes.
class ABACLADE_SYM generic_error : public exception {
public:
   /*! Constructor.

   @param x
      Source error.
   */
   generic_error();
   generic_error(generic_error const & x);

   //! Assignment operator. See abc::exception::operator=().
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
// abc::os_error_mapping

namespace abc {

/*! Defines a member mapped_error to the default OS-specific error code associated to an exception
class. */
template <class TError>
struct os_error_mapping {
   //! Default error code the class errclass maps from.
   static errint_t const mapped_error = 0;
};

/*! Defines an OS-specific error code to be the default for an exception class.

@param errclass
   generic_error-derived class.
@param err
   OS-specific error code.
*/
#define ABC_MAP_ERROR_CLASS_TO_ERRINT(errclass, err) \
   template <> \
   class os_error_mapping<errclass> { \
   public: \
      static errint_t const mapped_error = err; \
   }

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error

namespace abc {

//! A function/method received an argument that had an inappropriate value.
class ABACLADE_SYM argument_error : public virtual generic_error {
public:
   /*! Constructor.

   TODO: add arguments name/value, to be passed by macro ABC_THROW_ARGUMENT_ERROR(argname).
   */
   argument_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error

namespace abc {

//! Base for arithmetic errors.
class ABACLADE_SYM arithmetic_error : public virtual generic_error {
public:
   //! Constructor.
   arithmetic_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::buffer_error

namespace abc {

//! A buffer operation could not be performed.
class ABACLADE_SYM buffer_error : public virtual generic_error {
public:
   //! Constructor.
   buffer_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error

namespace abc {

//! The divisor of a division or modulo operation was zero.
class ABACLADE_SYM division_by_zero_error : public virtual arithmetic_error {
public:
   //! Constructor.
   division_by_zero_error();

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error

namespace abc {

//! Invalid value provided for a variable/argument.
class ABACLADE_SYM domain_error : public virtual generic_error {
public:
   //! Constructor.
   domain_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::environment_error

namespace abc {

//! Base for errors that occur in the outer system.
class ABACLADE_SYM environment_error : public virtual generic_error {
public:
   //! Constructor.
   environment_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error

namespace abc {

//! A floating point operation failed.
class ABACLADE_SYM floating_point_error : public virtual arithmetic_error {
public:
   //! Constructor.
   floating_point_error();

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::iterator_error

namespace abc {

//! Invalid iterator operation, such as moving an iterator to outside the container’s range.
class ABACLADE_SYM iterator_error : public virtual generic_error {
public:
   //! Constructor.
   iterator_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error

namespace abc {

//! Base for errors due to an invalid key or index being used on a mapping or sequence.
class ABACLADE_SYM lookup_error : public virtual generic_error {
public:
   //! Constructor.
   lookup_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error

namespace abc {

//! Sequence subscript out of range.
class ABACLADE_SYM index_error : public virtual lookup_error {
public:
   /*! Constructor.

   @param x
      Source error.
   */
   index_error();
   index_error(index_error const & x);

   //! Assignment operator. See abc::lookup_error::operator=().
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

protected:
   //! See lookup_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Index that caused the error.
   std::ptrdiff_t m_iInvalid;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::key_error

namespace abc {

//! Mapping (dictionary) key not found in the set of existing keys.
class ABACLADE_SYM key_error : public virtual lookup_error {
public:
   //! Constructor.
   key_error();

   //! See abc::lookup_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::invalid_path_error

namespace abc {

//! The specified path is not valid.
class ABACLADE_SYM invalid_path_error : public virtual generic_error {
public:
   //! Constructor.
   invalid_path_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io_error

namespace abc {

//! An I/O operation failed for an I/O-related reason.
class ABACLADE_SYM io_error : public virtual environment_error {
public:
   //! Constructor.
   io_error();

   //! See abc::environment_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error

namespace abc {

//! An attempt was made to access an invalid memory location.
class ABACLADE_SYM memory_address_error : public virtual generic_error {
public:
   /*! Constructor.

   @param x
      Source error.
   */
   memory_address_error();
   memory_address_error(memory_address_error const & x);

   //! Assignment operator. See abc::generic_error::operator=().
   memory_address_error & operator=(memory_address_error const & x);

   /*! Returns the faulty address.

   @return
      Value of the pointer that was dereferenced.
   */
   void const * address() const {
      return m_pInvalid;
   }

   /*! See abc::generic_error::init().

   @param pInvalid
      Pointer that could not be dereferenced.
   @param err
      OS-defined error number associated to the error.
   */
   void init(errint_t err = 0) {
      init(smc_achUnknownAddress, err);
   }
   void init(void const * pInvalid, errint_t err = 0);

protected:
   //! See generic_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Address that could not be dereferenced.
   void const * m_pInvalid;
   //! String used as special value for when the address is not available.
   static char_t const smc_achUnknownAddress[];
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error

namespace abc {

//! An invalid memory access (e.g. misaligned pointer) was detected.
class ABACLADE_SYM memory_access_error : public virtual memory_address_error {
public:
   //! Constructor.
   memory_access_error();

   //! See abc::memory_address_error::init().
   void init(void const * pInvalid, errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error

namespace abc {

//! A memory allocation request could not be satisfied.
class ABACLADE_SYM memory_allocation_error : public virtual generic_error {
public:
   //! See abc::generic_error::related_std.
   typedef std::bad_alloc related_std;

   //! Constructor.
   memory_allocation_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_error

namespace abc {

//! A network-related error occurred.
class ABACLADE_SYM network_error : public virtual environment_error {
public:
   //! Constructor.
   network_error();

   //! See abc::environment_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::network_io_error

namespace abc {

//! An I/O operation failed for a network-related reason.
class ABACLADE_SYM network_io_error : public virtual io_error, public virtual network_error {
public:
   //! Constructor.
   network_io_error();

   //! See abc::io_error::init() and abc::network_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::not_implemented_error

namespace abc {

/*! Method not implemented for this class. Usually thrown when a class is not able to provide a full
implementation of an interface; in practice, this should be avoided. */
class ABACLADE_SYM not_implemented_error : public virtual generic_error {
public:
   //! Constructor.
   not_implemented_error();

   //! See abc::generic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error

namespace abc {

//! An attempt was made to access the memory location 0 (nullptr).
class ABACLADE_SYM null_pointer_error : public virtual memory_address_error {
public:
   //! Constructor.
   null_pointer_error();

   //! See abc::memory_address_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error

namespace abc {

/*! Result of an arithmetic operation too large to be represented. Because of the lack of
standardization of floating point exception handling in C, most floating point operations are also
not checked. */
class ABACLADE_SYM overflow_error : public virtual arithmetic_error {
public:
   //! Constructor.
   overflow_error();

   //! See abc::arithmetic_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::pointer_iterator_error

namespace abc {

//! Invalid operation on a pointer-like iterator.
class ABACLADE_SYM pointer_iterator_error : public virtual iterator_error {
public:
   /*! Constructor.

   @param x
      Source error.
   */
   pointer_iterator_error();
   pointer_iterator_error(pointer_iterator_error const & x);

   //! Assignment operator. See abc::iterator_error::operator=().
   pointer_iterator_error & operator=(pointer_iterator_error const & x);

   /*! Returns the container’s begin iterator’s pointer.

   @return
      Value of container.cbegin().base().
   */
   void const * container_begin_pointer() const {
      return m_pContBegin;
   }

   /*! Returns the container’s end iterator’s pointer.

   @return
      Value of container.cend().base().
   */
   void const * container_end_pointer() const {
      return m_pContBegin;
   }

   /*! Returns the invalid iterator pointer value.

   @return
      Pointer that was not valid in the context in which it was used.
   */
   void const * iterator_pointer() const {
      return m_pInvalid;
   }

   /*! See abc::iterator_error::init().

   @param pContBegin
      Value returned by the container’s cbegin().base().
   @param pContEnd
      Value returned by the container’s cend().base().
   @param pInvalid
      Pointer member of the iterator that caused the error.
   @param err
      OS-defined error number associated to the exception.
   */
   void init(
      void const * pContBegin, void const * pContEnd, void const * pInvalid, errint_t err = 0
   );

protected:
   //! See iterator_error::write_extended_info().
   virtual void write_extended_info(io::text::writer * ptwOut) const override;

private:
   //! Value returned by the container’s cbegin().base().
   void const * m_pContBegin;
   //! Value returned by the container’s cend().base().
   void const * m_pContEnd;
   //! Pointer value of the iterator that caused the error.
   void const * m_pInvalid;
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::security_error

namespace abc {

//! An operation failed to prevent a security hazard.
class ABACLADE_SYM security_error : public virtual environment_error {
public:
   //! Constructor.
   security_error();

   //! See abc::environment_error::init().
   void init(errint_t err = 0);
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

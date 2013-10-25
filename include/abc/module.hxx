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

#ifndef ABC_MODULE_HXX
#define ABC_MODULE_HXX

#include <abc/core.hxx>
#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/atomic.hxx>
#include <abc/vector.hxx>
#include <abc/file_path.hxx>
#include <stdlib.h> // EXIT_SUCCESS EXIT_FAILURE



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals

namespace abc {

/** Thread ID type. */
#if ABC_HOST_API_POSIX
	typedef pthread_t tid_t;
#elif ABC_HOST_API_WIN32
	typedef DWORD tid_t;
#else
	#error TODO-PORT: HOST_API
#endif

/** Process ID type. */
#if ABC_HOST_API_POSIX
	// pid_t is already defined.
#elif ABC_HOST_API_WIN32
	typedef DWORD pid_t;
#else
	#error TODO-PORT: HOST_API
#endif

/** Native OS dynamic library/module handle. */
#if ABC_HOST_API_POSIX
	typedef void * hdynmod_t;
#elif ABC_HOST_API_WIN32
	typedef HINSTANCE hdynmod_t;
#else
	#error TODO-PORT: HOST_API
#endif

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread


namespace abc {

/** Base class for threads.
*/
template <typename T>
class thread :
	public T {
private:

	/** Thread ID (TID). */
	tid_t m_tid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::process


namespace abc {

/** Base class for processes.
*/
template <typename T>
class process :
	public T {
private:

	/** Process ID (PID). */
	pid_t m_pid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dynamic_module


#if ABC_HOST_API_WIN32

namespace abc {

/** Dynamically loadable module.
*/
class ABCAPI dynamic_module {

	ABC_CLASS_PREVENT_COPYING(dynamic_module)

public:

	/** Constructor.

	TODO: comment signature.
	*/
	dynamic_module(dynamic_module && dm);
	dynamic_module(file_path const & fp, bool bInit);
	dynamic_module(hdynmod_t hdynmod) :
		m_hdynmod(hdynmod),
		m_bOwn(false) {
	}


	/** Destructor.
	*/
	~dynamic_module() {
		if (m_bOwn) {
			::FreeLibrary(m_hdynmod);
		}
	}


	/** Assignment operator.

	TODO: comment signature.
	*/
	dynamic_module & operator=(dynamic_module && dm);


	/** Returns the file name of the module.

	return
		Full path to the module.
	*/
	file_path file_name() const;


protected:

	/** Handle to the module. */
	hdynmod_t m_hdynmod;


private:

	/** If false, the handle was provided by the caller of the constructor, and it will not be
	released. */
	bool m_bOwn;
};

} //namespace abc

#endif //if ABC_HOST_API_WIN32


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::resource_module


namespace abc {

/** Resource dynamically loadable module.
*/
class ABCAPI resource_module
#if ABC_HOST_API_WIN32
	: public dynamic_module
#endif
{

	ABC_CLASS_PREVENT_COPYING(resource_module)

public:

	/** Constructor.

	TODO: comment signature.
	*/
	resource_module(file_path const & fp);
	resource_module(resource_module && rm);


	/** Destructor.
	*/
	~resource_module();


	/** Assignment operator.

	TODO: comment signature.
	*/
	resource_module & operator=(resource_module && rm) {
#if ABC_HOST_API_POSIX
		UNUSED_ARG(rm);
#elif ABC_HOST_API_WIN32
		dynamic_module::operator=(std::move(rm));
#else
	#error TODO-PORT: HOST_API
#endif
		return *this;
	}


	/** Loads a string from the module’s resources.

	TODO: comment signature.
	*/
	size_t load_string(short id, char_t * psz, size_t cchMax) const;


protected:

	/** Constructor. This overload is meant to be used by module_impl_base, so that it can supply its
	own HINSTANCE (Win), which must not be released upon destruction of this object.

	TODO: comment signature.
	*/
#if ABC_HOST_API_POSIX
	resource_module() {
	}
#elif ABC_HOST_API_WIN32
	resource_module(hdynmod_t hdynmod) :
		dynamic_module(hdynmod) {
	}
#else
	#error TODO-PORT: HOST_API
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::code_module


namespace abc {

/** Code dynamically loadable module.
*/
class ABCAPI code_module
#if ABC_HOST_API_WIN32
	: public dynamic_module
#endif
{

	ABC_CLASS_PREVENT_COPYING(code_module)

public:

	/** Constructor.

	TODO: comment signature.
	*/
	code_module(file_path const & fp);
	code_module(code_module && cm);


	/** Destructor.
	*/
	~code_module();


	/** Assignment operator.

	TODO: comment signature.
	*/
	code_module & operator=(code_module && cm) {
#if ABC_HOST_API_POSIX
		m_hdynmod = cm.m_hdynmod;
		cm.m_hdynmod = NULL;
#elif ABC_HOST_API_WIN32
		dynamic_module::operator=(std::move(cm));
#else
	#error TODO-PORT: HOST_API
#endif
		return *this;
	}


	/** Returns a pointer to the specified symbol in the module.

	sSymbol
		Symbol name.
	[ppfn]
		Pointer to a variable that will receive, upon return, the address of the symbol. Specifying
		this arguments will automatically select the right template type, so the resulting method
		invocation will be much more readable.
	return
		Address of the symbol; same as *ppfn, if ppfn is provided.
	*/
	template <typename F>
	F get_symbol(istr const & sSymbol, F * ppfn = NULL) {
		F pfn(reinterpret_cast<F>(_get_symbol(sSymbol)));
		if (ppfn) {
			*ppfn = pfn;
		}
		return pfn;
	}


protected:

	/** Constructor. This overload is meant to be used by module_impl_base, so that it can supply its
	own HINSTANCE (Win) or nothing (POSIX.1-2001), which must not be released upon destruction of
	this object.

	TODO: comment signature.
	*/
#if ABC_HOST_API_POSIX
	code_module() :
		m_hdynmod(NULL) {
	}
#elif ABC_HOST_API_WIN32
	code_module(hdynmod_t hdynmod) :
		dynamic_module(hdynmod) {
	}
#else
	#error TODO-PORT: HOST_API
#endif


private:

	/** Returns a void pointer to the specified symbol in the module.

	sSymbol
		Symbol name.
	*/
	void * _get_symbol(istr const & sSymbol);


#if ABC_HOST_API_POSIX
private:

	/** Handle to the module. */
	hdynmod_t m_hdynmod;
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::module_impl_base


/** DOC:1063 Application modules

Programs using ABC declare their main() function by deriving a class from abc::app_module_impl,
overriding its main() method, and declaring that the derived class contains the program’s entry
point using ABC_MAIN_APP_MODULE().

The ABC_MAIN_APP_MODULE() macro defines the actual entry point of the program, using whatever
protocol is supported by the host (e.g. int main(…) on POSIX, BOOL WinMain(…) on Windows). This is a
very thin wrapper around a static method of abc::app_module_impl which takes care of setting up the
outermost try/catch block to intercept uncaught exceptions (see [DOC:8503 Stack tracing]), as well
as instantiating the application-defined abc::app_module_impl-derived class, invoking its main()
method and returning.
*/

namespace abc {

/** Base class for implementing a dynamically loadable module.
*/
class ABCAPI module_impl_base :
	public code_module,
	public resource_module {

	ABC_CLASS_PREVENT_COPYING(module_impl_base)

public:

	/** Constructor.

	TODO: comment signature.
	*/
	module_impl_base();


	/** Destructor.
	*/
	~module_impl_base() {
#if ABC_HOST_API_WIN32
		ABC_ASSERT(m_cRefs == 0);
#endif
	}


#if ABC_HOST_API_WIN32

	/** Increases the number of references to this module.
	*/
	void add_ref() {
		atomic::increment(&m_cRefs);
	}


	/** Decreases the number of references to this module.
	*/
	void release() {
		atomic::decrement(&m_cRefs);
	}


	/** Returns the number of references to this module.

	return
		Reference count.
	*/
	atomic::int_t use_count() {
		return m_cRefs;
	}

#endif //if ABC_HOST_API_WIN32


protected:

	/** Fills up a string vector from the command-line arguments.

	cArgs
		Number of arguments.
	ppszArgs
		Arguments.
	pvsRet
		Vector to receive unsafe istr instances containing each argument.
	*/
	static void _build_args(int cArgs, char_t ** ppszArgs, mvector<istr const> * pvsRet);
#if ABC_HOST_API_WIN32
	// Overload that uses ::GetCommandLine() internally.
	static void _build_args(mvector<istr const> * pvsRet);
#endif


#if ABC_HOST_API_WIN32

	/** Enables clients of a module_impl_base-derived class to pass the module HINSTANCE to the
	underlying module_impl_base, allowing derived class to use a default constructor instead of
	requiring them to conditionally enable a Win32-specific one just to forward the HINSTANCE.

	hinst
		Handle to the module’s instance.
	*/
	static void _preconstruct(HINSTANCE hinst) {
		sm_hinst = hinst;
	}

#endif //if ABC_HOST_API_WIN32


private:

#if ABC_HOST_API_WIN32
	/** Reference count. Used by module types that decide for themselves when they can be discarded
	(DLLs, services). */
	atomic::int_t mutable volatile m_cRefs;
#endif

private:

#if ABC_HOST_API_WIN32
	/** Stores the Windows-provided module HINSTANCE, so it can be provided to the constructors of
	code_module and resource_module. */
	static HINSTANCE sm_hinst;
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::module_impl


namespace abc {

/** Partial implementation of an executable module.
*/
template <class T>
class module_impl :
	public module_impl_base {
public:

	/** Constructor.
	*/
	module_impl() {
		sm_ptOnlyInstance = static_cast<T *>(this);
	}


	/** Destructor.
	*/
	~module_impl() {
		sm_ptOnlyInstance = NULL;
	}


private:

	/** Pointer to the one and only instance of the application-defined module class. */
	static T * sm_ptOnlyInstance;
};

} //namespace abc

/** Defines the static members of a abc::module_impl specialization.
*/
#define ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
	template <> \
	/*static*/ cls * ::abc::module_impl<cls>::sm_ptOnlyInstance(NULL);


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::app_module_impl


namespace abc {

/** Partial implementation of an application module.
*/
template <class T>
class app_module_impl :
	public module_impl<T> {
public:

	/** C-style entry point for executables.

	cArgs
		Count of arguments.
	ppszArgs
		Arguments.
	return
		Return code of the program.
	*/
	static int entry_point_main(int cArgs, char_t ** ppszArgs) {
		// Establish this as early as possible.
		exception::async_handler_manager eahm;
		try {
			// Create and initialize the module.
			T t;

			// Use a smvector to avoid dynamic allocation for just a few arguments.
			smvector<istr const, 8> vsArgs;
			module_impl<T>::_build_args(cArgs, ppszArgs, &vsArgs);

			// Invoke the program-defined main().
			return t.main(vsArgs);
		} catch (std::exception const & x) {
			exception::write_with_scope_trace(NULL, &x);
			return 123;
		} catch (...) {
			exception::write_with_scope_trace();
			return 123;
		}
	}


#if ABC_HOST_API_WIN32

	/** Entry point for Windows executables.

	hinst
		Module’s instance handle.
	iShowCmd
		Indication on how the application’s main window should be displayed; one of SW_* flags.
	*/
	static int entry_point_win_exe(HINSTANCE hinst, int iShowCmd) {
		UNUSED_ARG(iShowCmd);

		// Establish this as early as possible.
		exception::async_handler_manager eahm;
		try {
			// Create and initialize the module.
			_preconstruct(hinst);
			T t;

			smvector<istr const, 8> vsArgs;

			// Invoke the program-defined main().
			return t.main(vsArgs);
		} catch (std::exception const & x) {
			exception::write_with_scope_trace(NULL, &x);
			return 123;
		} catch (...) {
			exception::write_with_scope_trace();
			return 123;
		}
	}

#endif //if ABC_HOST_API_WIN32


// Overridables to define the behavior of the application.
public:

	/** Entry point of the application.

	vsArgs
		Command-line arguments.
	return
		Return code of the program.
	*/
	int main(mvector<istr const> const & vsArgs) {
		UNUSED_ARG(vsArgs);
		return 0;
	}
};

} //namespace abc


/** Declares an abc::app_module_impl-derived class as being the main module for the application.

cls
	Main abc::app_module_impl-derived class.
*/
#if ABC_HOST_API_POSIX
	#define ABC_MAIN_APP_MODULE(cls) \
		ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
		\
		extern "C" int main(int cArgs, char ** ppszArgs) { \
			return cls::entry_point_main(cArgs, ppszArgs); \
		}
#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX
	// TODO: find a way to define ABC_HOST_API_WIN32_GUI, and maybe come up with a better name.
	#ifdef ABC_HOST_API_WIN32_GUI
		#define ABC_MAIN_APP_MODULE(cls) \
			ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
			\
			extern "C" int WINAPI wWinMain( \
				HINSTANCE hinst, HINSTANCE, wchar_t * pszCmdLine, int iShowCmd \
			) { \
				UNUSED_ARG(pszCmdLine); \
				return cls::entry_point_win_exe(hinst, iShowCmd); \
			}
	#else
		#define ABC_MAIN_APP_MODULE(cls) \
			ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
			\
			extern "C" int ABC_STL_CALLCONV wmain(int cArgs, wchar_t ** ppszArgs) { \
				return cls::entry_point_main(cArgs, ppszArgs); \
			}
	#endif
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: OUTPUT
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::library_module_impl


namespace abc {

/** Partial implementation of a shared library module.
*/
template <class T>
class library_module_impl :
	public module_impl<T> {
public:

#if ABC_HOST_API_POSIX

	// TODO

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

	/** Entry point for Windows DLLs.

	hinst
		Module’s instance handle.
	iReason
		Reason why the DLL entry point was invoked; one of DLL_{PROCESS,THREAD}_{ATTACH,DETACH}.
	return
		true in case of success, or false otherwise.
	*/
	static BOOL entry_point_win_dll(HINSTANCE hinst, DWORD iReason) try {
		switch (iReason) {
			case DLL_PROCESS_ATTACH: {
				// Allocate a new module on the heap, since this function will return immediately.
				_preconstruct(hinst);
				std::unique_ptr<T> pt(new T());
				if (!pt->dll_main(iReason)) {
					return false;
				}
				// If we got to this point, dll_main() succeeded, so we want to avoid deleting pt.
				pt.release();
				return true;
			}

			case DLL_PROCESS_DETACH: {
				// The unique_ptr will take care of deleting pt.
				std::unique_ptr<T> pt(T::sm_ptOnlyInstance);
				return pt->dll_main(iReason);
			}

			case DLL_THREAD_ATTACH:
			case DLL_THREAD_DETACH: {
				T * pt(T::sm_ptOnlyInstance);
				return pt->dll_main(iReason);
			}

			default:
				return true;
		}
	} catch (std::exception const & x) {
		exception::write_with_scope_trace(NULL, &x);
		return false;
	} catch (...) {
		exception::write_with_scope_trace();
		return false;
	}

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: OUTPUT
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


// Overridables to define the behavior of the library.
public:

#if ABC_HOST_API_POSIX

	// TODO

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_POSIX

	/** TODO: comment signature.
	*/
	bool dll_main(int iReason) {
		UNUSED_ARG(iReason);
		return true;
	}


	/** Invoked by COM to determine whether the DLL is no longer in use and can be unloaded.

	return
		S_OK if the DLL is no longer needed, or S_FALSE otherwise.
	*/
	HRESULT DllCanUnloadNow() {
		return use_count() > 0 ? S_FALSE : S_OK;
	}

#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: OUTPUT
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else
};

} //namespace abc


/** Declares an abc::library_module_impl-derived class as being the main module for the library.

cls
	Main abc::library_module_impl-derived class.
*/
#if ABC_HOST_API_POSIX
	#define ABC_MAIN_LIBRARY_MODULE(cls) \
		ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
		\
		extern "C" int main(int cArgs, char ** ppszArgs) { \
			return cls::entry_point_main(cArgs, ppszArgs); \
		}
#elif ABC_HOST_API_WIN32
	#define ABC_MAIN_LIBRARY_MODULE(cls) \
		ABC_DEFINE_MODULE_IMPL_SPEC_STATICS(cls) \
		\
		extern "C" BOOL WINAPI DllMain(HINSTANCE hinst, DWORD iReason, void * pReserved) { \
			UNUSED_ARG(pReserved); \
			return cls::entry_point_win_dll(hinst, iReason); \
		}
#else //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32
	#error TODO-PORT: OUTPUT
#endif //if ABC_HOST_API_POSIX … elif ABC_HOST_API_WIN32 … else


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_MODULE_HXX


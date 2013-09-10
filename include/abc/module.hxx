// -*- coding: utf-8; mode: c++; tab-width: 3 -*-
//--------------------------------------------------------------------------------------------------
// Application-Building Components
// Copyright 2010-2013 Raffaello D. Di Napoli
//--------------------------------------------------------------------------------------------------
// This file is part of Application-Building Components (henceforth referred to as ABC).
//
// ABC is free software: you can redistribute it and/or modify it under the terms of the GNU General
// Public License as published by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// ABC is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the
// implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
// Public License for more details.
//
// You should have received a copy of the GNU General Public License along with ABC. If not, see
// <http://www.gnu.org/licenses/>.
//--------------------------------------------------------------------------------------------------

#ifndef ABC_MODULE_HXX
#define ABC_MODULE_HXX

#ifdef ABC_CXX_PRAGMA_ONCE
	#pragma once
#endif

#include <abc/atomic.hxx>
#include <abc/vector.hxx>
#include <abc/file_path.hxx>
#include <stdlib.h> // EXIT_SUCCESS EXIT_FAILURE



////////////////////////////////////////////////////////////////////////////////////////////////////
// Declarations

namespace abc {

#if ABC_OUTPUT_POSIX_EXE
	#define ABC_DECLARE_MODULE_IMPL_CLASS(cls) \
		extern "C" int main(int cArgs, char_t ** ppszArgs) { \
			return cls::entry_point_main(cArgs, ppszArgs); \
		}
#elif ABC_OUTPUT_WIN32_EXE
	#define ABC_DECLARE_MODULE_IMPL_CLASS(cls) \
		extern "C" int WINAPI wWinMain( \
			HINSTANCE hinst, HINSTANCE, wchar_t * pszCmdLine, int iShowCmd \
		) { \
			UNUSED_ARG(pszCmdLine); \
			return cls::entry_point_win_exe(hinst, iShowCmd); \
		}
#elif ABC_OUTPUT_WIN32_DLL
	#define ABC_DECLARE_MODULE_IMPL_CLASS(cls) \
		extern "C" BOOL WINAPI DllMain(HINSTANCE hinst, DWORD iReason, void * pReserved) { \
			UNUSED_ARG(pReserved); \
			return cls::entry_point_win_dll(hinst, iReason); \
		}
#else
	#error TODO-PORT: OUTPUT
#endif


/// Thread ID type.
#if ABC_HOST_API_POSIX
	typedef pthread_t tid_t;
#elif ABC_HOST_API_WIN32
	typedef DWORD tid_t;
#else
	#error TODO-PORT: HOST_API
#endif

/// Process ID type.
#if ABC_HOST_API_POSIX
	// pid_t is already defined.
#elif ABC_HOST_API_WIN32
	typedef DWORD pid_t;
#else
	#error TODO-PORT: HOST_API
#endif

/// Native OS dynamic library/module handle.
#if ABC_HOST_API_POSIX
	typedef void * hdynmod_t;
#elif ABC_HOST_API_WIN32
	typedef HINSTANCE hdynmod_t;
#else
	#error TODO-PORT: HOST_API
#endif


/// Base class for threads.
template <typename T>
class thread;

/// Base class for processes.
template <typename T>
class process;


#if ABC_HOST_API_WIN32
/// Dynamically loadable module.
class dynamic_module;
#endif

/// Resource dynamically loadable module.
class resource_module;

/// Code dynamically loadable module.
class code_module;

/// Base class for implementing a dynamically loadable module.
class module_impl_base;

/// Pointer to the module_impl_base interface of the application-defined module object.
extern module_impl_base * g_pmib;

/// Partial implementation of an executable module.
template <class T>
class module_impl;

} //namespace abc



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread


namespace abc {

template <typename T>
class thread :
	public T {
private:

	/// Thread ID (TID).
	tid_t m_tid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::process


namespace abc {

template <typename T>
class process :
	public T {
private:

	/// Process ID (PID).
	pid_t m_pid;
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::dynamic_module


#if ABC_HOST_API_WIN32

namespace abc {

class dynamic_module {

	ABC_CLASS_PREVENT_COPYING(dynamic_module)

public:

	/// Constructor.
	//
	dynamic_module(dynamic_module && dm);
	dynamic_module(file_path const & fp, bool bInit);
	dynamic_module(hdynmod_t hdynmod) :
		m_hdynmod(hdynmod),
		m_bOwn(false) {
	}


	/// Destructor.
	//
	~dynamic_module() {
		if (m_bOwn) {
			::FreeLibrary(m_hdynmod);
		}
	}


	/// Assignment operator.
	dynamic_module & operator=(dynamic_module && dm);

	/// Returns the file name of the module.
	file_path get_file_name() const;


private:

	/// Handle to the module.
	hdynmod_t m_hdynmod;
	/// If false, the handle was provided by the caller of the constructor, and it will not be
	// released.
	bool m_bOwn;
};

} //namespace abc

#endif //if ABC_HOST_API_WIN32


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::resource_module


namespace abc {

class resource_module
#if ABC_HOST_API_WIN32
	: public dynamic_module
#endif
{

	ABC_CLASS_PREVENT_COPYING(resource_module)

public:

	/// Constructor.
	//
	resource_module(file_path const & fp);
	resource_module(resource_module && rm);


	/// Destructor.
	~resource_module();


	/// Assignment operator.
	//
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


	/// Loads a string from the module’s resources.
	size_t load_string(short id, char_t * psz, size_t cchMax) const;


protected:

	/// Constructor. This overload is meant to be used by module_impl_base, so that it can supply its
	// own HINSTANCE (Win), which must not be released upon destruction of this object.
	//
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

class code_module
#if ABC_HOST_API_WIN32
	: public dynamic_module
#endif
{

	ABC_CLASS_PREVENT_COPYING(code_module)

public:

	/// Constructor.
	code_module(file_path const & fp);
	code_module(code_module && cm);

	/// Destructor.
	~code_module();


	/// Assignment operator.
	//
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


	/// Returns a pointer to the specified symbol in the module.
	//
	template <typename F>
	F get_symbol(cstring const & sSymbol, F * ppfn = NULL) {
		F pfn(reinterpret_cast<F>(_get_symbol(sSymbol)));
		if (ppfn) {
			*ppfn = pfn;
		}
		return pfn;
	}


protected:

	/// Constructor. This overload is meant to be used by module_impl_base, so that it can supply its
	// own HINSTANCE (Win) or nothing (POSIX.1-2001), which must not be released upon destruction of
	// this object.
	//
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

	/// Returns a void pointer to the specified symbol in the module.
	void * _get_symbol(cstring const & sSymbol);


#if ABC_HOST_API_POSIX
private:

	/// Handle to the module.
	hdynmod_t m_hdynmod;
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::module_impl_base


namespace abc {

class module_impl_base :
	public code_module,
	public resource_module {

	ABC_CLASS_PREVENT_COPYING(module_impl_base)

public:

	/// Constructor.
	module_impl_base();


	/// Destructor.
	//
	~module_impl_base() {
#if ABC_HOST_API_WIN32
		assert(m_cRefs == 0);
#endif
		g_pmib = NULL;
	}


#if ABC_HOST_API_WIN32

	/// Increases the number of references to this module.
	//
	void add_ref() {
		atomic::increment(&m_cRefs);
	}


	/// Decreases the number of references to this module.
	//
	void release() {
		atomic::decrement(&m_cRefs);
	}


	/// Returns the number of references to this module.
	//
	bool use_count() {
		return m_cRefs;
	}

#endif //if ABC_HOST_API_WIN32


protected:

	/// Fills up a string vector from the command line arguments.
	static void _build_args(
#if ABC_HOST_API_POSIX
		int cArgs, char_t ** ppszArgs,
#elif ABC_HOST_API_WIN32
		// Will use ::GetCommandLine() internally.
#else
	#error TODO-PORT: HOST_API
#endif
		buffered_vector<cstring const> * pvsRet
	);


#if ABC_HOST_API_WIN32

	/// Enables clients of a module_impl_base-derived class to pass the module HINSTANCE to the
	// underlying module_impl_base, allowing derived class to use a default constructor instead of
	// requiring them to conditionally enable a Win32-specific one just to forward the HINSTANCE.
	//
	void _preconstruct(HINSTANCE hinst) {
		sm_hinst = hinst;
	}

#endif //if ABC_HOST_API_WIN32


private:

#if ABC_HOST_API_WIN32
	/// Reference count. Used by module types that decide for themselves when they can be discarded
	// (DLLs, services).
	atomic::int_t mutable volatile m_cRefs;
#endif

private:

#if ABC_HOST_API_WIN32
	/// Stores the Windows-provided module HINSTANCE, so it can be provided to the constructors of
	// code_module and resource_module.
	static HINSTANCE sm_hinst;
#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::module_impl


namespace abc {

template <class T>
class module_impl :
	public module_impl_base {
public:

#if ABC_OUTPUT_POSIX_EXE

	/// Entry point for POSIX executables.
	//
	static int entry_point_main(int cArgs, char_t ** ppszArgs) {
		// Establish this as early as possible.
		exception::async_handler_manager eahm;
		try {
			// Create and initialize the module.
			T t;

			// Use a vector<cstring const> to avoid dynamic allocation of the vector’s array for just a
			// few arguments.
			vector<cstring const, 8> vsArgs;
			_build_args(cArgs, ppszArgs, &vsArgs);

			// Invoke the program-defined main().
			return t.main(vsArgs);
		} catch (std::exception const & x) {
			exception::_uncaught_exception_end(&x);
			return 123;
		} catch (...) {
			exception::_uncaught_exception_end();
			return 123;
		}
	}

#elif ABC_OUTPUT_WIN32_EXE

	/// Entry point for Windows executables.
	//
	static int entry_point_win_exe(HINSTANCE hinst, int iShowCmd) {
		UNUSED_ARG(iShowCmd);

		// Establish this as early as possible.
		exception::async_handler_manager eahm;
		try {
			// Create and initialize the module.
			_preconstruct(hinst);
			T t;

			vector<cstring const, 8> vsArgs;

			// Invoke the program-defined main().
			return t.main(vsArgs);
		} catch (std::exception const & x) {
			exception::_uncaught_exception_end(&x);
			return 123;
		} catch (...) {
			exception::_uncaught_exception_end();
			return 123;
		}
	}

#elif ABC_OUTPUT_WIN32_DLL

	/// Entry point for Windows DLLs.
	//
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
				std::unique_ptr<T> pt(static_cast<T *>(g_pmib));
				return pt->dll_main(iReason);
			}

			case DLL_THREAD_ATTACH:
			case DLL_THREAD_DETACH: {
				T * pt(static_cast<T *>(g_pmib));
				return pt->dll_main(iReason);
			}

			default:
				return true;
		}
	} catch (std::exception const & x) {
		exception::_uncaught_exception_end(&x);
		return false;
	} catch (...) {
		exception::_uncaught_exception_end();
		return false;
	}

#else
	#error TODO-PORT: OUTPUT
#endif


// Overridables to define the behavior of the program/library.
public:

#if ABC_OUTPUT_POSIX_EXE || ABC_OUTPUT_WIN32_EXE

	int main(vector<cstring const> const & vsArgs) {
		UNUSED_ARG(vsArgs);
		return EXIT_SUCCESS;
	}

#elif ABC_OUTPUT_WIN32_DLL

	bool dll_main(int iReason) {
		UNUSED_ARG(iReason);
		return true;
	}


	HRESULT DllCanUnloadNow() {
		return use_count() > 0 ? S_FALSE : S_OK;
	}

#endif
};

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


#endif //ifndef ABC_MODULE_HXX


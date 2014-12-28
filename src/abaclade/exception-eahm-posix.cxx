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

// #include <abaclade.hxx> already done in exception-os.cxx.

#include <cstdlib> // std::abort()
#include <signal.h> // sigaction sig*()
#include <ucontext.h> // ucontext_t


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception

namespace abc {

// These should be member variables of exception::async_handler_manager, but they’re not due to
// their header file requirements.

namespace {

//! Signals that we can translate into C++ exceptions.
int const g_aiHandledSignals[] = {
// Signal (Default action) Description (standard).
// SIGABRT, // (Core) Abort signal from abort(3) (POSIX.1-1990).
// SIGALRM, // (Term) Timer signal from alarm(2) (POSIX.1-1990).
   SIGBUS,  // (Core) Bus error (bad memory access) (POSIX.1-2001).
// SIGCHLD, // (Ign ) Child stopped or terminated (POSIX.1-1990).
// SIGCONT, // (Cont) Continue if stopped (POSIX.1-1990).
   SIGFPE,  // (Core) Floating point exception (POSIX.1-1990).
// SIGHUP,  // (Term) Hangup on controlling terminal or death of controlling process (POSIX.1-1990).
// SIGILL,  // (Core) Illegal Instruction (POSIX.1-1990).
// SIGINT,  // (Term) Interrupt from keyboard (POSIX.1-1990).
// SIGPIPE, // (Term) Broken pipe: write to pipe with no readers (POSIX.1-1990).
// SIGPROF, // (Term) Profiling timer expired (POSIX.1-2001).
// SIGQUIT, // (Core) Quit from keyboard (POSIX.1-1990).
   SIGSEGV  // (Core) Invalid memory reference (POSIX.1-1990).
// SIGTERM  // (Term) Termination signal (POSIX.1-1990).
// SIGTRAP  // (Core) Trace/breakpoint trap (POSIX.1-2001).
// SIGTSTP  // (Stop) Stop typed at terminal (POSIX.1-1990).
// SIGTTIN  // (Stop) Terminal input for background process (POSIX.1-1990).
// SIGTTOU  // (Stop) Terminal output for background process (POSIX.1-1990).
// SIGUSR1  // (Term) User-defined signal 1 (POSIX.1-1990).
// SIGUSR2  // (Term) User-defined signal 2 (POSIX.1-1990).
};
//! Default handler for each of the signals above.
struct ::sigaction g_asaDefault[ABC_COUNTOF(g_aiHandledSignals)];

//! Possible exception types thrown by throw_after_fault().
ABC_ENUM_AUTO_VALUES(fault_exception_types,
   arithmetic_error,
   division_by_zero_error,
   floating_point_error,
   memory_access_error,
   memory_address_error,
   null_pointer_error,
   overflow_error
);

//! Type of arguments to to throw_after_fault(); see g_ptafa.
struct throw_after_fault_args {
   //! Type of exception to be throw.
   fault_exception_types::enum_type fet;
   //! Exception type-specific argument 0.
   void * pArg;
};

/*! Arguments to throw_after_fault(). Defining this as thread-local instead of real arguments
greatly reduces the amount of processor architecture-specific subroutine call code that needs to be
emulated (and maintained) in fault_handler(). */
thread_local_ptr<throw_after_fault_args> g_ptafa;

void throw_after_fault() {
   throw_after_fault_args * ptafa = g_ptafa.get();
   switch (ptafa->fet) {
      case fault_exception_types::arithmetic_error:
         ABC_THROW(abc::arithmetic_error, ());
      case fault_exception_types::division_by_zero_error:
         ABC_THROW(abc::division_by_zero_error, ());
      case fault_exception_types::floating_point_error:
         ABC_THROW(abc::floating_point_error, ());
      case fault_exception_types::memory_access_error:
         ABC_THROW(abc::memory_access_error, (ptafa->pArg));
      case fault_exception_types::memory_address_error:
         ABC_THROW(abc::memory_address_error, (ptafa->pArg));
      case fault_exception_types::null_pointer_error:
         ABC_THROW(abc::null_pointer_error, ());
      case fault_exception_types::overflow_error:
         ABC_THROW(abc::overflow_error, ());
   }
}

/*! Translates POSIX signals into C++ exceptions, whenever possible. This works by injecting the
stack frame of a call to throw_after_fault(), and then returning, ending processing of the signal.
Execution will resume from throw_after_fault(), which creates the appearance of an C++ exception
being thrown at the location of the offending instruction, without calling any of the (many)
functions that are forbidden in a signal handler.

@param iSignal
   Signal number for which the function is being called.
@param psi
   Additional information on the signal.
@param pctx
   Thread context. This is used to manipulate the stack of the thread to inject a call frame.
*/
void fault_handler(int iSignal, ::siginfo_t * psi, void * pctx) {
   /* Don’t let external programs mess with us: if the source is not the kernel, ignore the error.
   POSIX.1-2008 states that:
      “Historically, an si_code value of less than or equal to zero indicated that the signal was
      generated by a process via the kill() function, and values of si_code that provided additional
      information for implementation-generated signals, such as SIGFPE or SIGSEGV, were all
      positive. […] if si_code is less than or equal to zero, the signal was generated by a process.
      However, since POSIX.1b did not specify that SI_USER (or SI_QUEUE) had a value less than or
      equal to zero, it is not true that when the signal is generated by a process, the value of
      si_code will always be less than or equal to zero. XSI applications should check whether
      si_code is SI_USER or SI_QUEUE in addition to checking whether it is less than or equal to
      zero.”
   So we do exactly that – except we skip checking for SI_USER and SI_QUEUE at this point because
   they don’t apply to many signals this handler takes care of. */
   if (psi->si_code <= 0) {
      return;
   }

   throw_after_fault_args * ptafa = g_ptafa.get();
   switch (iSignal) {
      case SIGBUS:
         /* TODO: this is the only way we can test SIGBUS on x86, otherwise the program will get
         stuck in an endless memory-allocating loop. How can this be made to only execute when
         running that one test? */

         // Disable alignment checking if the architecture supports it.
#ifdef __GNUC__
   #if defined(__i386__)
         __asm__(
            "pushf\n"
            "andl $0xfffbffff,(%esp)\n"
            "popf"
         );
   #elif defined(__x86_64__)
         __asm__(
            "pushf\n"
            "andl $0xfffffffffffbffff,(%rsp)\n"
            "popf"
         );
   #endif
#endif //ifdef __GNUC__

         /* There aren’t many codes here that are safe to handle; most of them indicate that there
         is some major memory corruption going on, and in that case we really don’t want to keep on
         going – even the code to throw an exception could be compromised. */
         switch (psi->si_code) {
            case BUS_ADRALN: // Invalid address alignment.
               ptafa->fet = fault_exception_types::memory_access_error;
               ptafa->pArg = psi->si_addr;
               break;
            default:
               std::abort();
         }
         break;

      case SIGFPE:
         switch (psi->si_code) {
            case FPE_INTDIV: // Integer divide by zero.
               ptafa->fet = fault_exception_types::division_by_zero_error;
               break;
            case FPE_INTOVF: // Integer overflow.
               ptafa->fet = fault_exception_types::overflow_error;
               break;
            case FPE_FLTDIV: // Floating-point divide by zero.
            case FPE_FLTOVF: // Floating-point overflow.
            case FPE_FLTUND: // Floating-point underflow.
            case FPE_FLTRES: // Floating-point inexact result.
            case FPE_FLTINV: // Floating-point invalid operation.
            case FPE_FLTSUB: // Subscript out of range.
               ptafa->fet = fault_exception_types::floating_point_error;
               break;
            default:
               /* At the time of writing, the above case labels don’t leave out any values, but
               that’s not necessarily going to be true in 5 years, so… */
               ptafa->fet = fault_exception_types::arithmetic_error;
               break;
         }
         break;

      case SIGSEGV:
         if (psi->si_addr == nullptr) {
            ptafa->fet = fault_exception_types::null_pointer_error;
         } else {
            ptafa->fet = fault_exception_types::memory_address_error;
            ptafa->pArg = psi->si_addr;
         }
         break;

      default:
         /* Handle all unrecognized cases here. Since here we only handle signals for which the
         default actions is a core dump, calling abort (which sends SIGABRT, also causing a core
         dump) is the same as invoking the default action. */
         std::abort();
   }

   // Obtain the faulting thread’s context and the instruction and stack pointers.
   void ** ppCode;
   std::intptr_t ** ppiStack;
   ::ucontext_t * puctx = static_cast< ::ucontext_t *>(pctx);
#if ABC_HOST_API_LINUX
   #if defined(__i386__)
      ppCode = reinterpret_cast<void **>(&puctx->uc_mcontext.gregs[REG_EIP]);
      ppiStack = reinterpret_cast<std::intptr_t **>(&puctx->uc_mcontext.gregs[REG_ESP]);
   #elif defined(__x86_64__)
      ppCode = reinterpret_cast<void **>(&puctx->uc_mcontext.gregs[REG_RIP]);
      ppiStack = reinterpret_cast<std::intptr_t **>(&puctx->uc_mcontext.gregs[REG_RSP]);
   #else
      #error "TODO: HOST_ARCH"
   #endif
#elif ABC_HOST_API_FREEBSD //if ABC_HOST_API_LINUX
   #if defined(__i386__)
      ppCode = reinterpret_cast<void **>(&puctx->uc_mcontext.mc_eip);
      ppiStack = reinterpret_cast<std::intptr_t **>(&puctx->uc_mcontext.mc_esp);
   #elif defined(__x86_64__)
      ppCode = reinterpret_cast<void **>(&puctx->uc_mcontext.mc_rip);
      ppiStack = reinterpret_cast<std::intptr_t **>(&puctx->uc_mcontext.mc_rsp);
   #else
      #error "TODO: HOST_ARCH"
   #endif
#else //if ABC_HOST_API_LINUX … elif ABC_HOST_API_FREEBSD
   #error "TODO: HOST_API"
#endif //if ABC_HOST_API_LINUX … elif ABC_HOST_API_FREEBSD … else
   /* Push the address of the current (failing) instruction, then jump to the address of the
   appropriate thrower function. This emulates a subroutine call. */
   *--*ppiStack = reinterpret_cast<std::intptr_t>(*ppCode);
   *ppCode = reinterpret_cast<void *>(&throw_after_fault);
}

} //namespace


exception::async_handler_manager::async_handler_manager() {
   // Initialize the arguments for fault_handler().
   g_ptafa.reset_new();

   struct ::sigaction saNew;
   saNew.sa_sigaction = &fault_handler;
   sigemptyset(&saNew.sa_mask);
   /* Without SA_NODEFER (POSIX.1-2001), the handler would be disabled during its own execution,
   only to be restored when the handler returns. Since we’ll throw a C++ exception from within the
   handler, the restoration would be skipped, and if the signal were raised again, we’d just crash.
   SA_SIGINFO (POSIX.1-2001) provides the handler with more information about the signal, which we
   use to generate more precise exceptions. */
   saNew.sa_flags = SA_NODEFER | SA_SIGINFO;

   // Setup handlers for the signals in g_aiHandledSignals.
   for (std::size_t i = ABC_COUNTOF(g_aiHandledSignals); i-- > 0; ) {
      ::sigaction(g_aiHandledSignals[i], &saNew, &g_asaDefault[i]);
   }
}

exception::async_handler_manager::~async_handler_manager() {
   // Restore the saved signal handlers.
   for (std::size_t i = ABC_COUNTOF(g_aiHandledSignals); i-- > 0; ) {
      ::sigaction(g_aiHandledSignals[i], &g_asaDefault[i], nullptr);
   }
}

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

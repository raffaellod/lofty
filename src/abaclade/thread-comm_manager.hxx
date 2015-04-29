/* -*- coding: utf-8; mode: c++; tab-width: 3; indent-tabs-mode: nil -*-

Copyright 2015
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

#include <abaclade.hxx>
#include <abaclade/thread.hxx>


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::thread::comm_manager

namespace abc {

class thread::comm_manager : public noncopyable {
public:
   //! Constructor.
   comm_manager();

   //! Destructor.
   ~comm_manager();

private:
#if ABC_HOST_API_POSIX
   /*! Handles Abaclade-defined signals used to interrupt threads, injecting an exception in the
   thread’s context.

   @param iSignal
      Signal number for which the function is being called.
   @param psi
      Additional information on the signal.
   @param pctx
      Thread context. This is used to manipulate the stack of the thread to inject a call frame.
   */
   static void execution_interruption_signal_handler(int iSignal, ::siginfo_t * psi, void * pctx);
#endif

public:
#if ABC_HOST_API_POSIX
   static int sm_iExecutionInterruptionSignal;
#endif
};

} //namespace abc

////////////////////////////////////////////////////////////////////////////////////////////////////

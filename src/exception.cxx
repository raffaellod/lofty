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

#include <abc/core.hxx>
#include <abc/file_iostream.hxx>
#include <abc/trace.hxx>
#if ABC_HOST_API_LINUX
	#include <errno.h> // errno E*
	#include <signal.h> // sigaction sig*()
#endif



////////////////////////////////////////////////////////////////////////////////////////////////////
// abc globals


namespace abc {

#if ABC_HOST_API_POSIX

ABCAPI void throw_os_error() {
	throw_os_error(errno);
}
ABCAPI void throw_os_error(errint_t err) {
	ABC_ASSERT(err != 0);
	switch (err) {
		case E2BIG: // Argument list too long (POSIX.1-2001)
		case EBADF: // Bad file number (POSIX.1-2001)
#ifdef EBADFD
		case EBADFD: // File descriptor in bad state (Linux)
#endif
		case EBADMSG: // Bad message (POSIX.1-2001)
#ifdef EBADR
		case EBADR: // Invalid request descriptor (Linux)
#endif
#ifdef EBADRQC
		case EBADRQC: // Invalid request code (Linux)
#endif
#ifdef EBADSLT
		case EBADSLT: // Invalid slot (Linux)
#endif
#ifdef ECHRNG
		case ECHRNG: // Channel number out of range (Linux)
#endif
		case EDESTADDRREQ: // Destination address required (POSIX.1-2001)
		case EINVAL: // Invalid argument (POSIX.1-2001)
		case EMSGSIZE: // Message too long (POSIX.1-2001)
		case ENAMETOOLONG: // File name too long (POSIX.1-2001)
#ifdef ENOTBLK
		case ENOTBLK: // Block device required (Linux)
#endif
		case ENOTSOCK: // Socket operation on non-socket (POSIX.1-2001)
			abc_throw(argument_error, (err));


		case ERANGE: // Math result not representable (POSIX.1-2001, C99)
			abc_throw(arithmetic_error, (err));


#ifdef ENOBUFS
		case ENOBUFS: // No buffer space available (Linux)
			abc_throw(buffer_error, (err));
#endif


		case EDOM: // Math argument out of domain of func (POSIX.1-2001, C99)
			abc_throw(domain_error, (err));


		case ECHILD: // No child processes (POSIX.1-2001)
		case EDEADLK: // Resource deadlock avoided (POSIX.1-2001)
		case EINTR: // Interrupted function call (POSIX.1-2001)
		case ENOEXEC: // Exec format error (POSIX.1-2001)
		case ENOLCK: // No locks available (POSIX.1-2001)
		case ESRCH: // No such process (POSIX.1-2001)
			abc_throw(environment_error, (err));


		case ENODEV: // No such device (POSIX.1-2001)
		case ENOENT: // No such file or directory (POSIX.1-2001)
			abc_throw(file_not_found_error, (err));


		case EIDRM: // Identifier removed (POSIX.1-2001)
		case EILSEQ: // Illegal byte sequence (POSIX.1-2001, C99)
		case EMULTIHOP: // Multihop attempted (POSIX.1-2001)
		case ENOPROTOOPT: // Protocol not available (POSIX.1-2001)
		default:
			abc_throw(generic_error, (err));


		case EAGAIN: // Try again (POSIX.1-2001)
		case EALREADY: // Operation already in progress (POSIX.1-2001)
		case EBUSY: // Device or resource busy (POSIX.1-2001)
		case ECANCELED: // Operation canceled (POSIX.1-2001)
		case EDQUOT: // Quota exceeded (POSIX.1-2001)
		case EEXIST: // File exists (POSIX.1-2001)
		case EFBIG: // File too large (POSIX.1-2001)
		case EINPROGRESS: // Operation now in progress (POSIX.1-2001)
		case EIO: // I/O error (POSIX.1-2001)
		case EISDIR: // Is a directory (POSIX.1-2001)
#ifdef EISNAM
		case EISNAM: // Is a named type file (Linux)
#endif
		case ELOOP: // Too many symbolic links encountered (POSIX.1-2001)
#ifdef EMEDIUMTYPE
		case EMEDIUMTYPE: // Wrong medium type (Linux)
#endif
		case EMFILE: // Too many open files (POSIX.1-2001)
		case EMLINK: // Too many links (POSIX.1-2001)
		case ENFILE: // Too many open files in system (POSIX.1-2001)
		case ENODATA: // No data available (POSIX.1-2001)
#ifdef ENOMEDIUM
		case ENOMEDIUM: // No medium found (Linux)
#endif
		case ENOSPC: // No space left on device (POSIX.1-2001)
		case ENOTDIR: // Not a directory (POSIX.1-2001)
		case ENOTEMPTY: // Directory not empty (POSIX.1-2001)
		case ENOTTY: // Not a typewriter (POSIX.1-2001)
		case ENXIO: // No such device or address (POSIX.1-2001)
		case ENOMSG: // No message of the desired type (POSIX.1-2001)
		case ENOTSUP: // Operation not supported (POSIX.1-2001)
		case EPIPE: // Broken pipe (POSIX.1-2001)
		case EROFS: // Read-only file system (POSIX.1-2001)
		case ESPIPE: // Illegal seek (POSIX.1-2001)
		case ESTALE: // Stale NFS file handle (POSIX.1-2001)
#ifdef ESTRPIPE
		case ESTRPIPE: // Streams pipe error (Linux)
#endif
		case ETXTBSY: // Text file busy (POSIX.1-2001)
// These two values may or may not be different.
#if EWOULDBLOCK != EAGAIN
		case EWOULDBLOCK: // Operation would block (POSIX.1-2001)
#endif
		case EXDEV: // Improper link (POSIX.1-2001)
			abc_throw(io_error, (err));


		case ENOMEM: // Out of memory (POSIX.1-2001)
			abc_throw(memory_allocation_error, (err));


		case EFAULT: // Bad address (POSIX.1-2001)
			abc_throw(memory_address_error, (err));


		case EADDRINUSE: // Address already in use (POSIX.1-2001).
		case EADDRNOTAVAIL: // Cannot assign requested address (POSIX.1-2001)
		case EAFNOSUPPORT: // Address family not supported (POSIX.1-2001)
		case EISCONN: // Transport endpoint is already connected (POSIX.1-2001)
#ifdef ENOTUNIQ
		case ENOTUNIQ: // Name not unique on network (Linux)
#endif
// These two values are supposed to differ, but on Linux they don’t.
#if EOPNOTSUPP != ENOTSUP
		case EOPNOTSUPP: // Operation not supported on socket (POSIX.1-2001)
#endif
#ifdef EPFNOSUPPORT
		case EPFNOSUPPORT: // Protocol family not supported (Linux)
#endif
		case EPROTO: // Protocol error (POSIX.1-2001)
		case EPROTONOSUPPORT: // Protocol not supported (POSIX.1-2001)
		case EPROTOTYPE: // Protocol wrong type for socket (POSIX.1-2001)
#ifdef ESOCKTNOSUPPORT
		case ESOCKTNOSUPPORT: // Socket type not supported (Linux)
#endif
			abc_throw(network_error, (err));


#ifdef ECOMM
		case ECOMM: // Communication error on send (Linux)
#endif
		case ECONNABORTED: // Connection aborted (POSIX.1-2001)
		case ECONNREFUSED: // Connection refused (POSIX.1-2001)
		case ECONNRESET: // Connection reset by peer (POSIX.1-2001)
#ifdef EHOSTDOWN
		case EHOSTDOWN: // Host is down (Linux)
#endif
		case EHOSTUNREACH: // No route to host (POSIX.1-2001)
		case ENETDOWN: // Network is down (POSIX.1-2001)
		case ENETRESET: // Connection aborted by network (POSIX.1-2001)
		case ENETUNREACH: // Network is unreachable (POSIX.1-2001)
		case ENOLINK: // Link has been severed (POSIX.1-2001)
		case ENOTCONN: // Transport endpoint is not connected (POSIX.1-2001)
#ifdef EREMCHG
		case EREMCHG: // Remote address changed (Linux)
#endif
#ifdef EREMOTEIO
		case EREMOTEIO: // Remote I/O error (Linux)
#endif
#ifdef ESHUTDOWN
		case ESHUTDOWN: // Cannot send after socket shutdown (Linux)
#endif
		case ETIMEDOUT: // Connection timed out (POSIX.1-2001)
			abc_throw(network_io_error, (err));


		case ENOSYS: // Function not implemented (POSIX.1-2001)
			abc_throw(not_implemented_error, (err));


		case EOVERFLOW: // Value too large for defined data type (POSIX.1-2001)
			abc_throw(overflow_error, (err));


		case EACCES: // Permission denied (POSIX.1-2001)
		case EPERM: // Operation not permitted (POSIX.1-2001)
			abc_throw(security_error, (err));
	}
}

#elif ABC_HOST_API_WIN32

ABCAPI void throw_os_error() {
	throw_os_error(::GetLastError());
}
ABCAPI void throw_os_error(errint_t err) {
	ABC_ASSERT(err != ERROR_SUCCESS);
	switch (err) {
		// TODO: Win32 defines these “positive failures”: what to do? They’re not generic_error’s, so
		// we shouldn’t throw due to them.
		case ERROR_SUCCESS_REBOOT_REQUIRED: // The requested operation is successful. Changes will not
			// be effective until the system is rebooted.
		case ERROR_SUCCESS_RESTART_REQUIRED: // The requested operation is successful. Changes will
			// not be effective until the service is restarted.
			break;


		case ERROR_BAD_DESCRIPTOR_FORMAT: // A security descriptor is not in the right format
			// (absolute or self-relative).
		case ERROR_BAD_DEVICE: // The specified device name is invalid.
		case ERROR_BAD_DRIVER: // The specified driver is invalid.
		case ERROR_INVALID_ACCEL_HANDLE: // Invalid accelerator table handle.
		case ERROR_INVALID_ACCESS: // The access code is invalid.
		case ERROR_INVALID_ACCOUNT_NAME: // The name provided is not a properly formed account name.
		case ERROR_INVALID_ACL: // The access control list (ACL) structure is invalid.
		case ERROR_INVALID_AT_INTERRUPT_TIME: // Cannot request exclusive semaphores at interrupt
			// time.
		case ERROR_INVALID_BLOCK: // The storage control block address is invalid.
		case ERROR_INVALID_BLOCK_LENGTH: // When accessing a new tape of a multivolume partition, the
			// current block size is incorrect.
		case ERROR_INVALID_CATEGORY: // The IOCTL call made by the application program is not correct.
		case ERROR_INVALID_COMBOBOX_MESSAGE: // Invalid message for a combo box because it does not
			// have an edit control.
		case ERROR_INVALID_COMPUTERNAME: // The format of the specified computer name is invalid.
		case ERROR_INVALID_CURSOR_HANDLE: // Invalid cursor handle.
		case ERROR_INVALID_DATA: // The data is invalid.
		case ERROR_INVALID_DATATYPE: // The specified data type is invalid.
		case ERROR_INVALID_DOMAINNAME: // The format of the specified domain name is invalid.
		case ERROR_INVALID_DWP_HANDLE: // Invalid handle to a multiple-window position structure.
		case ERROR_INVALID_EA_HANDLE: // The specified extended attribute handle is invalid.
		case ERROR_INVALID_EA_NAME: // The specified extended attribute name was invalid.
		case ERROR_INVALID_EDIT_HEIGHT: // Height must be less than 256.
		case ERROR_INVALID_ENVIRONMENT: // The environment specified is invalid.
		case ERROR_INVALID_EVENT_COUNT: // The number of specified semaphore events for DosMuxSemWait
			// is not correct.
		case ERROR_INVALID_EVENTNAME: // The format of the specified event name is invalid.
		case ERROR_INVALID_FILTER_PROC: // Invalid hook procedure.
		case ERROR_INVALID_FLAG_NUMBER: // The flag passed is not correct.
		case ERROR_INVALID_FLAGS: // Invalid flags.
		case ERROR_INVALID_FORM_NAME: // The specified form name is invalid.
		case ERROR_INVALID_FORM_SIZE: // The specified form size is invalid.
		case ERROR_INVALID_FUNCTION: // Incorrect function.
		case ERROR_INVALID_GROUP_ATTRIBUTES: // The specified attributes are invalid, or incompatible
			// with the attributes for the group as a whole.
		case ERROR_INVALID_GROUPNAME: // The format of the specified group name is invalid.
		case ERROR_INVALID_GW_COMMAND: // Invalid GW_* command.
		case ERROR_INVALID_HANDLE: // The handle is invalid.
		case ERROR_INVALID_HOOK_FILTER: // Invalid hook procedure type.
		case ERROR_INVALID_HOOK_HANDLE: // Invalid hook handle.
		case ERROR_INVALID_ICON_HANDLE: // Invalid icon handle.
		case ERROR_INVALID_ID_AUTHORITY: // The value provided was an invalid value for an identifier
			// authority.
		case ERROR_INVALID_INDEX: // Invalid index.
		case ERROR_INVALID_KEYBOARD_HANDLE: // Invalid keyboard layout handle.
		case ERROR_INVALID_LB_MESSAGE: // Invalid message for single-selection list box.
		case ERROR_INVALID_LEVEL: // The system call level is not correct.
		case ERROR_INVALID_LIST_FORMAT: // The DosMuxSemWait list is not correct.
		case ERROR_INVALID_LOGON_TYPE: // A logon request contained an invalid logon type value.
		case ERROR_INVALID_MENU_HANDLE: // Invalid menu handle.
		case ERROR_INVALID_MESSAGE: // The window cannot act on the sent message.
		case ERROR_INVALID_MESSAGEDEST: // The format of the specified message destination is invalid.
		case ERROR_INVALID_MESSAGENAME: // The format of the specified message name is invalid.
		case ERROR_INVALID_MSGBOX_STYLE: // Invalid message box style.
		case ERROR_INVALID_NAME: // The file name, directory name, or volume label syntax is
			// incorrect.
		case ERROR_INVALID_NETNAME: // The format of the specified network name is invalid.
		case ERROR_INVALID_PARAMETER: // The parameter is incorrect.
		case ERROR_INVALID_PASSWORDNAME: // The format of the specified password is invalid.
		case ERROR_INVALID_PIXEL_FORMAT: // The pixel format is invalid.
		case ERROR_INVALID_PRINT_MONITOR: // The specified print monitor does not have the required
			// functions.
		case ERROR_INVALID_PRINTER_COMMAND: // The printer command is invalid.
		case ERROR_INVALID_PRINTER_NAME: // The printer name is invalid.
		case ERROR_INVALID_PRIORITY: // The specified priority is invalid.
		case ERROR_INVALID_SCROLLBAR_RANGE: // Scroll bar range cannot be greater than 0x7FFF.
		case ERROR_INVALID_SECURITY_DESCR: // The security descriptor structure is invalid.
		case ERROR_INVALID_SEGMENT_NUMBER: // The system detected a segment number that was not
			// correct.
		case ERROR_INVALID_SEPARATOR_FILE: // The specified separator file is invalid.
		case ERROR_INVALID_SERVICE_LOCK: // The specified service database lock is invalid.
		case ERROR_INVALID_SERVICENAME: // The format of the specified service name is invalid.
		case ERROR_INVALID_SHARENAME: // The format of the specified share name is invalid.
		case ERROR_INVALID_SHOWWIN_COMMAND: // Cannot show or remove the window in the way specified.
		case ERROR_INVALID_SID: // The security identifier structure is invalid.
		case ERROR_INVALID_SIGNAL_NUMBER: // The signal being posted is not correct.
		case ERROR_INVALID_SPI_VALUE: // Invalid system-wide (SPI_*) parameter.
		case ERROR_INVALID_TARGET_HANDLE: // The target internal file identifier is incorrect.
		case ERROR_INVALID_THREAD_ID: // Invalid thread identifier.
		case ERROR_INVALID_TIME: // The specified time is invalid.
		case ERROR_INVALID_VERIFY_SWITCH: // The verify-on-write switch parameter value is not
			// correct.
		case ERROR_INVALID_WINDOW_HANDLE: // Invalid window handle.
		case ERROR_INVALID_WINDOW_STYLE: // The window style or class attribute is invalid for this
			// operation.
		case ERROR_SECRET_TOO_LONG: // The length of a secret exceeds the maximum length allowed.
		case ERROR_TLW_WITH_WSCHILD: // Cannot create a top-level child window.
		case ERROR_TOO_MANY_SIDS: // Too many security IDs have been specified.
		case ERROR_UNKNOWN_PRINT_MONITOR: // The specified print monitor is unknown.
		case ERROR_UNKNOWN_PRINTER_DRIVER: // The printer driver is unknown.
		case ERROR_UNKNOWN_PRINTPROCESSOR: // The print processor is unknown.
		case ERROR_UNKNOWN_REVISION: // The revision level is unknown.
		case ERROR_WINDOW_NOT_COMBOBOX: // The window is not a combo box.
		case ERROR_WINDOW_NOT_DIALOG: // The window is not a valid dialog window.
		case ERROR_WINDOW_OF_OTHER_THREAD: // Invalid window; it belongs to another thread.
			abc_throw(argument_error, (err));


		case ERROR_BUFFER_OVERFLOW: // The file name is too long.
		case ERROR_INSUFFICIENT_BUFFER: // The data area passed to a system call is too small.
		case ERROR_INVALID_USER_BUFFER: // The supplied user buffer is not valid for the requested
			// operation.
			abc_throw(buffer_error, (err));


		case ERROR_CHILD_MUST_BE_VOLATILE: // Cannot create a stable subkey under a volatile parent
			// key.
		case ERROR_INVALID_DLL: // One of the library files needed to run this application is damaged.
		case ERROR_INVALID_MINALLOCSIZE: // The operating system cannot run %1.
		case ERROR_INVALID_MODULETYPE: // The operating system cannot run %1.
		case ERROR_INVALID_ORDINAL: // The operating system cannot run %1.
		case ERROR_INVALID_PRINTER_STATE: // The state of the printer is invalid.
		case ERROR_INVALID_SEGDPL: // The operating system cannot run %1.
		case ERROR_INVALID_STACKSEG: // The operating system cannot run %1.
		case ERROR_INVALID_STARTING_CODESEG: // The operating system cannot run %1.
		case ERROR_SET_POWER_STATE_VETOED: // An attempt to change the system power state was vetoed
			// by another application or driver.
		case ERROR_THREAD_1_INACTIVE: // The signal handler cannot be set.
		case ERROR_TOO_MANY_CMDS: // The network BIOS command limit has been reached.
		case ERROR_TOO_MANY_CONTEXT_IDS: // During a logon attempt, the user's security context
			// accumulated too many security IDs.
		case ERROR_TOO_MANY_LUIDS_REQUESTED: // Too many LUIDs were requested at one time.
		case ERROR_TOO_MANY_SEM_REQUESTS: // The semaphore cannot be set again.
		case ERROR_TOO_MANY_SEMAPHORES: // Cannot create another system semaphore.
		case ERROR_TOO_MANY_TCBS: // Cannot create another thread.
		case ERROR_WAIT_NO_CHILDREN: // There are no child processes to wait for.
			abc_throw(environment_error, (err));


		case ERROR_PATH_NOT_FOUND: // The system cannot find the path specified.
		case ERROR_UNKNOWN_PORT: // The specified port is unknown.
			abc_throw(file_not_found_error, (err));


		case ERROR_ALREADY_INITIALIZED: // An attempt was made to perform an initialization operation
			// when initialization has already been completed.
		case ERROR_ALREADY_REGISTERED: // The service is already registered.
		case ERROR_ALREADY_RUNNING_LKG: // The system is currently running with the last-known-good
			// configuration.
		case ERROR_ALREADY_WAITING: // The specified printer handle is already being waited on.
		case ERROR_APP_WRONG_OS: // The specified program is not a Windows or MS-DOS program.
		case ERROR_ARENA_TRASHED: // The storage control blocks were destroyed.
		case ERROR_BAD_ARGUMENTS: // The argument string passed to DosExecPgm is not correct.
		case ERROR_BAD_COMMAND: // The device does not recognize the command.
		case ERROR_BAD_DEV_TYPE: // The network resource type is not correct.
		case ERROR_BAD_DRIVER_LEVEL: // The system does not support the command requested.
		case ERROR_BAD_ENVIRONMENT: // The environment is incorrect.
		case ERROR_BAD_EXE_FORMAT: // Is not a valid application.
		case ERROR_BAD_FORMAT: // An attempt was made to load a program with an incorrect format.
		case ERROR_BAD_LENGTH: // The program issued a command but the command length is incorrect.
		case ERROR_BAD_NET_NAME: // The network name cannot be found.
		case ERROR_BAD_NET_RESP: // The specified server cannot perform the requested operation.
		case ERROR_BAD_PROFILE: // The network connection profile is corrupted.
		case ERROR_BAD_PROVIDER: // The specified network provider name is invalid.
		case ERROR_BAD_REM_ADAP: // The remote adapter is not compatible.
		case ERROR_BAD_THREADID_ADDR: // The address for the thread identifier is not correct.
		case ERROR_BAD_VALIDATION_CLASS: // The validation information class requested was invalid.
		case ERROR_BOOT_ALREADY_ACCEPTED: // The current boot has already been accepted for use as the
			// last-known-good control set.
		case ERROR_BUSY: // The requested resource is in use.
		case ERROR_CALL_NOT_IMPLEMENTED: // This function is not valid on this platform.
		case ERROR_CAN_NOT_COMPLETE: // Cannot complete this function.
		case ERROR_CAN_NOT_DEL_LOCAL_WINS: // The local Windows Internet Naming Service (WINS) cannot
			// be deleted.
		case ERROR_CANCELLED: // The operation was canceled by the user.
		case ERROR_CANNOT_OPEN_PROFILE: // Unable to open the network connection profile.
		case ERROR_CANT_ACCESS_DOMAIN_INFO: // Indicates that a Windows NT Server could not be
			// contacted or that objects within the domain are protected such that necessary
			// information could not be retrieved.
		case ERROR_CHILD_NOT_COMPLETE: // The %1 application cannot be run in Windows NT mode.
		case ERROR_CHILD_WINDOW_MENU: // Child windows cannot have menus.
		case ERROR_CIRCULAR_DEPENDENCY: // Circular service dependency was specified.
		case ERROR_CLASS_HAS_WINDOWS: // Class still has open windows.
		case ERROR_CLIPBOARD_NOT_OPEN: // Thread does not have a clipboard open.
		case ERROR_CLIPPING_NOT_SUPPORTED: // The requested clipping operation is not supported.
		case ERROR_COMMITMENT_LIMIT: // The paging file is too small for this operation to complete.
		case ERROR_CONTINUE: // Continue with work in progress.
		case ERROR_CONTROL_ID_NOT_FOUND: // Control identifier not found.
		case ERROR_DATABASE_DOES_NOT_EXIST: // The database specified does not exist.
		case ERROR_DC_NOT_FOUND: // Invalid device context (DC) handle.
		case ERROR_DDE_FAIL: // An error occurred in sending the command to the application.
		case ERROR_DEPENDENT_SERVICES_RUNNING: // A stop control has been sent to a service that other
			// running services are dependent on.
		case ERROR_DESTROY_OBJECT_OF_OTHER_THREAD: // Cannot destroy object created by another thread.
		case ERROR_DIFFERENT_SERVICE_ACCOUNT: // The account specified for this service is different
			// from the account specified for other services running in the same process.
		case ERROR_DISCARDED: // The segment is already discarded and cannot be locked.
		case ERROR_DLL_INIT_FAILED: // A dynamic link library (DLL) initialization routine failed.
		case ERROR_DLL_NOT_FOUND: // One of the library files needed to run this application cannot be
			// found.
		case ERROR_DUP_DOMAINNAME: // The workgroup or domain name is already in use by another
			// computer on the network.
		case ERROR_DUP_NAME: // A duplicate name exists on the network.
		case ERROR_DUPLICATE_SERVICE_NAME: // The name is already in use as either a service name or a
			// service display name.
		case ERROR_DYNLINK_FROM_INVALID_RING: // The operating system cannot run this application
			// program.
		case ERROR_ENVVAR_NOT_FOUND: // The system could not find the environment option that was
			// entered.
		case ERROR_EVENTLOG_CANT_START: // No event tracking file could be opened, so the event
			// tracking service did not start.
		case ERROR_EVENTLOG_FILE_CHANGED: // The event tracking file has changed between read
			// operations.
		case ERROR_EVENTLOG_FILE_CORRUPT: // The event tracking file is corrupted.
		case ERROR_EXCEPTION_IN_SERVICE: // An exception occurred in the service when handling the
			// control request.
		case ERROR_EXCL_SEM_ALREADY_OWNED: // The exclusive semaphore is owned by another process.
		case ERROR_EXE_MACHINE_TYPE_MISMATCH: // The image file %1 is valid, but is for a machine type
			// other than the current machine.
		case ERROR_EXE_MARKED_INVALID: // The operating system cannot run %1.
		case ERROR_FULLSCREEN_MODE: // The requested operation cannot be performed in full-screen
			// mode.
		case ERROR_GEN_FAILURE: // A device attached to the system is not functioning.
		case ERROR_GRACEFUL_DISCONNECT: // The network connection was gracefully closed.
		case ERROR_GROUP_EXISTS: // The specified group already exists.
		case ERROR_HOOK_NEEDS_HMOD: // Cannot set nonlocal hook without a module handle.
		case ERROR_HOOK_NOT_INSTALLED: // The hook procedure is not installed.
		case ERROR_HOOK_TYPE_NOT_ALLOWED: // Hook type not allowed.
		case ERROR_HOST_UNREACHABLE: // The remote system is not reachable by the transport.
		case ERROR_HOTKEY_ALREADY_REGISTERED: // Hot key is already registered.
		case ERROR_HOTKEY_NOT_REGISTERED: // Hot key is not registered.
		case ERROR_HWNDS_HAVE_DIFF_PARENT: // All handles to windows in a multiple-window position
			// structure must have the same parent.
		case ERROR_INC_BACKUP: // The backup failed. Was a full backup done before?
		case ERROR_INCORRECT_ADDRESS: // The network address could not be used for the operation
			// requested.
		case ERROR_INFLOOP_IN_RELOC_CHAIN: // The operating system cannot run %1.
		case ERROR_INTERNAL_DB_CORRUPTION: // Unable to complete the requested operation because of
			// either a catastrophic media failure or a data structure corruption on the disk.
		case ERROR_INTERNAL_DB_ERROR: // The local security authority database contains an internal
			// inconsistency.
		case ERROR_INTERNAL_ERROR: // The security account database contains an internal
			// inconsistency.
		case ERROR_IOPL_NOT_ENABLED: // The operating system is not presently configured to run this
			// application.
		case ERROR_ITERATED_DATA_EXCEEDS_64k: // The operating system cannot run %1.
		case ERROR_LB_WITHOUT_TABSTOPS: // This list box does not support tab stops.
		case ERROR_LICENSE_QUOTA_EXCEEDED: // The service being accessed is licensed for a particular
			// number of connections. No more connections can be made to the service at this time
			// because there are already as many connections as the service can accept.
		case ERROR_LISTBOX_ID_NOT_FOUND: // The list box identifier was not found.
		case ERROR_LOCK_VIOLATION: // The process cannot access the file because another process has
			// locked a portion of the file.
		case ERROR_LOCKED: // The segment is locked and cannot be reallocated.
		case ERROR_LOG_FILE_FULL: // The event tracking file is full.
		case ERROR_LUIDS_EXHAUSTED: // No more locally unique identifiers (LUIDs) are available.
		case ERROR_MAX_THRDS_REACHED: // No more threads can be created in the system.
		case ERROR_MEDIA_CHANGED: // The media in the drive may have changed.
		case ERROR_MENU_ITEM_NOT_FOUND: // A menu item was not found.
		case ERROR_META_EXPANSION_TOO_LONG: // The global file name characters, "*" or "?," are
			// entered incorrectly or too many global file name characters are specified.
		case ERROR_METAFILE_NOT_SUPPORTED: // The requested metafile operation is not supported.
		case ERROR_MOD_NOT_FOUND: // The specified module could not be found.
		case ERROR_MORE_DATA: // More data is available.
		case ERROR_MR_MID_NOT_FOUND: // The system cannot find message text for message number 0x%1 in
			// the message file for %2.
		case ERROR_NESTING_NOT_ALLOWED: // Cannot nest calls to the LoadModule function.
		case ERROR_NO_PROC_SLOTS: // The system cannot start another process at this time.
		case ERROR_NO_SCROLLBARS: // The window does not have scroll bars.
		case ERROR_NO_SHUTDOWN_IN_PROGRESS: // Unable to abort the system shutdown because no shutdown
			// was in progress.
		case ERROR_NO_SIGNAL_SENT: // No process in the command subtree has a signal handler.
		case ERROR_NO_SPOOL_SPACE: // Space to store the file waiting to be printed is not available
			// on the server.
		case ERROR_NO_SYSTEM_MENU: // The window does not have a system menu.
		case ERROR_NO_WILDCARD_CHARACTERS: // No wildcards were found.
		case ERROR_NOLOGON_INTERDOMAIN_TRUST_ACCOUNT: // The account used is an interdomain trust
			// account. Use your global user account or local user account to access this server.
		case ERROR_NOLOGON_SERVER_TRUST_ACCOUNT: // The account used is a server trust account. Use
			// your global user account or local user account to access this server.
		case ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT: // The account used is a computer account. Use
			// your global user account or local user account to access this server.
		case ERROR_NON_MDICHILD_WINDOW: // Cannot process a message from a window that is not a
			// multiple-document interface (MDI) window.
		case ERROR_NOT_CHILD_WINDOW: // The window is not a child window.
		case ERROR_NOT_CONNECTED: // This network connection does not exist.
		case ERROR_NOT_CONTAINER: // Cannot enumerate a noncontainer.
		case ERROR_NOT_OWNER: // Attempt to release mutex not owned by caller.
		case ERROR_OLD_WIN_VERSION: // The specified program requires a newer version of Windows.
		case ERROR_OPEN_FILES: // This network connection has files open or requests pending.
		case ERROR_PAGEFILE_QUOTA: // Insufficient quota to complete the requested service.
		case ERROR_PARTIAL_COPY: // Only part of a ReadProcessMemory or WriteProcessMemory request was
			// completed.
		case ERROR_POPUP_ALREADY_ACTIVE: // Pop-up menu already active.
		case ERROR_PORT_UNREACHABLE: // No service is operating at the destination network endpoint on
			// the remote system.
		case ERROR_POSSIBLE_DEADLOCK: // A potential deadlock condition has been detected.
		case ERROR_PRIVATE_DIALOG_INDEX: // Using private DIALOG window words.
		case ERROR_PRIVILEGE_NOT_HELD: // A required privilege is not held by the client.
		case ERROR_PROCESS_ABORTED: // The process terminated unexpectedly.
		case ERROR_PROTOCOL_UNREACHABLE: // The remote system does not support the transport protocol.
		case ERROR_REC_NON_EXISTENT: // The name does not exist in the WINS database.
		case ERROR_RELOC_CHAIN_XEEDS_SEGLIM: // The operating system cannot run %1.
		case ERROR_REM_NOT_LIST: // The remote computer is not available.
		case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED: // An attempt was made to establish a session to a
			// network server, but there are already too many sessions established to that server.
		case ERROR_REQUIRES_INTERACTIVE_WINDOWSTATION: // This operation requires an interactive
			// window station.
		case ERROR_RESOURCE_DATA_NOT_FOUND: // The specified image file did not contain a resource
			// section.
		case ERROR_RESOURCE_LANG_NOT_FOUND: // The specified resource language identifier cannot be
			// found in the image file.
		case ERROR_RESOURCE_NAME_NOT_FOUND: // The specified resource name cannot be found in the
			// image file.
		case ERROR_RESOURCE_TYPE_NOT_FOUND: // The specified resource type cannot be found in the
			// image file.
		case ERROR_RETRY: // The operation could not be completed. A retry should be performed.
		case ERROR_REVISION_MISMATCH: // Indicates two revision levels are incompatible.
		case ERROR_RING2_STACK_IN_USE: // The ring 2 stack is in use.
		case ERROR_RING2SEG_MUST_BE_MOVABLE: // The code segment cannot be greater than or equal to 64
			// KB.
		case ERROR_RMODE_APP: // The specified program was written for an earlier version of Windows.
		case ERROR_RPL_NOT_ALLOWED: // Replication with a nonconfigured partner is not allowed.
		case ERROR_SEM_IS_SET: // The semaphore is set and cannot be closed.
		case ERROR_SEM_NOT_FOUND: // The specified system semaphore name was not found.
		case ERROR_SEM_OWNER_DIED: // The previous ownership of this semaphore has ended.
		case ERROR_SEM_TIMEOUT: // The semaphore time-out period has expired.
		case ERROR_SEM_USER_LIMIT: // Insert the diskette for drive %1.
		case ERROR_SERVER_DISABLED: // The server is currently disabled.
		case ERROR_SERVER_HAS_OPEN_HANDLES: // The server is in use and cannot be unloaded.
		case ERROR_SERVER_NOT_DISABLED: // The server is currently enabled.
		case ERROR_SERVICE_ALREADY_RUNNING: // An instance of the service is already running.
		case ERROR_SERVICE_CANNOT_ACCEPT_CTRL: // The service cannot accept control messages at this
			// time.
		case ERROR_SERVICE_DATABASE_LOCKED: // The service database is locked.
		case ERROR_SERVICE_DEPENDENCY_DELETED: // The dependency service does not exist or has been
			// marked for deletion.
		case ERROR_SERVICE_DEPENDENCY_FAIL: // The dependency service or group failed to start.
		case ERROR_SERVICE_DISABLED: // The specified service is disabled and cannot be started.
		case ERROR_SERVICE_DOES_NOT_EXIST: // The specified service does not exist as an installed
			// service.
		case ERROR_SERVICE_EXISTS: // The specified service already exists.
		case ERROR_SERVICE_LOGON_FAILED: // The service did not start due to a logon failure.
		case ERROR_SERVICE_MARKED_FOR_DELETE: // The specified service has been marked for deletion.
		case ERROR_SERVICE_NEVER_STARTED: // No attempts to start the service have been made since the
			// last boot.
		case ERROR_SERVICE_NO_THREAD: // A thread could not be created for the service.
		case ERROR_SERVICE_NOT_ACTIVE: // The service has not been started.
		case ERROR_SERVICE_NOT_FOUND: // The specified service does not exist.
		case ERROR_SERVICE_REQUEST_TIMEOUT: // The service did not respond to the start or control
			// request in a timely fashion.
		case ERROR_SERVICE_SPECIFIC_ERROR: // The service has returned a service-specific error code.
		case ERROR_SERVICE_START_HANG: // After starting, the service stopped responding in a
			// start-pending state.
		case ERROR_SESSION_CREDENTIAL_CONFLICT: // The credentials supplied conflict with an existing
			// set of credentials.
		case ERROR_SET_POWER_STATE_FAILED: // The basic input/output system (BIOS) failed an attempt
			// to change the system power state.
		case ERROR_SHUTDOWN_IN_PROGRESS: // A system shutdown is in progress.
		case ERROR_SIGNAL_PENDING: // A signal is already pending.
		case ERROR_SIGNAL_REFUSED: // The recipient process has refused the signal.
		case ERROR_SINGLE_INSTANCE_APP: // Cannot start more than one instance of the specified
			// program.
		case ERROR_STATIC_INIT: // The importation from the file failed.
		case ERROR_STACK_OVERFLOW: // Recursion too deep; the stack overflowed.
		case ERROR_SWAPERROR: // Error performing inpage operation.
		case ERROR_SYSTEM_TRACE: // System trace information was not specified in your Config.sys
			// file, or tracing is disallowed.
		case ERROR_INVALID_EXE_SIGNATURE: // Cannot run %1 in Windows NT mode.
		case ERROR_PROC_NOT_FOUND: // The specified procedure could not be found.
		case ERROR_SOME_NOT_MAPPED: // Some mapping between account names and security IDs was not
			// done.
		case ERROR_TIMEOUT: // This operation returned because the time-out period expired.
		case ERROR_TRANSFORM_NOT_SUPPORTED: // The requested transformation operation is not
			// supported.
		default:
			abc_throw(generic_error, (err));


		case ERROR_BAD_PATHNAME: // The specified path is invalid.
		case ERROR_INVALID_DRIVE: // The system cannot find the drive specified.
			abc_throw(invalid_path_error, (err));


		case ERROR_ALREADY_ASSIGNED: // The local device name is already in use.
		case ERROR_ALREADY_EXISTS: // Cannot create a file when that file already exists.
		case ERROR_ATOMIC_LOCKS_NOT_SUPPORTED: // The file system does not support atomic changes to
			// the lock type.
		case ERROR_BADDB: // The configuration registry database is corrupt.
		case ERROR_BADKEY: // The configuration registry key is invalid.
		case ERROR_BAD_NETPATH: // The network path was not found.
		case ERROR_BAD_PIPE: // The pipe state is invalid.
		case ERROR_BAD_UNIT: // The system cannot find the specified device .
		case ERROR_BEGINNING_OF_MEDIA: // The beginning of the tape or partition was encountered.
		case ERROR_BROKEN_PIPE: // The pipe has been ended.
		case ERROR_BUS_RESET: // The I/O bus was reset.
		case ERROR_BUSY_DRIVE: // The system cannot perform a JOIN or SUBST at this time.
		case ERROR_CANCEL_VIOLATION: // A lock request was not outstanding for the supplied cancel
			// region.
		case ERROR_CANNOT_COPY: // The copy functions cannot be used.
		case ERROR_CANNOT_MAKE: // The directory or file cannot be created.
		case ERROR_CANTOPEN: // The configuration registry key could not be opened.
		case ERROR_CANTREAD: // The configuration registry key could not be read.
		case ERROR_CANTWRITE: // The configuration registry key could not be written.
		case ERROR_COUNTER_TIMEOUT: // A serial I/O operation completed because the time-out period
			// expired. In other words, the IOCTL_SERIAL_XOFF_COUNTER did not reach zero.
		case ERROR_CRC: // Data error (cyclic redundancy check).
		case ERROR_CURRENT_DIRECTORY: // The directory cannot be removed.
		case ERROR_DEVICE_ALREADY_REMEMBERED: // An attempt was made to remember a device that had
			// previously been remembered.
		case ERROR_DEVICE_IN_USE: // The device is in use by an active process and cannot be
			// disconnected.
		case ERROR_DEVICE_NOT_PARTITIONED: // Tape partition information could not be found when
			// loading a tape.
		case ERROR_DEVICE_REMOVED: // Device has been removed
		case ERROR_DIR_NOT_EMPTY: // The directory is not empty.
		case ERROR_DIR_NOT_ROOT: // The directory is not a subdirectory of the root directory.
		case ERROR_DIRECT_ACCESS_HANDLE: // Attempt to use a file handle to an open disk partition for
			// an operation other than raw disk I/O.
		case ERROR_DIRECTORY: // The directory name is invalid.
		case ERROR_DISK_CHANGE: // The program stopped because an alternate diskette was not inserted.
		case ERROR_DISK_CORRUPT: // The disk structure is corrupted and non-readable.
		case ERROR_DISK_FULL: // There is not enough space on the disk.
		case ERROR_DISK_OPERATION_FAILED: // While accessing the hard disk, a disk operation failed
			// even after retries.
		case ERROR_DISK_RECALIBRATE_FAILED: // While accessing the hard disk, a recalibrate operation
			// failed, even after retries.
		case ERROR_DISK_RESET_FAILED: // While accessing the hard disk, a disk controller reset was
			// needed, but even that failed.
		case ERROR_DRIVE_LOCKED: // The disk is in use or locked by another process.
		case ERROR_EA_FILE_CORRUPT: // The extended attribute file on the mounted file system is
			// corrupt.
		case ERROR_EA_LIST_INCONSISTENT: // The extended attributes are inconsistent.
		case ERROR_EA_TABLE_FULL: // The extended attribute table file is full.
		case ERROR_EAS_DIDNT_FIT: // The extended attributes did not fit in the buffer.
		case ERROR_EAS_NOT_SUPPORTED: // The mounted file system does not support extended attributes.
		case ERROR_END_OF_MEDIA: // The physical end of the tape has been reached.
		case ERROR_EOM_OVERFLOW: // Physical end of tape encountered.
		case ERROR_FILE_CORRUPT: // The file or directory is corrupted and non-readable.
		case ERROR_FILE_EXISTS: // The file exists.
		case ERROR_FILE_INVALID: // The volume for a file has been externally altered so that the
			// opened file is no longer valid.
		case ERROR_FILE_NOT_FOUND: // The system cannot find the file specified.
		case ERROR_FILEMARK_DETECTED: // A tape access reached a filemark.
		case ERROR_FILENAME_EXCED_RANGE: // The file name or extension is too long.
		case ERROR_FLOPPY_BAD_REGISTERS: // The floppy disk controller returned inconsistent results
			// in its registers.
		case ERROR_FLOPPY_ID_MARK_NOT_FOUND: // No identifier address mark was found on the floppy
			// disk.
		case ERROR_FLOPPY_UNKNOWN_ERROR: // The floppy disk controller reported an error that is not
			// recognized by the floppy disk driver.
		case ERROR_FLOPPY_WRONG_CYLINDER: // Mismatch between the floppy disk sector identifier field
			// and the floppy disk controller track address.
		case ERROR_FULL_BACKUP: // The backup failed. Check the directory to which you are backing the
			// database.
		case ERROR_HANDLE_DISK_FULL: // The disk is full.
		case ERROR_HANDLE_EOF: // Reached the end of the file.
		case ERROR_IO_DEVICE: // The request could not be performed because of an I/O device error.
		case ERROR_IO_INCOMPLETE: // Overlapped I/O event is not in a signaled state.
		case ERROR_IO_PENDING: // Overlapped I/O operation is in progress.
		case ERROR_IRQ_BUSY: // Unable to open a device that was sharing an interrupt request (IRQ)
			// with other devices. At least one other device that uses that IRQ was already opened.
		case ERROR_IS_JOIN_PATH: // Not enough resources are available to process this command.
		case ERROR_IS_JOIN_TARGET: // A JOIN or SUBST command cannot be used for a drive that contains
			// previously joined drives.
		case ERROR_IS_JOINED: // An attempt was made to use a JOIN or SUBST command on a drive that
			// has already been joined.
		case ERROR_IS_SUBST_PATH: // The path specified is being used in a substitute.
		case ERROR_IS_SUBST_TARGET: // An attempt was made to join or substitute a drive for which a
			// directory on the drive is the target of a previous substitute.
		case ERROR_IS_SUBSTED: // An attempt was made to use a JOIN or SUBST command on a drive that
			// has already been substituted.
		case ERROR_JOIN_TO_JOIN: // The system tried to join a drive to a directory on a joined drive.
		case ERROR_JOIN_TO_SUBST: // The system tried to join a drive to a directory on a substituted
			// drive.
		case ERROR_JOURNAL_HOOK_SET: // The journal hook procedure is already installed.
		case ERROR_KEY_DELETED: // Illegal operation attempted on a registry key that has been marked
			// for deletion.
		case ERROR_KEY_HAS_CHILDREN: // Cannot create a symbolic link in a registry key that already
			// has subkeys or values.
		case ERROR_LABEL_TOO_LONG: // The volume label you entered exceeds the label character limit
			// of the target file system.
		case ERROR_LOCK_FAILED: // Unable to lock a region of a file.
		case ERROR_MAPPED_ALIGNMENT: // The base address or the file offset specified does not have
			// the proper alignment.
		case ERROR_MORE_WRITES: // A serial I/O operation was completed by another write to the serial
			// port. The IOCTL_SERIAL_XOFF_COUNTER reached zero.
		case ERROR_NEGATIVE_SEEK: // An attempt was made to move the file pointer before the beginning
			// of the file.
		case ERROR_NO_DATA: // The pipe is being closed.
		case ERROR_NO_DATA_DETECTED: // No more data is on the tape.
		case ERROR_NO_LOG_SPACE: // System could not allocate the required space in a registry log.
		case ERROR_NO_MEDIA_IN_DRIVE: // No media in drive.
		case ERROR_NO_MORE_DEVICES: // No more local devices.
		case ERROR_NO_MORE_FILES: // There are no more files.
		case ERROR_NO_MORE_ITEMS: // No more data is available.
		case ERROR_NO_MORE_SEARCH_HANDLES: // No more internal file identifiers available.
		case ERROR_NO_VOLUME_LABEL: // The disk has no volume label.
		case ERROR_NONE_MAPPED: // No mapping between account names and security IDs was done.
		case ERROR_NOT_DOS_DISK: // The specified disk or diskette cannot be accessed.
		case ERROR_NOT_ENOUGH_QUOTA: // Not enough quota is available to process this command.
		case ERROR_NOT_ENOUGH_SERVER_MEMORY: // Not enough server storage is available to process this
			// command.
		case ERROR_NOT_JOINED: // The system tried to delete the JOIN of a drive that is not joined.
		case ERROR_NOT_LOCKED: // The segment is already unlocked.
		case ERROR_NOT_READY: // The device is not ready.
		case ERROR_NOT_REGISTRY_FILE: // The system has attempted to load or restore a file into the
			// registry, but the specified file is not in a registry file format.
		case ERROR_NOT_SAME_DEVICE: // The system cannot move the file to a different disk drive.
		case ERROR_NOT_SUBSTED: // The system tried to delete the substitution of a drive that is not
			// substituted.
		case ERROR_NOTIFY_ENUM_DIR: // A notify change request is being completed and the information
			// is not being returned in the caller's buffer. The caller now needs to enumerate the
			// files to find the changes.
		case ERROR_OPEN_FAILED: // The system cannot open the device or file specified.
		case ERROR_OPERATION_ABORTED: // The I/O operation has been aborted because of either a thread
			// exit or an application request.
		case ERROR_OUT_OF_PAPER: // The printer is out of paper.
		case ERROR_PARTITION_FAILURE: // Tape could not be partitioned.
		case ERROR_PATH_BUSY: // The path specified cannot be used at this time.
		case ERROR_PIPE_BUSY: // All pipe instances are busy.
		case ERROR_PIPE_CONNECTED: // There is a process on other end of the pipe.
		case ERROR_PIPE_LISTENING: // Waiting for a process to open the other end of the pipe.
		case ERROR_PIPE_NOT_CONNECTED: // No process is on the other end of the pipe.
		case ERROR_PRINT_CANCELLED: // Your file waiting to be printed was deleted.
		case ERROR_PRINT_MONITOR_ALREADY_INSTALLED: // The specified print monitor has already been
			// installed.
		case ERROR_PRINT_MONITOR_IN_USE: // The specified print monitor is currently in use.
		case ERROR_PRINT_PROCESSOR_ALREADY_INSTALLED: // The specified print processor has already
			// been installed.
		case ERROR_PRINTER_ALREADY_EXISTS: // The printer already exists.
		case ERROR_PRINTER_DELETED: // The specified printer has been deleted.
		case ERROR_PRINTER_DRIVER_ALREADY_INSTALLED: // The specified printer driver is already
			// installed.
		case ERROR_PRINTER_DRIVER_IN_USE: // The specified printer driver is currently in use.
		case ERROR_PRINTER_HAS_JOBS_QUEUED: // The requested operation is not allowed when there are
			// jobs queued to the printer.
		case ERROR_PRINTQ_FULL: // The printer queue is full.
		case ERROR_READ_FAULT: // The system cannot read from the specified device.
		case ERROR_REDIR_PAUSED: // The specified printer or disk device has been paused.
		case ERROR_REDIRECTOR_HAS_OPEN_HANDLES: // The redirector is in use and cannot be unloaded.
		case ERROR_REGISTRY_CORRUPT: // The registry is corrupted. The structure of one of the files
			// that contains registry data is corrupted, or the system's image of the file in memory is
			// corrupted, or the file could not be recovered because the alternate copy or log was
			// absent or corrupted.
		case ERROR_REGISTRY_IO_FAILED: // An I/O operation initiated by the registry failed
			// unrecoverably. The registry could not read in, or write out, or flush, one of the files
			// that contain the system's image of the registry.
		case ERROR_REGISTRY_RECOVERED: // One of the files in the registry database had to be
			// recovered by use of a log or alternate copy. The recovery was successful.
		case ERROR_RXACT_COMMIT_FAILURE: // An internal security database corruption has been
			// encountered.
		case ERROR_RXACT_INVALID_STATE: // The transaction state of a registry subtree is incompatible
			// with the requested operation.
		case ERROR_SAME_DRIVE: // The system cannot join or substitute a drive to or for a directory
			// on the same drive.
		case ERROR_SECTOR_NOT_FOUND: // The drive cannot find the sector requested.
		case ERROR_SEEK: // The drive cannot locate a specific area or track on the disk.
		case ERROR_SEEK_ON_DEVICE: // The file pointer cannot be set on the specified device or file.
		case ERROR_SERIAL_NO_DEVICE: // No serial device was successfully initialized. The serial
			// driver will unload.
		case ERROR_SETMARK_DETECTED: // A tape access reached the end of a set of files.
		case ERROR_SHARING_BUFFER_EXCEEDED: // Too many files opened for sharing.
		case ERROR_SPL_NO_ADDJOB: // An AddJob call was not issued.
		case ERROR_SPL_NO_STARTDOC: // A StartDocPrinter call was not issued.
		case ERROR_SPOOL_FILE_NOT_FOUND: // The spool file was not found.
		case ERROR_SUBST_TO_JOIN: // The system tried to SUBST a drive to a directory on a joined
			// drive.
		case ERROR_SUBST_TO_SUBST: // The system tried to substitute a drive to a directory on a
			// substituted drive.
		case ERROR_TOO_MANY_LINKS: // An attempt was made to create more links on a file than the file
			// system supports.
		case ERROR_TOO_MANY_OPEN_FILES: // The system cannot open the file.
		case ERROR_USER_MAPPED_FILE: // The requested operation cannot be performed on a file with a
			// user-mapped section open.
		case ERROR_UNABLE_TO_LOCK_MEDIA: // Unable to lock the media eject mechanism.
		case ERROR_UNABLE_TO_UNLOAD_MEDIA: // Unable to unload the media.
		case ERROR_UNRECOGNIZED_MEDIA: // The disk media is not recognized. It may not be formatted.
		case ERROR_UNRECOGNIZED_VOLUME: // The volume does not contain a recognized file system.
			// Verify that all required file system drivers are loaded and that the volume is not
			// corrupted.
		case ERROR_WORKING_SET_QUOTA: // Insufficient quota to complete the requested service.
		case ERROR_WRITE_FAULT: // The system cannot write to the specified device.
		case ERROR_WRITE_PROTECT: // The media is write protected.
		case ERROR_WRONG_DISK: // The wrong diskette is in the drive. Insert %2 (Volume Serial Number:
			// %3) into drive %1.
			abc_throw(io_error, (err));


		case ERROR_CANNOT_FIND_WND_CLASS: // Cannot find window class.
		case ERROR_CLASS_ALREADY_EXISTS: // Class already exists.
		case ERROR_CLASS_DOES_NOT_EXIST: // Class does not exist.
			abc_throw(key_error, (err));


		case ERROR_NO_SYSTEM_RESOURCES: // Insufficient system resources exist to complete the
			// requested service.
		case ERROR_NONPAGED_SYSTEM_RESOURCES: // Insufficient system resources exist to complete the
			// requested service.
		case ERROR_NOT_ENOUGH_MEMORY: // Not enough storage is available to process this command.
		case ERROR_OUT_OF_STRUCTURES: // Storage to process this request is not available.
		case ERROR_OUTOFMEMORY: // Not enough storage is available to complete this operation.
		case ERROR_PAGED_SYSTEM_RESOURCES: // Insufficient system resources exist to complete the
			// requested service.
			abc_throw(memory_allocation_error, (err));


		case ERROR_INVALID_ADDRESS: // Attempt to access invalid address.
		case ERROR_NOACCESS: // Invalid access to memory location.
			abc_throw(memory_address_error, (err));


		case ERROR_ACTIVE_CONNECTIONS: // Active connections still exist.
		case ERROR_ADDRESS_ALREADY_ASSOCIATED: // The network transport endpoint already has an
			// address associated with it.
		case ERROR_CONNECTION_ACTIVE: // An invalid operation was attempted on an active network
			// connection.
		case ERROR_CONNECTION_COUNT_LIMIT: // A connection to the server could not be made because the
			// limit on the number of concurrent connections for this account has been reached.
		case ERROR_CONNECTION_UNAVAIL: // The device is not currently connected but it is a remembered
			// connection.
		case ERROR_DOMAIN_CONTROLLER_NOT_FOUND: // Could not find the domain controller for this
			// domain.
		case ERROR_DOMAIN_EXISTS: // The specified domain already exists.
		case ERROR_DOMAIN_LIMIT_EXCEEDED: // An attempt was made to exceed the limit on the number of
			// domains per server.
		case ERROR_NETLOGON_NOT_STARTED: // An attempt was made to logon, but the network logon
			// service was not started.
		case ERROR_NETNAME_DELETED: // The specified network name is no longer available.
		case ERROR_NETWORK_ACCESS_DENIED: // Network access is denied.
		case ERROR_NO_BROWSER_SERVERS_FOUND: // The list of servers for this workgroup is not
			// currently available
		case ERROR_NO_LOGON_SERVERS: // There are currently no logon servers available to service the
			// logon request.
		case ERROR_NO_NET_OR_BAD_PATH: // No network provider accepted the given network path.
		case ERROR_NO_NETWORK: // The network is not present or not started.
		case ERROR_NOT_LOGGED_ON: // The operation being requested was not performed because the user
			// has not logged on to the network. The specified service does not exist.
		case ERROR_NOT_SUPPORTED: // The network request is not supported.
		case ERROR_REQ_NOT_ACCEP: // No more connections can be made to this remote computer at this
			// time because there are already as many connections as the computer can accept.
		case ERROR_SHARING_PAUSED: // The remote server has been paused or is in the process of being
			// started.
		case ERROR_TOO_MANY_NAMES: // The name limit for the local computer network adapter card was
			// exceeded.
		case ERROR_TOO_MANY_SESS: // The network BIOS session limit was exceeded.
		case ERROR_UNEXP_NET_ERR: // An unexpected network error occurred.
		case ERROR_WINS_INTERNAL: // WINS encountered an error while processing the command.
			abc_throw(network_error, (err));


		case ERROR_ADAP_HDW_ERR: // A network adapter hardware error occurred.
		case ERROR_ADDRESS_NOT_ASSOCIATED: // An address has not yet been associated with the network
			// endpoint.
		case ERROR_CONNECTION_ABORTED: // The network connection was aborted by the local system.
		case ERROR_CONNECTION_INVALID: // An operation was attempted on a nonexistent network
			// connection.
		case ERROR_CONNECTION_REFUSED: // The remote system refused the network connection.
		case ERROR_DEV_NOT_EXIST: // The specified network resource or device is no longer available.
		case ERROR_NET_WRITE_FAULT: // A write fault occurred on the network.
		case ERROR_NETWORK_BUSY: // The network is busy.
		case ERROR_NETWORK_UNREACHABLE: // The remote network is not reachable by the transport.
		case ERROR_REQUEST_ABORTED: // The request was aborted.
			abc_throw(network_io_error, (err));


		case ERROR_ARITHMETIC_OVERFLOW: // Arithmetic result exceeded 32 bits.
			abc_throw(overflow_error, (err));


		case ERROR_ACCESS_DENIED: // Access is denied.
		case ERROR_ACCOUNT_DISABLED: // Logon failure - account currently disabled.
		case ERROR_ACCOUNT_EXPIRED: // The user's account has expired.
		case ERROR_ACCOUNT_LOCKED_OUT: // The referenced account is currently locked out and may not
			// be logged on to.
		case ERROR_ACCOUNT_RESTRICTION: // Logon failure - user account restriction.
		case ERROR_ALIAS_EXISTS: // The specified local group already exists.
		case ERROR_ALLOTTED_SPACE_EXCEEDED: // No more memory is available for security information
			// updates.
		case ERROR_BAD_IMPERSONATION_LEVEL: // Either a required impersonation level was not provided,
			// or the provided impersonation level is invalid.
		case ERROR_BAD_INHERITANCE_ACL: // The inherited access control list (ACL) or access control
			// entry (ACE) could not be built.
		case ERROR_BAD_LOGON_SESSION_STATE: // The logon session is not in a state that is consistent
			// with the requested operation.
		case ERROR_BAD_TOKEN_TYPE: // The type of the token is inappropriate for its attempted use.
		case ERROR_BAD_USERNAME: // The specified user name is invalid.
		case ERROR_CANT_DISABLE_MANDATORY: // The group cannot be disabled.
		case ERROR_CANT_OPEN_ANONYMOUS: // Cannot open an anonymous level security token.
		case ERROR_DOMAIN_TRUST_INCONSISTENT: // The name or security identifier (SID) of the domain
			// specified is inconsistent with the trust information for that domain.
		case ERROR_EA_ACCESS_DENIED: // Access to the extended attribute was denied.
		case ERROR_GENERIC_NOT_MAPPED: // Generic access types were contained in an access mask that
			// should already be mapped to nongeneric types.
		case ERROR_ILL_FORMED_PASSWORD: // Unable to update the password. The value provided for the
			// new password contains values that are not allowed in passwords.
		case ERROR_INVALID_DOMAIN_ROLE: // This operation is only allowed for the Primary Domain
			// Controller (PDC) of the domain.
		case ERROR_INVALID_DOMAIN_STATE: // The domain was in the wrong state to perform the security
			// operation.
		case ERROR_INVALID_LOGON_HOURS: // Logon failure - account logon time restriction violation.
		case ERROR_INVALID_MEMBER: // A new member could not be added to a local group because the
			// member has the wrong account type.
		case ERROR_INVALID_OWNER: // This security identifier may not be assigned as the owner of this
			// object.
		case ERROR_INVALID_PASSWORD: // The specified network password is not correct.
		case ERROR_INVALID_PRIMARY_GROUP: // This security identifier may not be assigned as the
			// primary group of an object.
		case ERROR_INVALID_SERVER_STATE: // The security account manager (SAM) or local security
			// authority (LSA) server was in the wrong state to perform the security operation.
		case ERROR_INVALID_SERVICE_ACCOUNT: // The account name is invalid or does not exist.
		case ERROR_INVALID_SERVICE_CONTROL: // The requested control is not valid for this service.
		case ERROR_INVALID_SUB_AUTHORITY: // The subauthority part of a security identifier is invalid
			// for this particular use.
		case ERROR_INVALID_WORKSTATION: // Logon failure - user not allowed to log on to this
			// computer.
		case ERROR_LAST_ADMIN: // The last remaining administration account cannot be disabled or
			// deleted.
		case ERROR_LM_CROSS_ENCRYPTION_REQUIRED: // A cross-encrypted password is necessary to change
			// this user password.
		case ERROR_LOCAL_USER_SESSION_KEY: // No encryption key is available. A well-known encryption
			// key was returned.
		case ERROR_LOGIN_TIME_RESTRICTION: // Attempting to log in during an unauthorized time of day
			// for this account.
		case ERROR_LOGIN_WKSTA_RESTRICTION: // The account is not authorized to log in from this
			// station.
		case ERROR_LOGON_FAILURE: // Logon failure - unknown user name or bad password.
		case ERROR_LOGON_NOT_GRANTED: // Logon failure - the user has not been granted the requested
			// logon type at this computer.
		case ERROR_LOGON_SESSION_COLLISION: // The logon session identifier is already in use.
		case ERROR_LOGON_SESSION_EXISTS: // Cannot start a new logon session with an identifier that
			// is already in use.
		case ERROR_LOGON_TYPE_NOT_GRANTED: // Logon failure - the user has not been granted the
			// requested logon type at this computer.
		case ERROR_MEMBER_IN_ALIAS: // The specified account name is already a member of the local
			// group.
		case ERROR_MEMBER_IN_GROUP: // Either the specified user account is already a member of the
			// specified group, or the specified group cannot be deleted because it contains a member.
		case ERROR_MEMBER_NOT_IN_ALIAS: // The specified account name is not a member of the local
			// group.
		case ERROR_MEMBER_NOT_IN_GROUP: // The specified user account is not a member of the specified
			// group account.
		case ERROR_MEMBERS_PRIMARY_GROUP: // The user cannot be removed from a group because the group
			// is currently the user's primary group.
		case ERROR_NO_ASSOCIATION: // No application is associated with the specified file for this
			// operation.
		case ERROR_NO_IMPERSONATION_TOKEN: // An attempt has been made to operate on an impersonation
			// token by a thread that is not currently impersonating a client.
		case ERROR_NO_INHERITANCE: // Indicates an ACL contains no inheritable components.
		case ERROR_NO_QUOTAS_FOR_ACCOUNT: // No system quota limits are specifically set for this
			// account.
		case ERROR_NO_SECURITY_ON_OBJECT: // Unable to perform a security operation on an object that
			// has no associated security.
		case ERROR_NO_SUCH_ALIAS: // The specified local group does not exist.
		case ERROR_NO_SUCH_DOMAIN: // The specified domain did not exist.
		case ERROR_NO_SUCH_GROUP: // The specified group does not exist.
		case ERROR_NO_SUCH_LOGON_SESSION: // A specified logon session does not exist. It may already
			// have been terminated.
		case ERROR_NO_SUCH_MEMBER: // A new member could not be added to a local group because the
			// member does not exist.
		case ERROR_NO_SUCH_PACKAGE: // A specified authentication package is unknown.
		case ERROR_NO_SUCH_PRIVILEGE: // A specified privilege does not exist.
		case ERROR_NO_SUCH_USER: // The specified user does not exist.
		case ERROR_NO_TOKEN: // An attempt was made to reference a token that does not exist.
		case ERROR_NO_TRUST_LSA_SECRET: // The workstation does not have a trust secret.
		case ERROR_NO_TRUST_SAM_ACCOUNT: // The Security access Model (SAM) database on the Windows NT
			// Server does not have a computer account for this workstation trust relationship.
		case ERROR_NO_USER_SESSION_KEY: // There is no user session key for the specified logon
			// session.
		case ERROR_NOT_ALL_ASSIGNED: // Not all privileges referenced are assigned to the caller.
		case ERROR_NOT_AUTHENTICATED: // The operation being requested was not performed because the
			// user has not been authenticated.
		case ERROR_NOT_LOGON_PROCESS: // The requested action is restricted for use by logon processes
			// only. The calling process has not registered as a logon process.
		case ERROR_NT_CROSS_ENCRYPTION_REQUIRED: // A cross-encrypted password is necessary to change
			// a user password.
		case ERROR_NULL_LM_PASSWORD: // The password is too complex to be converted to a LAN Manager
			// password. The LAN Manager password returned is a null string.
		case ERROR_PASSWORD_EXPIRED: // Logon failure - the specified account password has expired.
		case ERROR_PASSWORD_MUST_CHANGE: // The user must change his password before he logs on the
			// first time.
		case ERROR_PASSWORD_RESTRICTION: // Unable to update the password because a password update
			// rule has been violated.
		case ERROR_SHARING_VIOLATION: // The process cannot access the file because it is being used
			// by another process.
		case ERROR_SCREEN_ALREADY_LOCKED: // Screen already locked.
		case ERROR_SPECIAL_ACCOUNT: // Cannot perform this operation on built-in accounts.
		case ERROR_SPECIAL_GROUP: // Cannot perform this operation on this built-in special group.
		case ERROR_SPECIAL_USER: // Cannot perform this operation on this built-in special user.
		case ERROR_TOKEN_ALREADY_IN_USE: // The token is already in use as a primary token.
		case ERROR_TRUST_FAILURE: // The network logon failed.
		case ERROR_TRUSTED_DOMAIN_FAILURE: // The trust relationship between the primary domain and
			// the trusted domain failed.
		case ERROR_TRUSTED_RELATIONSHIP_FAILURE: // The trust relationship between this workstation
			// and the primary domain failed.
		case ERROR_USER_EXISTS: // The specified user already exists.
		case ERROR_VC_DISCONNECTED: // The session was canceled.
		case ERROR_WRONG_PASSWORD: // Unable to update the password. The value provided as the current
			// password is incorrect.
			abc_throw(security_error, (err));


		case ERROR_NO_UNICODE_TRANSLATION: // No mapping for the Unicode character exists in the
			// target multibyte code page.
			abc_throw(text_encode_error, (err));
	}
}

#endif


// Default translations between exception class to OS-specific error code.

#if ABC_HOST_API_POSIX
	ABC_MAP_ERROR_CLASS_TO_ERRINT(argument_error, EINVAL);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(domain_error, EDOM);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(file_not_found_error, ENOENT);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(io_error, EIO);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_address_error, EFAULT);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(overflow_error, EOVERFLOW);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(null_pointer_error, EFAULT);
#elif ABC_HOST_API_WIN32
	ABC_MAP_ERROR_CLASS_TO_ERRINT(file_not_found_error, ERROR_PATH_NOT_FOUND);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(invalid_path_error, ERROR_BAD_PATHNAME);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_address_error, ERROR_INVALID_ADDRESS);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(memory_allocation_error, ERROR_NOT_ENOUGH_MEMORY);
	ABC_MAP_ERROR_CLASS_TO_ERRINT(null_pointer_error, ERROR_INVALID_ADDRESS);
#endif

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::exception


namespace abc {

exception::exception() :
	m_pszWhat("abc::exception"),
	m_pszSourceFunction(NULL),
	m_pszSourceFileName(NULL),
	m_iSourceLine(0),
	m_bInFlight(false) {
}
exception::exception(exception const & x) :
	m_pszWhat(x.m_pszWhat),
	m_pszSourceFunction(x.m_pszSourceFunction),
	m_pszSourceFileName(x.m_pszSourceFileName),
	m_iSourceLine(x.m_iSourceLine),
	m_bInFlight(x.m_bInFlight) {
	// See [DOC:8503 Stack tracing].
	if (m_bInFlight) {
		_scope_trace_impl::trace_stream_addref();
	}
}


/*virtual*/ exception::~exception() {
	// See [DOC:8503 Stack tracing].
	if (m_bInFlight) {
		_scope_trace_impl::trace_stream_release();
	}
}


exception & exception::operator=(exception const & x) {
	abc_trace_fn((this/*, x*/));

	m_pszWhat = x.m_pszWhat;
	m_pszSourceFunction = x.m_pszSourceFunction;
	m_pszSourceFileName = x.m_pszSourceFileName;
	m_iSourceLine = x.m_iSourceLine;
	// Adopt the source’s in-flight status. See [DOC:8503 Stack tracing].
	// If the in-flight status is not changing, avoid the pointless (and dangerous, if done in this
	// sequence - it could delete the trace stream if *this was the last reference to it)
	// release()/addref().
	if (m_bInFlight != x.m_bInFlight) {
		if (m_bInFlight) {
			_scope_trace_impl::trace_stream_release();
		}
		m_bInFlight = x.m_bInFlight;
		if (m_bInFlight) {
			_scope_trace_impl::trace_stream_addref();
		}
	}
	return *this;
}


void exception::_before_throw(char const * pszFileName, uint16_t iLine, char const * pszFunction) {
	m_pszSourceFunction = pszFunction;
	m_pszSourceFileName = pszFileName;
	m_iSourceLine = iLine;
	// Clear any old trace stream buffer and create a new one with *this as its only reference. See
	// [DOC:8503 Stack tracing].
	_scope_trace_impl::trace_stream_reset();
	_scope_trace_impl::trace_stream_addref();
	m_bInFlight = true;
}


char const * exception::what() const {
	return m_pszWhat;
}


/*static*/ void exception::write_with_scope_trace(
	ostream * pos /*= NULL*/, std::exception const * pstdx /*= NULL*/
) {
	if (!pos) {
		pos = file_ostream::stderr().get();
	}
	exception const * pabcx;
	if (pstdx) {
		// We have a std::exception: print its what() and check if it’s also a abc::exception.
		pos->print(SL("Unhandled exception: {}\n"), pstdx->what());
		pabcx = dynamic_cast<exception const *>(pstdx);
		// If the virtual method _print_extended_info() is not the default one provided by
		// abc::exception, the class has a custom implementation, probably to print something useful.
		if (pabcx /*&& pabcx->_print_extended_info != exception::_print_extended_info*/) {
			try {
				pos->write(SL("Extended information:\n"));
				pabcx->_print_extended_info(pos);
			} catch (...) {
				// The exception is not rethrown, because we don’t want exception details to interfere
				// with the display of the (more important) exception information.
				// FIXME: EXC-SWALLOW
			}
		}
	} else {
		// Some other type of exception; not much to say.
		pabcx = NULL;
		pos->write(SL("Unhandled exception: (unknown type)\n"));
	}

	pos->write(SL("Stack trace (most recent call first):\n"));
	if (pabcx) {
		// Frame 0 is the location of the abc_throw() statement.
		pos->print(
			SL("#0 {} at {}:{}\n"),
			pabcx->m_pszSourceFunction, pabcx->m_pszSourceFileName, pabcx->m_iSourceLine
		);
	}
	// Print the stack trace collected via abc_trace_fn().
	pos->write(_scope_trace_impl::get_trace_contents());
}


void exception::_print_extended_info(ostream * pos) const {
	// Nothing to print.
	UNUSED_ARG(pos);
}


#if ABC_HOST_API_LINUX

// These should be member variables of exception::async_handler_manager.

/** Signals that we can translate into C++ exceptions. */
static int const g_aiHandledSignals[] = {
// Signal (Default action) Description (standard).
//	SIGABRT, // (Core) Abort signal from abort(3) (POSIX.1-1990).
//	SIGALRM, // (Term) Timer signal from alarm(2) (POSIX.1-1990).
	SIGBUS,  // (Core) Bus error (bad memory access) (POSIX.1-2001).
//	SIGCHLD, // (Ign ) Child stopped or terminated (POSIX.1-1990).
//	SIGCONT, // (Cont) Continue if stopped (POSIX.1-1990).
	SIGFPE,  // (Core) Floating point exception (POSIX.1-1990).
//	SIGHUP,  // (Term) Hangup on controlling terminal or death of controlling process (POSIX.1-1990).
//	SIGILL,  // (Core) Illegal Instruction (POSIX.1-1990).
//	SIGINT,  // (Term) Interrupt from keyboard (POSIX.1-1990).
//	SIGPIPE, // (Term) Broken pipe: write to pipe with no readers (POSIX.1-1990).
//	SIGPROF, // (Term) Profiling timer expired (POSIX.1-2001).
//	SIGQUIT, // (Core) Quit from keyboard (POSIX.1-1990).
	SIGSEGV  // (Core) Invalid memory reference (POSIX.1-1990).
//	SIGTERM  // (Term) Termination signal (POSIX.1-1990).
//	SIGTRAP  // (Core) Trace/breakpoint trap (POSIX.1-2001).
//	SIGTSTP  // (Stop) Stop typed at terminal (POSIX.1-1990).
//	SIGTTIN  // (Stop) Terminal input for background process (POSIX.1-1990).
//	SIGTTOU  // (Stop) Terminal output for background process (POSIX.1-1990).
//	SIGUSR1  // (Term) User-defined signal 1 (POSIX.1-1990).
//	SIGUSR2  // (Term) User-defined signal 2 (POSIX.1-1990).
};
/** Default handler for each of the signals above. */
static struct ::sigaction g_asaDefault[countof(g_aiHandledSignals)];


/** Translates POSIX signals into C++ exceptions, whenever possible.
*/
static void eahm_sigaction(int iSignal, ::siginfo_t * psi, void * pctx) {
	abc_trace_fn((iSignal, psi, pctx));

	// Don’t let external programs mess with us: if the source is not the kernel, ignore the error.
	// POSIX.1-2008 states that:
	//    “Historically, an si_code value of less than or equal to zero indicated that the signal was
	//    generated by a process via the kill() function, and values of si_code that provided
	//    additional information for implementation-generated signals, such as SIGFPE or SIGSEGV,
	//    were all positive. […] if si_code is less than or equal to zero, the signal was generated
	//    by a process. However, since POSIX.1b did not specify that SI_USER (or SI_QUEUE) had a
	//    value less than or equal to zero, it is not true that when the signal is generated by a
	//    process, the value of si_code will always be less than or equal to zero. XSI applications
	//    should check whether si_code is SI_USER or SI_QUEUE in addition to checking whether it is
	//    less than or equal to zero.”
	// So we do exactly that - except we skip checking for SI_USER and SI_QUEUE at this point because
	// they don’t apply to many signals this handler takes care of.
	if (psi->si_code <= 0) {
		return;
	}

	switch (iSignal) {
		case SIGBUS:
			// TODO: this is the only way we can test SIGBUS on x86, otherwise the program will get
			// stuck in an endless memory-allocating loop. How can this be made to only execute when
			// testing that one test?

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
#endif

			// There aren’t many codes here that are safe to handle; most of them indicate that there
			// is some major memory corruption going on, and in that case we really don’t want to keep
			// on going - even the code to throw an exception could be compromised.
			switch (psi->si_code) {
				case BUS_ADRALN: // Invalid address alignment.
					abc_throw(abc::memory_access_error, (psi->si_addr));
			}
			break;

		case SIGFPE:
			switch (psi->si_code) {
				case FPE_INTDIV: // Integer divide by zero.
					abc_throw(abc::division_by_zero_error, ());

				case FPE_INTOVF: // Integer overflow.
					abc_throw(abc::overflow_error, ());

				case FPE_FLTDIV: // Floating-point divide by zero.
				case FPE_FLTOVF: // Floating-point overflow.
				case FPE_FLTUND: // Floating-point underflow.
				case FPE_FLTRES: // Floating-point inexact result.
				case FPE_FLTINV: // Floating-point invalid operation.
				case FPE_FLTSUB: // Subscript out of range.
					abc_throw(abc::floating_point_error, ());
			}
			// At the time of writing, the above case labels don’t leave out any values, but that’s not
			// necessarily going to be true in 5 years, so…
			abc_throw(abc::arithmetic_error, ());

		case SIGSEGV:
			if (psi->si_addr == NULL) {
				abc_throw(abc::null_pointer_error, ());
			} else {
				abc_throw(abc::memory_address_error, (psi->si_addr));
			}
	}
	// Handle all unrecognized cases here. Since here we only handle signals for which the default
	// actions is a core dump, calling abort (which sends SIGABRT, also causing a core dump) is the
	// same as invoking the default action.
	::abort();
}


exception::async_handler_manager::async_handler_manager() {
	struct ::sigaction saNew;
	saNew.sa_sigaction = eahm_sigaction;
	::sigemptyset(&saNew.sa_mask);
	// Without SA_NODEFER (POSIX.1-2001), the handler would be disabled during its own execution,
	// only to be restored when the handler returns. Since we’ll throw a C++ exception from within
	// the handler, the restoration would be skipped, and if the signal were raised again, we’d just
	// crash.
	// SA_SIGINFO (POSIX.1-2001) provides the handler with more information about the signal, which
	// we use to generate better exceptions.
	saNew.sa_flags = SA_NODEFER | SA_SIGINFO;

	// Setup handlers for the signals in g_aiHandledSignals.
	for (int i(countof(g_aiHandledSignals)); --i >= 0; ) {
		::sigaction(g_aiHandledSignals[i], &saNew, &g_asaDefault[i]);
	}
}


exception::async_handler_manager::~async_handler_manager() {
	// Restore the saved signal handlers.
	for (int i(countof(g_aiHandledSignals)); --i >= 0; ) {
		::sigaction(g_aiHandledSignals[i], &g_asaDefault[i], NULL);
	}
}

#elif ABC_HOST_API_WIN32 //if ABC_HOST_API_LINUX

exception::async_handler_manager::async_handler_manager() {
	// TODO: implementation.
}


exception::async_handler_manager::~async_handler_manager() {
	// TODO: implementation.
}

#endif //if ABC_HOST_API_LINUX … elif ABC_HOST_API_WIN32

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::argument_error


namespace abc {

argument_error::argument_error() :
	generic_error() {
	m_pszWhat = "abc::argument_error";
}


void argument_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<argument_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::arithmetic_error


namespace abc {

arithmetic_error::arithmetic_error() :
	generic_error() {
	m_pszWhat = "abc::arithmetic_error";
}


void arithmetic_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<arithmetic_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::assertion_error


namespace abc {

//TODO: tls
/*tls*/ bool assertion_error::sm_bReentering(false);


/*static*/ void assertion_error::_assertion_failed(
	char const * pszFileName, unsigned iLine, char const * pszFunction, char const * pszExpr
) {
	if (!sm_bReentering) {
		sm_bReentering = true;
		try {
			std::shared_ptr<file_ostream> pfosStdErr(file_ostream::stderr());
			pfosStdErr->print(
				SL("Assertion failed: ( {} ) in file {}:{}: in function {}\n"),
				pszExpr, pszFileName, iLine, pszFunction
			);
		} catch (...) {
			sm_bReentering = false;
			throw;
		}
		sm_bReentering = false;
	}
	abc_throw(assertion_error, ());
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::division_by_zero_error


namespace abc {

division_by_zero_error::division_by_zero_error() :
	arithmetic_error() {
	m_pszWhat = "abc::division_by_zero_error";
}


void division_by_zero_error::init(errint_t err /*= 0*/) {
	arithmetic_error::init(err ? err : os_error_mapping<division_by_zero_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::domain_error


namespace abc {

domain_error::domain_error() :
	generic_error() {
	m_pszWhat = "abc::domain_error";
}

void domain_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<domain_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::floating_point_error


namespace abc {

floating_point_error::floating_point_error() :
	arithmetic_error() {
	m_pszWhat = "abc::floating_point_error";
}


void floating_point_error::init(errint_t err /*= 0*/) {
	arithmetic_error::init(err ? err : os_error_mapping<floating_point_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::generic_error


namespace abc {

generic_error::generic_error() :
	exception() {
	m_pszWhat = "abc::generic_error";
}
generic_error::generic_error(generic_error const & x) :
	exception(x),
	m_err(x.m_err) {
}


generic_error & generic_error::operator=(generic_error const & x) {
	abc_trace_fn((this/*, x*/));

	exception::operator=(x);
	m_err = x.m_err;
	return *this;
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::environment_error


namespace abc {

environment_error::environment_error() :
	generic_error() {
	m_pszWhat = "abc::environment_error";
}


void environment_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<environment_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::file_not_found_error


namespace abc {

file_not_found_error::file_not_found_error() :
	environment_error() {
	m_pszWhat = "abc::file_not_found_error";
}


void file_not_found_error::init(errint_t err /*= 0*/) {
	environment_error::init(err ? err : os_error_mapping<file_not_found_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::index_error


namespace abc {

index_error::index_error() :
	lookup_error() {
	m_pszWhat = "abc::index_error";
}
index_error::index_error(index_error const & x) :
	generic_error(x),
	lookup_error(x),
	m_iInvalid(x.m_iInvalid) {
}


index_error & index_error::operator=(index_error const & x) {
	abc_trace_fn((this/*, x*/));

	lookup_error::operator=(x);
	m_iInvalid = x.m_iInvalid;
	return *this;
}


void index_error::init(intptr_t iInvalid, errint_t err /*= 0*/) {
	lookup_error::init(err ? err : os_error_mapping<index_error>::mapped_error);
	m_iInvalid = iInvalid;
}


void index_error::_print_extended_info(ostream * pos) const {
	pos->print(SL("invalid index: {}\n"), m_iInvalid);
	lookup_error::_print_extended_info(pos);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::invalid_path_error


namespace abc {

invalid_path_error::invalid_path_error() :
	generic_error() {
	m_pszWhat = "abc::invalid_path_error";
}


void invalid_path_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<invalid_path_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::io_error


namespace abc {

io_error::io_error() :
	environment_error() {
	m_pszWhat = "abc::io_error";
}


void io_error::init(errint_t err /*= 0*/) {
	environment_error::init(err ? err : os_error_mapping<io_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::lookup_error


namespace abc {

lookup_error::lookup_error() :
	generic_error() {
	m_pszWhat = "abc::lookup_error";
}


void lookup_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<lookup_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_access_error


namespace abc {

memory_access_error::memory_access_error() :
	memory_address_error() {
	m_pszWhat = "abc::memory_access_error";
}


void memory_access_error::init(void const * pInvalid, errint_t err /*= 0*/) {
	memory_address_error::init(
		pInvalid, err ? err : os_error_mapping<memory_access_error>::mapped_error
	);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_address_error


namespace abc {

char_t const memory_address_error::smc_achUnknownAddress[] = SL("unknown memory address");


memory_address_error::memory_address_error() :
	generic_error() {
	m_pszWhat = "abc::memory_address_error";
}
memory_address_error::memory_address_error(memory_address_error const & x) :
	generic_error(x),
	m_pInvalid(x.m_pInvalid) {
}


memory_address_error & memory_address_error::operator=(memory_address_error const & x) {
	abc_trace_fn((this/*, x*/));

	generic_error::operator=(x);
	m_pInvalid = x.m_pInvalid;
	return *this;
}


void memory_address_error::init(void const * pInvalid, errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<memory_address_error>::mapped_error);
	m_pInvalid = pInvalid;
}


void memory_address_error::_print_extended_info(ostream * pos) const {
	if (m_pInvalid != smc_achUnknownAddress) {
		pos->print(SL("invalid address: {}\n"), m_pInvalid);
	} else {
		pos->write(smc_achUnknownAddress);
	}
	generic_error::_print_extended_info(pos);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::memory_allocation_error


namespace abc {

memory_allocation_error::memory_allocation_error() :
	generic_error() {
	m_pszWhat = "abc::memory_allocation_error";
}


void memory_allocation_error::init(errint_t err /*= 0*/) {
	generic_error::init(err ? err : os_error_mapping<memory_allocation_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::null_pointer_error


namespace abc {

null_pointer_error::null_pointer_error() :
	memory_address_error() {
	m_pszWhat = "abc::null_pointer_error";
}


void null_pointer_error::init(errint_t err /*= 0*/) {
	memory_address_error::init(NULL, err ? err : os_error_mapping<null_pointer_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::overflow_error


namespace abc {

overflow_error::overflow_error() :
	arithmetic_error() {
	m_pszWhat = "abc::overflow_error";
}


void overflow_error::init(errint_t err /*= 0*/) {
	arithmetic_error::init(err ? err : os_error_mapping<overflow_error>::mapped_error);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////
// abc::syntax_error


namespace abc {

syntax_error::syntax_error() :
	generic_error() {
	m_pszWhat = "abc::syntax_error";
}
syntax_error::syntax_error(syntax_error const & x) :
	generic_error(x),
	m_crDescription(x.m_crDescription),
	m_crSource(x.m_crSource),
	m_iChar(x.m_iChar),
	m_iLine(x.m_iLine) {
}


syntax_error & syntax_error::operator=(syntax_error const & x) {
	abc_trace_fn((this/*, x*/));

	generic_error::operator=(x);
	m_crDescription = x.m_crDescription;
	m_crSource = x.m_crSource;
	m_iChar = x.m_iChar;
	m_iLine = x.m_iLine;
	return *this;
}


void syntax_error::init(
	char_range const & crDescription /*= char_range()*/, 
	char_range const & crSource /*= char_range()*/, unsigned iChar /*= 0*/, unsigned iLine /*= 0*/,
	errint_t err /*= 0*/
) {
	generic_error::init(err ? err : os_error_mapping<syntax_error>::mapped_error);
	m_crDescription = crDescription;
	m_crSource = crSource;
	m_iChar = iChar;
	m_iLine = iLine;
}


void syntax_error::_print_extended_info(ostream * pos) const {
	istr sFormat;
	if (m_crSource) {
		if (m_iChar) {
			if (m_iLine) {
				sFormat = SL("{0} in {1}:{2}:{3}\n");
			} else {
				sFormat = SL("{0} in expression \"{1}\", character {3}\n");
			}
		} else {
			if (m_iLine) {
				sFormat = SL("{0} in {1}:{2}\n");
			} else {
				sFormat = SL("{0} in expression \"{1}\"\n");
			}
		}
	} else {
		if (m_iChar) {
			if (m_iLine) {
				sFormat = SL("{0} in <input>:{2}:{3}\n");
			} else {
				sFormat = SL("{0} in <expression>, character {3}\n");
			}
		} else {
			if (m_iLine) {
				sFormat = SL("{0} in <input>:{2}\n");
			} else {
				sFormat = SL("{0}\n");
			}
		}
	}

	pos->print(sFormat, m_crDescription, m_crSource, m_iLine, m_iChar);
	generic_error::_print_extended_info(pos);
}

} //namespace abc


////////////////////////////////////////////////////////////////////////////////////////////////////


/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "net_exception.h"

#ifdef _WINDOWS

#include <Winsock2.h>

using namespace mrt;

NetException::NetException(const int code) {
	const char * error;
	switch(code) {
		case WSAEINTR: 
			error = "Interrupted function call"; break;
		case WSAEACCES: 
			error = "Permission denied"; break;
		case WSAEFAULT: 
			error = "Bad address"; break;
		case WSAEINVAL: 
			error = "Invalid argument"; break;
		case WSAEMFILE: 
			error = "Too many open files"; break;
		case WSAEWOULDBLOCK: 
			error = "Resource temporarily unavailable"; break;
		case WSAEINPROGRESS: 
			error = "Operation now in progress"; break;
		case WSAEALREADY: 
			error = "Operation already in progress"; break;
		case WSAENOTSOCK: 
			error = "Socket operation on nonsocket"; break;
		case WSAEDESTADDRREQ: 
			error = "Destination address required"; break;
		case WSAEMSGSIZE: 
			error = "Message too long"; break;
		case WSAEPROTOTYPE: 
			error = "Protocol wrong type for socket"; break;
		case WSAENOPROTOOPT: 
			error = "Bad protocol option"; break;
		case WSAEPROTONOSUPPORT: 
			error = "Protocol not supported"; break;
		case WSAESOCKTNOSUPPORT: 
			error = "Socket type not supported"; break;
		case WSAEOPNOTSUPP: 
			error = "Operation not supported"; break;
		case WSAEPFNOSUPPORT: 
			error = "Protocol family not supported"; break;
		case WSAEAFNOSUPPORT: 
			error = "Address family not supported by protocol family"; break;
		case WSAEADDRINUSE: 
			error = "Address already in use"; break;
		case WSAEADDRNOTAVAIL: 
			error = "Cannot assign requested address"; break;
		case WSAENETDOWN: 
			error = "Network is down"; break;
		case WSAENETUNREACH: 
			error = "Network is unreachable"; break;
		case WSAENETRESET: 
			error = "Network dropped connection on reset"; break;
		case WSAECONNABORTED: 
			error = "Software caused connection abort"; break;
		case WSAECONNRESET: 
			error = "Connection reset by peer"; break;
		case WSAENOBUFS: 
			error = "No buffer space available"; break;
		case WSAEISCONN: 
			error = "Socket is already connected"; break;
		case WSAENOTCONN: 
			error = "Socket is not connected"; break;
		case WSAESHUTDOWN: 
			error = "Cannot send after socket shutdown"; break;
		case WSAETIMEDOUT: 
			error = "Connection timed out"; break;
		case WSAECONNREFUSED: 
			error = "Connection refused"; break;
		case WSAEHOSTDOWN: 
			error = "Host is down"; break;
		case WSAEHOSTUNREACH: 
			error = "No route to host"; break;
		case WSAEPROCLIM: 
			error = "Too many processes"; break;
		case WSASYSNOTREADY: 
			error = "Network subsystem is unavailable"; break;
		case WSAVERNOTSUPPORTED: 
			error = "Winsock.dll version out of range"; break;
		case WSANOTINITIALISED: 
			error = "Successful WSAStartup not yet performed"; break;
		case WSAEDISCON: 
			error = "Graceful shutdown in progress"; break;
		case WSATYPE_NOT_FOUND: 
			error = "Class type not found"; break;
		case WSAHOST_NOT_FOUND: 
			error = "Host not found"; break;
		case WSATRY_AGAIN: 
			error = "Nonauthoritative host not found"; break;
		case WSANO_RECOVERY: 
			error = "This is a nonrecoverable error"; break;
		case WSANO_DATA: 
			error = "Valid name, no data record of requested type"; break;
		case WSA_INVALID_HANDLE: 
			error = "Specified event object handle is invalid"; break;
		case WSA_INVALID_PARAMETER: 
			error = "One or more parameters are invalid"; break;
		case WSA_IO_INCOMPLETE: 
			error = "WSA_IO_INCOMPLETE"; break;
		case WSA_IO_PENDING: 
			error = "Overlapped operations will complete later"; break;
		case WSA_NOT_ENOUGH_MEMORY: 
			error = "Insufficient memory available"; break;
		case WSA_OPERATION_ABORTED: 
			error = "WSA_OPERATION_ABORTED"; break;
/*		case WSAINVALIDPROCTABLE: 
			error = "Invalid procedure table from service provider"; break;
		case WSAINVALIDPROVIDER: 
			error = "Invalid service provider version number"; break;
		case WSAPROVIDERFAILEDINIT: 
			error = "Unable to initialize a service provider"; break;
		case WSASYSCALLFAILURE: 
			error = "System call failure"; break;
*/
		default:
			error = "Unknown error";
	}

	wsa_error = mrt::formatString("error %d: %s", code, error);
}

NetException::~NetException() {}

const std::string NetException::getCustomMessage() {
	return wsa_error;
}

#endif

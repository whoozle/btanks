#ifndef MRT_NETEXCEPTION_H__
#define MRT_NETEXCEPTION_H__

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

#ifdef _WINDOWS
#	include "exception.h"

namespace mrt {

DERIVE_EXCEPTION_NO_DEFAULT(MRTAPI, NetException, (const int wsacode), std::string wsa_error; );

}

#define throw_net(str) throw_generic_no_default(mrt::NetException, str, (WSAGetLastError()))


#else
#	include "ioexception.h"
#	define throw_net throw_io
#endif

#endif


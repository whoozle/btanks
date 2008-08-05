#ifndef __MRT_SOCKET_SET_H__
#define __MRT_SOCKET_SET_H__

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

#include "export_mrt.h"
#ifdef __APPLE__
#  undef check
#endif

namespace mrt {

class Socket;

class MRTAPI SocketSet {
public: 
	static const int Read;
	static const int Write;
	static const int Exception;

	SocketSet();
	void add(const Socket &sock, const int how = Read | Write | Exception);
	void add(const Socket *sock, const int how = Read | Write | Exception);
	void remove(const Socket &sock);
	
	const int check(const unsigned int timeout);
	const bool check(const Socket &sock, const int how);
	const bool check(const Socket *sock, const int how);
	
	void reset();
	
	~SocketSet();
protected: 
	void * _r_set, *_w_set, *_e_set;
	int _n;
private: 
	SocketSet(const SocketSet &);
	const SocketSet & operator=(const SocketSet &);

};
}

#endif


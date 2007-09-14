/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "socket_set.h"
#include "tcp_socket.h"
#include "ioexception.h"
#include <assert.h>

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <sys/select.h>
/* According to earlier standards */
#	include <sys/time.h>
#	include <sys/types.h>
#	include <unistd.h>
#endif

using namespace mrt;

const int SocketSet::Read = 1;
const int SocketSet::Write = 2;
const int SocketSet::Exception = 4;


SocketSet::SocketSet() : _r_set(new fd_set), _w_set(new fd_set), _e_set(new fd_set), _n(0) {
	reset();
}

void SocketSet::reset() {
	FD_ZERO((fd_set*)_r_set);
	FD_ZERO((fd_set*)_w_set);
	FD_ZERO((fd_set*)_e_set);
}


void SocketSet::add(const Socket &sock, const int how) {
	int fd = sock._sock;
	if (fd == -1)
		throw_ex(("attempt to add uninitialized socket to set"));
	if (!(how & (Read | Write | Exception))) {
		LOG_WARN(("skip add in set %d", how));
		return;
	}

	if (how & Read) {
		FD_SET(fd, (fd_set*)_r_set);
	}
	if (how & Write) {
		FD_SET(fd, (fd_set*)_w_set);
	}
	if (how & Exception) {
		FD_SET(fd, (fd_set*)_e_set);
	}
	if (fd >= _n) 
		_n = fd + 1;
}

void SocketSet::add(const Socket *sock, const int how) {
	if (sock == NULL)
		throw_ex(("attempt to add NULL socket to set"));
	add(*sock, how);
}


void SocketSet::remove(const Socket &sock) {
	if (sock._sock == -1)
		throw_ex(("attempt to remove uninitialized socket from set"));

	FD_CLR(sock._sock, (fd_set*)_r_set);
	FD_CLR(sock._sock, (fd_set*)_w_set);
	FD_CLR(sock._sock, (fd_set*)_e_set);
}

const int SocketSet::check(const unsigned int timeout) {
	struct timeval tv;
	tv.tv_sec = timeout / 1000;
	tv.tv_usec = ( timeout % 1000 ) * 1000;
	
	int r = select(_n, (fd_set*)_r_set, (fd_set*)_w_set, (fd_set*)_e_set, &tv);
	if (r == -1)
		throw_io(("select"));
	
	return r;
}

const bool SocketSet::check(const Socket &sock, const int how) {
	int fd = sock._sock;
	if (fd == -1)
		throw_ex(("check on uninitialized socket"));
	
	if ((how & Read) != 0 && FD_ISSET(fd, ((fd_set*)_r_set)))
		return true;
	if ((how & Write) != 0 && FD_ISSET(fd, ((fd_set*)_w_set)))
		return true;
	if ((how & Exception) != 0 && FD_ISSET(fd, ((fd_set*)_e_set)))
		return true;
	return false;
}

const bool SocketSet::check(const Socket *sock, const int how) {
	return check(*sock, how);
}

SocketSet::~SocketSet() {
	delete ((fd_set *)_r_set);
	delete ((fd_set *)_w_set);
	delete ((fd_set *)_e_set);
}


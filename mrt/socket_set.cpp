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

SocketSet::SocketSet() : _n(1) {
	_r_set = new fd_set;
	_w_set = new fd_set;
	_e_set = new fd_set;
}

void SocketSet::add(const TCPSocket &sock) {
	if (sock._sock == -1)
		throw_ex(("attempt to add uninitialized socket to set"));

	FD_SET(sock._sock, (fd_set*)_r_set);
	FD_SET(sock._sock, (fd_set*)_w_set);
	FD_SET(sock._sock, (fd_set*)_e_set);
	++_n;
}

void SocketSet::add(const TCPSocket *sock) {
	if (sock == NULL)
		throw_ex(("attempt to add NULL socket to set"));
	add(*sock);
}


void SocketSet::remove(const TCPSocket &sock) {
	if (sock._sock == -1)
		throw_ex(("attempt to remove uninitialized socket from set"));

	FD_CLR(sock._sock, (fd_set*)_r_set);
	FD_CLR(sock._sock, (fd_set*)_w_set);
	FD_CLR(sock._sock, (fd_set*)_e_set);
	--_n;
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

const bool SocketSet::check(const TCPSocket &sock, tState how) {
	fd_set *set = NULL;
	switch(how) {
	case Read:
		set = (fd_set*)_r_set;
	case Write:
		set = (fd_set*)_w_set;
	case Exception:
		set = (fd_set*)_e_set;
	}
	assert(set != NULL);
	return FD_ISSET(sock._sock, set) != 0;
}

const bool SocketSet::check(const TCPSocket *sock, tState how) {
	return check(*sock, how);
}

SocketSet::~SocketSet() {
	delete ((fd_set *)_r_set);
	delete ((fd_set *)_w_set);
	delete ((fd_set *)_e_set);
}


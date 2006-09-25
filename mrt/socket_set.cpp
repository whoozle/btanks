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

SocketSet::SocketSet() : _n(0) {
	_r_set = new fd_set;
	_w_set = new fd_set;
	_e_set = new fd_set;
	reset();
}

void SocketSet::reset() {
	FD_ZERO((fd_set*)_r_set);
	FD_ZERO((fd_set*)_w_set);
	FD_ZERO((fd_set*)_e_set);
}


void SocketSet::add(const TCPSocket &sock) {
	int fd = sock._sock;
	if (fd == -1)
		throw_ex(("attempt to add uninitialized socket to set"));
	
	FD_SET(fd, (fd_set*)_r_set);
	FD_SET(fd, (fd_set*)_w_set);
	FD_SET(fd, (fd_set*)_e_set);
	if (fd > _n) 
		_n = fd + 1;
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

const bool SocketSet::check(const TCPSocket &sock, const int how) {
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

const bool SocketSet::check(const TCPSocket *sock, const int how) {
	return check(*sock, how);
}

SocketSet::~SocketSet() {
	delete ((fd_set *)_r_set);
	delete ((fd_set *)_w_set);
	delete ((fd_set *)_e_set);
}


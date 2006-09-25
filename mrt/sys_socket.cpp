#include "sys_socket.h"
#include "ioexception.h"

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include "sys/socket.h"
#endif


using namespace mrt;

Socket::Socket() : _sock(-1) {}

void Socket::init() {
	static bool inited;
	if (inited)
		return;
	
#ifdef WIN32
	struct WSAData WSAData;
	if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
		throw_ex(("failed to initialize winsock 1.1. upgrade your windows installation and/or upgrade mrt runtime"));
#endif
	inited = true;
}

void Socket::create(const int af, int type, int protocol) {
	init();
	close();
	
	_sock = socket(af, type, protocol);
	if (_sock == -1) 
		throw_io(("socket"));
}

void Socket::close() {
	if (_sock == -1)
		return;
	
	shutdown(_sock, 2);
#ifdef WIN32
	::closesocket(_sock);
#else
	::close(_sock);
#endif
	_sock = -1;
}

Socket::~Socket() {
	close();
}

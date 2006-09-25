#include "sys_socket.h"
#include "ioexception.h"

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include "sys/socket.h"
#endif


using namespace mrt;

void Socket::init() {
#ifdef WIN32
if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
	throw_ex(("failed to initialize winsock 1.1. upgrade your windows installation and/or upgrade mrt runtime"));
#endif
}

const int Socket::create(const int af, int type, int protocol) {
	int s = socket(af, type, protocol);
	if (s == -1) 
		throw_io(("socket"));
	return s;
}


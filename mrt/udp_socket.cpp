#include "udp_socket.h"

using namespace mrt;

void UDPSocket::listen(const unsigned port, const bool reuse) {
	
}

const int UDPSocket::send(const Socket::addr &addr, const void *data, const int len) const {
	return -1;
}

const int UDPSocket::recv(Socket::addr &addr, void *data, const int len) const {
	return -1;
}

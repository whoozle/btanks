#include "net_exception.h"

#ifdef _WINDOWS

#include <Winsock2.h>

using namespace mrt;

NetException::NetException(const int code) {

	const char * error;
	switch(code) {
		case WSAEINTR: 
			error = "Interrupted function call."; break;
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

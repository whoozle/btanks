#include "exception.h"
#include <stdarg.h>

using namespace mrt;

Exception::Exception() {}

void Exception::addMessage(const char * file, const int line) {
	char buf[256];
	sprintf(buf, "[%s:%d]", file, line);
	_error = buf;	
}

void Exception::addMessage(const std::string &msg) {
	if (msg.size() == 0) return;
	
	_error += ": " + msg;
}

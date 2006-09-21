#include "ioexception.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

using namespace mrt;

IOException::IOException() {}


const std::string IOException::getCustomMessage() {
	char buf[1024];
	memset(buf, 0, sizeof(buf));

#ifdef WIN32
	strncpy(buf, _strerror(NULL), sizeof(buf));
#else 
	strncpy(buf, strerror(errno), sizeof(buf));
//	if (strerror_r(errno, buf, sizeof(buf)-1) != 0) perror("strerror");
#endif
	return buf;
}

IOException::~IOException() throw() {}

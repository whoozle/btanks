#include "./exception.h"

using namespace fanncxx;

Exception::~Exception() throw() {}

Exception::Exception(struct fann_error *ferr) {
	const char * errstr = fann_get_errstr(ferr);
	msg = errstr?errstr:"unknown error (NULL)";
}

const std::string Exception::getCustomMessage() {
	return msg;
}


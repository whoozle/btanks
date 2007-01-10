#ifndef __FANNCXX_EXCEPTION_H__
#define __FANNCXX_EXCEPTION_H__

#include "mrt/exception.h"
#include <fann.h>

namespace fanncxx {
	DERIVE_EXCEPTION_NO_DEFAULT(Exception, (struct fann_error *ferr), std::string msg;);
}

#endif


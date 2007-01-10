#ifndef __FANNCXX_EXCEPTION_H__
#define __FANNCXX_EXCEPTION_H__

#include "mrt/exception.h"
#include <fann.h>

namespace fann {
	DERIVE_EXCEPTION_NO_DEFAULT(Exception, (struct fann_error *ferr), std::string msg;);
}

#define throw_fnet(args) throw_generic_no_default(Exception, args, ((struct fann_error *)network))

#endif


#ifndef __IOEXCEPTION_H__
#define __IOEXCEPTION_H__

#include "exception.h"

namespace mrt {

DERIVE_EXCEPTION(IOException);

}

#define throw_io(str) throw_generic(mrt::IOException, str)

#endif

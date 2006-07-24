#ifndef __SDLX_NET_EXCEPTION_H__
#define __SDLX_NET_EXCEPTION_H__

#include "mrt/exception.h"
#include "mrt/fmt.h"

namespace sdlx {
DERIVE_EXCEPTION(NetException);
}

#define throw_net(s) throw_generic(sdlx::NetException, s);

#endif

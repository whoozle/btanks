#ifndef MRT_NETEXCEPTION_H__
#define MRT_NETEXCEPTION_H__

#ifdef _WINDOWS

namespace mrt {

DERIVE_EXCEPTION(MRTAPI, IOException);

}

#define throw_io(str) throw_generic(mrt::IOException, str)

namespace mrt {

DERIVE_EXCEPTION_NO_DEFAULT(MRTAPI, NetException, (const int wsacode));

}

#define throw_net(str) throw_generic_no_default(mrt::IOException, str, (WSAGetLastError()))


#else
#	include "ioexception.h"
#	define throw_net throw_io
#endif

#endif


#ifndef MRT_NETEXCEPTION_H__
#define MRT_NETEXCEPTION_H__

#ifdef _WINDOWS
#	include "exception.h"

namespace mrt {

DERIVE_EXCEPTION_NO_DEFAULT(MRTAPI, NetException, (const int wsacode), std::string wsa_error; );

}

#define throw_net(str) throw_generic_no_default(mrt::NetException, str, (WSAGetLastError()))


#else
#	include "ioexception.h"
#	define throw_net throw_io
#endif

#endif


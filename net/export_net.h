#ifndef BTANKS_NET_EXPORT_H__
#define BTANKS_NET_EXPORT_H__

#include "mrt/export_base.h"

#if !defined DLLIMPORT || !defined DLLEXPORT
#	error include export_base from mrt library
#endif

#ifndef BTNETAPI
#	define BTNETAPI DLLIMPORT
#endif

#endif

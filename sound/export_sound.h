#ifndef BTANKS_SOUND_EXPORT_H__
#define BTANKS_SOUND_EXPORT_H__

#include "mrt/export_base.h"

#if !defined DLLIMPORT || !defined DLLEXPORT
#	error include export_base from mrt library
#endif

#ifndef BTSOUNDAPI
#	define BTSOUNDAPI DLLIMPORT
#endif

#endif

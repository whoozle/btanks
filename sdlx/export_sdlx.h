#ifndef SDLX_EXPORT_H__
#define SDLX_EXPORT_H__

#include "mrt/export_base.h"

#if !defined DLLIMPORT || !defined DLLEXPORT
#	error include export_base from mrt library
#endif

#ifndef SDLXAPI
#	define SDLXAPI DLLIMPORT
#endif

#endif

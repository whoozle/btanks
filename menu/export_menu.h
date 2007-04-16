#ifndef BTANKS_MENU_EXPORT_H__
#define BTANKS_MENU_EXPORT_H__

#include "mrt/export_base.h"

#if !defined DLLIMPORT || !defined DLLEXPORT
#	error include export_base from mrt library
#endif

#ifndef BTMENUAPI
#	define BTMENUAPI DLLIMPORT
#endif

#endif

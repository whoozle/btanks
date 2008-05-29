#ifndef __SDLX__COMMON_H__
#define __SDLX__COMMON_H__

#ifdef USE_GLSDL
#	ifdef _WINDOWS
#		include "wrappers/d3dsdl.h"
#	else
#		include "wrappers/glSDL.h"
#	endif
#else
#	include <SDL.h>
#endif

#include "sdlx/export_sdlx.h"

#endif


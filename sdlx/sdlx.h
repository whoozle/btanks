#ifndef __SDLX__COMMON_H__
#define __SDLX__COMMON_H__

#ifdef USE_GLSDL
//#	ifdef _WINDOWS
//#		include "glSDL/d3dsdl.h"
//#	else
#		include "glSDL/glSDL.h"
//#	endif
#else
#	include <SDL/SDL.h>
#endif

#include "sdlx/export_sdlx.h"

#endif


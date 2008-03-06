#ifndef SDLX_MRT_FILE_RW_H__
#define SDLX_MRT_FILE_RW_H__

#include "sdlx.h"
#include "export_sdlx.h"
namespace mrt {
	class BaseFile;
}

namespace sdlx {
	
	SDL_RWops * SDLXAPI RWFromMRTFile(mrt::BaseFile *); //no close! 

}

#endif


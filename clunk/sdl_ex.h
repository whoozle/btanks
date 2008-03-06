#ifndef __SDL_CXX_LAYER_EXCEPTION_H__
#define __SDL_CXX_LAYER_EXCEPTION_H__

#include "clunk/export_clunk.h"
#include "mrt/exception.h"

namespace clunk {
DERIVE_EXCEPTION(CLUNKAPI, Exception);
}

#define throw_sdl(s) throw_generic(clunk::Exception, s);

#endif

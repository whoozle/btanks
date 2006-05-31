#ifndef ___SDLX_TTF_H__
#define ___SDLX_TTF_H__

#include "SDL/SDL_ttf.h"
#include <string>

namespace sdlx {
class Surface;
class TTF {
public: 
	static void init();
	
	TTF();
	~TTF();
	
	void open(const std::string &fname, const int psize);
	void renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg);
	void close();
private:
	TTF_Font * _font;
};

}

#endif


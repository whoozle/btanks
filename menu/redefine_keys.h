#ifndef BTANKS_REDEFINE_KEYS_H__
#define BTANKS_REDEFINE_KEYS_H__

#include "control.h"
#include "box.h"

#include <vector>
#include <string>

namespace sdlx {
	class Font;
}

class RedefineKeys : public Control {
public: 
	RedefineKeys();

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);

private: 
	const sdlx::Surface *_bg_table;
	const sdlx::Font * _font;
	Box _background;
	
	std::vector<std::string> _actions;
};

#endif


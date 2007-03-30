#ifndef BTANKS_REDEFINE_KEYS_H__
#define BTANKS_REDEFINE_KEYS_H__

#include "control.h"
#include "box.h"

#include <vector>
#include <string>

namespace sdlx {
	class Font;
	class Rect;
}

class RedefineKeys : public Control {
public: 
	RedefineKeys();

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

private: 
	const sdlx::Surface *_bg_table;
	const sdlx::Font * _font;
	Box _background;
	
	int _active_row, _active_col;
	
	typedef std::vector<std::pair<std::string, sdlx::Rect> > Actions; 
	Actions _actions;
};

#endif


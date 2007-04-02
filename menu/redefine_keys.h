#ifndef BTANKS_REDEFINE_KEYS_H__
#define BTANKS_REDEFINE_KEYS_H__

#include "control.h"
#include "box.h"

#include <vector>
#include <string>
#include <map>

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

	void reload();
	void save(); 

private: 
	
	const sdlx::Surface *_bg_table, *_selection;
	const sdlx::Font * _font, *_small_font;
	Box _background;
	
	int _active_row, _active_col;
	
	std::vector<std::string> _labels;
	typedef std::vector<std::pair<std::string, sdlx::Rect> > Actions; 
	Actions _actions;
	
	int _keys[3][7];
};

#endif


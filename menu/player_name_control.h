#ifndef BTANKS_MENU_PLAYER_NAME_CONTROL_H__
#define BTANKS_MENU_PLAYER_NAME_CONTROL_H__

#include "container.h"
#include <string>
#include "sdlx/rect.h"

namespace sdlx {
	class Font;
	class Surface;
}

class Label;

class PlayerNameControl : public Container {
public: 
	PlayerNameControl(const std::string &label, const std::string &config_key);
	void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	const sdlx::Font * _font;
	Label * _label, *_name;
	std::string _config_key;
	
	sdlx::Rect _dice_area, _edit_area;
	const sdlx::Surface * _dice, *_edit;
};

#endif


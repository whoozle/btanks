#ifndef BTANKS_MENU_Button_H__
#define BTANKS_MENU_Button_H__

namespace sdlx {
class Surface;
class Font;
}

#include <string>
#include "control.h"
#include "box.h"

class Button : public Control {
public: 
	Button(const std::string &font, const std::string &label);
	void getSize(int &w, int &h) const;
	virtual void render(sdlx::Surface& surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
private: 
	int _w;
	Box _background;
	const sdlx::Font * _font;
	const std::string _label;
};

#endif


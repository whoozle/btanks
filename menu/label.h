#ifndef BTANKS_MENU_LABEL_H__
#define BTANKS_MENU_LABEL_H__

namespace sdlx {
class Surface;
class Font;
}

#include <string>
#include "control.h"

class Label : public Control {
public: 
	Label(const sdlx::Font *font, const std::string &label);
	virtual void render(sdlx::Surface& surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;

private: 
	const sdlx::Font * _font;
	const std::string _label;
};

#endif


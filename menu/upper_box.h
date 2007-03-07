#ifndef __MENU_UPPER_BOX_H__
#define __MENU_UPPER_BOX_H__

#include <string>
#include "box.h"
#include "control.h"
#include "sdlx/rect.h"
namespace sdlx {
	class Font;
}

class UpperBox : public Control, public Box {
public: 
	std::string value;

	UpperBox(int w, int h, const bool server);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	void getSize(int &w, int &h) const;
private: 
	int _w, _h;
	bool _server;	
	const sdlx::Surface *_checkbox;
	const sdlx::Font *_big, *_medium;
	sdlx::Rect _on_area, _off_area;
};

#endif

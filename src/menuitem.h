#ifndef __BT_MENUITEM_H__
#define __BT_MENUITEM_H__

#include "sdlx/color.h"
#include "sdlx/surface.h"
#include <string>
#include <map>

namespace sdlx {
class TTF;
}

class MenuItem {
public:
	const std::string name;
	const std::string type;
	
	MenuItem(sdlx::TTF &font, const std::string &name, const std::string &type, const std::string &value);
	void render(sdlx::Surface &dst, const int x, const int y, const bool inverse);
	void getSize(int &w, int &h) const;

	virtual void onClick() {}
	virtual ~MenuItem() {}

private:
	void render(sdlx::TTF &);
	sdlx::TTF & _font;
		
	sdlx::Color _color;
	sdlx::Surface _normal, _inversed;
	std::string _value;
};

/*
class ChoiceItem : public MenuItem {
public:
	typedef std::map<int, std::string> Choices;
	
	ChoiceItem(sdlx::TTF &font, const std::string &name, const std::string &type, const Choices &choices);
private:
};
*/
#endif

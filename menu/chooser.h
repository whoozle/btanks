#ifndef BTANKS_MENU_CHOOSER_H__
#define BTANKS_MENU_CHOOSER_H__

#include "container.h"
#include "sdlx/rect.h"
#include <string>
#include <vector>

namespace sdlx {
class Surface;
class Font;
}

class Chooser : public Container {
public: 
	Chooser(const std::string &surface, const int n);
	Chooser(const std::string &font, const std::vector<std::string> &options);
	void getSize(int &w, int &h);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	void set(const int i);
	void set(const std::string &name);
	const std::string& getValue() const;
	
	const bool changed() const { return _changed; }
	void reset() { _changed = false; }
	
	void left();
	void right();

private: 
	bool _changed;
	std::vector<std::string> _options;
	int _i, _n;
	const sdlx::Surface *_surface, *_left_right;

	//textual chooser: 
	const sdlx::Font *_font;
	int _w;

	sdlx::Rect _left_area, _right_area;
};


#endif

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
	Chooser(const std::string &font, const std::vector<std::string> &options, const std::string &surface = std::string());
	void getSize(int &w, int &h) const;

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	void set(const int i);
	const int get() const { return _i; }
	const int size() const { return _n; }
	void set(const std::string &name);
	const std::string& getValue() const;
	
	void left();
	void right();
	
	void disable(const int i, const bool value = true);

private: 
	std::vector<std::string> _options;
	std::vector<bool> _disabled;
	int _i, _n;
	const sdlx::Surface *_surface, *_left_right;

	//textual chooser: 
	const sdlx::Font *_font;
	int _w;

	sdlx::Rect _left_area, _right_area;
};


#endif

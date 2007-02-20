#ifndef BTANKS_BOX_H__
#define BTANKS_BOX_H__

#include <string>

namespace sdlx {
class Surface;
}

class Box {
public: 
	Box() : _surface(0) {}
	int w, h;

	const bool inited() const { return _surface != 0; }
	void init(const std::string &tile, int w, int h);
	
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual ~Box() {}
	
	void getMargins(int &v, int &h) const;
private: 
	int x1, x2, y1, y2, xn, yn;
	
	const sdlx::Surface *_surface;
};

#endif


#ifndef BTANKS_MENU_SLIDER_H__
#define BTANKS_MENU_SLIDER_H__

#include "control.h"

class Slider : public Control {
public: 
	Slider(const float value);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	const sdlx::Surface * _tiles;
	int _n;
	float _value;
};

#endif


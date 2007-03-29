#ifndef BTANKS_MENU_SLIDER_H__
#define BTANKS_MENU_SLIDER_H__

#include "control.h"
#include <sigc++/sigc++.h>

class Slider : public Control, public sigc::trackable {
public: 
	Slider(const float value);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	const float get() const { return _value; }

private: 
	bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	const sdlx::Surface * _tiles;
	int _n;
	float _value;

	bool _grab;
	int _grab_state;
};

#endif


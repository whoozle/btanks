#ifndef __BTANKS_NUMBER_CONTROL_H__
#define __BTANKS_NUMBER_CONTROL_H__

#include <string>
#include "sdlx/rect.h"
#include "menu/control.h"

namespace sdlx {
	class Surface;
	class Font;
}

class BTANKSAPI NumberControl : public Control {
public: 
	NumberControl(const std::string &font, const int min = 0, const int max = 9999, const int step = 1);

	const int get() const;
	void set(const int v);

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	void up(const int v = 1);
	void down(const int v = 1);

	virtual void tick(const float dt);
	void setMinMax(const int m1, const int m2);

private: 
	void validate();
	int min, max, step, value;
	float mouse_pressed;
	int mouse_button;
	bool direction;
	
	const sdlx::Surface *_number;
	const sdlx::Font *_font;
	sdlx::Rect r_up, r_down;
};

#endif


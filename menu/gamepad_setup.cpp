#include "gamepad_setup.h"
#include "resource_manager.h"
#include "chooser.h"
#include "tooltip.h"

#include "math/binary.h"

#include "sdlx/surface.h"
#include "sdlx/joystick.h"
#include "i18n.h"

GamepadSetup::GamepadSetup(const int w, const int h) : _current_pad(NULL) {
	int mx, my;

	_gamepad_bg = ResourceManager->loadSurface("menu/gamepad.png");
	_gamepad_buttons = ResourceManager->loadSurface("menu/gamepad_buttons.png");
	_gamepad_ministick = ResourceManager->loadSurface("menu/gamepad_ministick.png");
	_background.init("menu/background_box_dark.png", w, h);
	_background.getMargins(mx, my);

	int n = sdlx::Joystick::getCount();
	//LOG_DEBUG(("%d joystick(s) found", n));
	std::vector<std::string> names;
	for(int i = 0; i < n; ++i) {
		std::string name = mrt::formatString("%s %d %s %d: %s", 
			I18n->get("menu", "joystick").c_str(), i + 1, 
			I18n->get("menu", "of").c_str(), n, sdlx::Joystick::getName(i).c_str());
		if (i == 0)
			load(sdlx::Joystick::getName(i));
		names.push_back(name);
	}
	
	int sw, sh;
	_current_pad = new Chooser("small", names);
	_current_pad->getSize(sw, sh);
	_gamepad_bg_pos = v2<int>(mx, my + sh + 10);
	add((w - sw - mx * 2) / 2, my, _current_pad);
	Tooltip * t = new Tooltip(I18n->get("menu", "test-gamepad"), false, w - 2 * mx - _gamepad_bg->getHeight() - 60);
	t->getSize(sw, sh);
	add(w - mx - sw, _gamepad_bg_pos.y, t);
}

void GamepadSetup::renderIcon(sdlx::Surface &surface, const int idx, const int x, const int y) {
	const int w = _gamepad_buttons->getWidth() / 7;
	const int h = _gamepad_buttons->getHeight();

	const sdlx::Rect src(w * idx, 0, w, h);
	
	surface.copyFrom(*_gamepad_buttons, src, _gamepad_bg_pos.x + x, _gamepad_bg_pos.y + y);
}

void GamepadSetup::renderButton(sdlx::Surface &surface, const int b, const int x, const int y) {
	assert(b >= 0 && b < 10);

	int idx = (b >= 4)?((b >= 8)?5:0):6;

	static int xp[10] = {332, 299, 366, 332,  70, 70, -120, -120,  172, 242,  };
	static int yp[10] = {226, 194, 193, 162,  43, 69,  43,   69,  198, 198,  };
	
	renderIcon(surface, idx, 
		x + (xp[b] >= 0?xp[b]:_gamepad_bg->getWidth() + xp[b]), 
		y + (yp[b] >= 0?yp[b]:_gamepad_bg->getHeight() + yp[b]));
}


void GamepadSetup::tick(const float dt) {
	if (_current_pad->changed()) {
		_current_pad->reset();
		int i = _current_pad->get();
		load(sdlx::Joystick::getName(i));
	}
	Container::tick(dt);
}

void GamepadSetup::load(const std::string &profile) {
	LOG_DEBUG(("loading profile '%s'", profile.c_str()));
	_profile = profile;	
}

void GamepadSetup::reload() {
	joy.close();
	joy.open(_current_pad?_current_pad->get():0);
}

void GamepadSetup::save() {
	joy.close();
}

void GamepadSetup::renderDPad(sdlx::Surface &surface, const bool left, const bool right, const bool up, const bool down, const int x, const int y) {
	static const int xp[] =   { 62,  98, 85,  85};
	static const int yp[] =   {193, 193, 170, 206};
	static const int icon[] = {2,  4,  1,  3 };

	if (left) 
		renderIcon(surface, icon[0], xp[0], yp[0]);
	if (right) 
		renderIcon(surface, icon[1], xp[1], yp[1]);
	if (up) 
		renderIcon(surface, icon[2], xp[2], yp[2]);
	if (down) 
		renderIcon(surface, icon[3], xp[3], yp[3]);
}

void GamepadSetup::renderMinistick(sdlx::Surface &surface, const int ai, const int x, const int y, const bool swap) {
	const int r = 16;
	const int xa = joy.getAxis(ai + (swap?0:1)) * r / 32767;
	const int ya = joy.getAxis(ai + (swap?1:0)) * r / 32767;
	int idx = ai / 2;
	assert(idx < 2);
	int xp[] = { 95, 220};
	int yp[] = {203, 203};
	
	surface.copyFrom(*_gamepad_ministick, _gamepad_bg_pos.x + xp[idx] + xa + _gamepad_ministick->getWidth() / 2, _gamepad_bg_pos.y + yp[idx] + ya + _gamepad_ministick->getHeight() / 2);
}

void GamepadSetup::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	surface.copyFrom(*_gamepad_bg, x + _gamepad_bg_pos.x, y + _gamepad_bg_pos.y);
	Container::render(surface, x, y);

	SDL_JoystickUpdate();
	
	int hats = joy.getNumHats();
	int axes = joy.getNumAxes();
	
	if (hats) {
		//assume first hat as D-PAD
		int hat = joy.getHat(0);
		renderDPad(surface, hat & SDL_HAT_LEFT, hat & SDL_HAT_RIGHT, hat & SDL_HAT_UP, hat & SDL_HAT_DOWN, x, y);
	} else {
		if (axes >= 6 || axes == 2) {
			//no hats. axe 4,5 - DPAD
			int base = (axes == 2)?0:4;
			int xa = joy.getAxis(base), ya = joy.getAxis(base + 1);
			static const int threshold = 3276;
			renderDPad(surface, xa < -threshold, xa > threshold, ya < -threshold, ya > threshold, x, y);
		}
	}

	if (axes >= ((hats)?4:6)) {
#ifdef WIN32
		renderMinistick(surface, 0, x, y);
#else
		renderMinistick(surface, 0, x, y, true);
#endif
		renderMinistick(surface, 2, x, y, true);
	}

	int n = math::min(joy.getNumButtons(), 10);
	for(int i = 0; i < n; ++i)  {
		if (joy.getButton(i))
			renderButton(surface, i, x, y);
	}
}

void GamepadSetup::getSize(int &w, int &h) const {
	Container::getSize(w, h);
	
	if (_background.w > w)
		w = _background.w;
	
	if (_background.h > h)
		h = _background.h;
}

bool GamepadSetup::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_RETURN:
	case SDLK_ESCAPE: 
		hide();
		save();
		return true;

	default: 
		return true;
	}
	Container::onKey(sym);
	return true;
}

bool GamepadSetup::onMouse(const int button, const bool pressed, const int x, const int y) {
	Container::onMouse(button, pressed, x, y);
	return true;
}

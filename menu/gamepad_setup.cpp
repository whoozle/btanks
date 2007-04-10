#include "gamepad_setup.h"
#include "resource_manager.h"
#include "chooser.h"

#include "sdlx/surface.h"
#include "sdlx/joystick.h"

GamepadSetup::GamepadSetup(const int w, const int h) {
	int mx, my;

	_gamepad_bg = ResourceManager->loadSurface("menu/gamepad.png");
	_background.init("menu/background_box_dark.png", w, h);
	_background.getMargins(mx, my);

	int n = sdlx::Joystick::getCount();
	LOG_DEBUG(("%d joystick(s) found", n));
	std::vector<std::string> names;
	for(int i = 0; i < n; ++i) {
		names.push_back(sdlx::Joystick::getName(i));
	}
	
	int sw, sh;
	_current_pad = new Chooser("small", names);
	_current_pad->getSize(sw, sh);
	_gamepad_bg_y = my + sh + 10;
	add((w - sw - mx * 2) / 2, my, _current_pad);
}

void GamepadSetup::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	surface.copyFrom(*_gamepad_bg, x + /*(_background.w - _gamepad_bg->getWidth()) / 2 */ mx, y + _gamepad_bg_y);
	Container::render(surface, x, y);
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
		hide();
		//save();
		return true;

	case SDLK_ESCAPE: 
		hide();
		//reload();
		return true;
	default: 
		return true;
	}
	
	return true;
}

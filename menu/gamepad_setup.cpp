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
		std::string name = sdlx::Joystick::getName(i);
		if (i == 0)
			load(name);
		names.push_back(name);
	}
	
	int sw, sh;
	_current_pad = new Chooser("small", names);
	_current_pad->getSize(sw, sh);
	_gamepad_bg_pos = v2<int>(mx, my + sh + 10);
	add((w - sw - mx * 2) / 2, my, _current_pad);
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

void GamepadSetup::save() {
	
}

void GamepadSetup::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	surface.copyFrom(*_gamepad_bg, x + _gamepad_bg_pos.x, y + _gamepad_bg_pos.y);
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

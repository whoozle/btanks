#include "player_name_control.h"
#include "resource_manager.h"
#include "nickname.h"
#include "config.h"
#include "label.h"
#include "sdlx/surface.h"

PlayerNameControl::PlayerNameControl(const std::string &label, const std::string &config_key) : 
	_font(ResourceManager->loadFont("small", true)), _config_key(config_key) {
	std::string name;
	Config->get(config_key, name, Nickname::generate());

	_label = new Label(_font, label);
	_name = new Label(_font, name);

	add(0, 0, _label);
	int sw, sh;
	_label->getSize(sw, sh);
	add(sw, 0, _name);

	Container::getSize(sw, sh);
	_dice = ResourceManager->loadSurface("menu/dice.png");

	_dice_area.x = sw + 2;
	_dice_area.y = (sh - _dice->getHeight()) / 2;
	_dice_area.w = _dice->getWidth();
	_dice_area.h = _dice->getHeight();
}

bool PlayerNameControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_dice_area.in(x, y)) {
		if (!pressed) 
			return true;
		std::string name = Nickname::generate();
		Config->set(_config_key, name);
		_name->set(name);

		int bw, bh;
		Container::getSize(bw, bh);
		_dice_area.x = bw + 2;

		invalidate(true);
		return true;
	} 
	if (Container::onMouse(button, pressed, x, y))
		return true;
	return false;
}

void PlayerNameControl::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	surface.copyFrom(*_dice, x + _dice_area.x, y + _dice_area.y);
}

void PlayerNameControl::getSize(int &w, int &h) const {
	Container::getSize(w, h);
	w += _dice_area.w + 4;
}

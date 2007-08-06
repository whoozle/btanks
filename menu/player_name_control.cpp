#include "player_name_control.h"
#include "resource_manager.h"
#include "nickname.h"
#include "config.h"
#include "label.h"
#include "sdlx/surface.h"

PlayerNameControl::PlayerNameControl(const std::string &label, const std::string &config_key) : 
	_font(ResourceManager->loadFont("small", true)), _config_key(config_key), _edit_flag(false) {
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
	_edit = ResourceManager->loadSurface("menu/edit.png");

	_dice_area.x = sw + 4;
	_dice_area.y = (sh - _edit->getHeight()) / 2;
	_dice_area.w = _dice->getWidth();
	_dice_area.h = _dice->getHeight();
	
	_edit_area.x = _dice_area.x + _dice_area.w + 6;
	_edit_area.y = (sh - _edit->getHeight()) / 2;
	_edit_area.w = _edit->getWidth();
	_edit_area.h = _edit->getHeight();
}

bool PlayerNameControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_dice_area.in(x, y)) {
		if (pressed) 
			return true;
	
		std::string name = Nickname::generate();
		Config->set(_config_key, name);
		_name->set(name);

		int bw, bh;
		Container::getSize(bw, bh);
	
		_dice_area.x = bw + 4;
		_edit_area.x = _dice_area.x + _dice_area.w + 6;

		_edit_flag = false;
		invalidate(true);
		return true;
	}
	if (_edit_area.in(x, y)) {
		if (pressed)
			return true;
		_edit_flag = true;
		invalidate(true);
		
		//LOG_DEBUG(("edit name!"));
		return true;
	}
	if (Container::onMouse(button, pressed, x, y))
		return true;
	return false;
}

void PlayerNameControl::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	surface.copyFrom(*_dice, x + _dice_area.x, y + _dice_area.y);
	surface.copyFrom(*_edit, x + _edit_area.x, y + _edit_area.y);
}

void PlayerNameControl::getSize(int &w, int &h) const {
	Container::getSize(w, h);
	w += _dice_area.w + _edit_area.w + 10;
}

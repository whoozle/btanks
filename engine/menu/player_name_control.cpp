#include "player_name_control.h"
#include "resource_manager.h"
#include "nickname.h"
#include "config.h"
#include "label.h"
#include "sdlx/surface.h"
#include "mrt/utf8_utils.h"

PlayerNameControl::PlayerNameControl(const std::string &label, const std::string &config_key, const int w) : 
	_font(ResourceManager->loadFont("small", true)), _config_key(config_key), _edit_flag(false), width(w) {
	_dice = ResourceManager->load_surface("menu/dice.png");
	_edit = ResourceManager->load_surface("menu/edit.png");

	std::string name;
	Config->get(config_key, name, Nickname::generate());
	mrt::utf8_resize(name, 32);

	_label = new Label(_font, label);
	_name = new Label(_font, name);

	int sw, sh;
	_label->get_size(sw, sh);
	add(-sw, 0, _label);

	int label_w = width - ( _dice->get_width() + _edit->get_width() + 10);
	if (label_w < 0)
		label_w = 4; //lol :)
	_name->set_size(label_w, sh);
	add(0, 0, _name);

	Container::get_size(sw, sh);
	if (w > 0) {
		sw = w - _edit->get_width() - _dice->get_width() - 10;
	}

	_dice_area.x = sw + 4;
	_dice_area.y = (sh - _edit->get_height()) / 2;
	_dice_area.w = _dice->get_width();
	_dice_area.h = _dice->get_height();
	
	_edit_area.x = _dice_area.x + _dice_area.w + 6;
	_edit_area.y = (sh - _edit->get_height()) / 2;
	_edit_area.w = _edit->get_width();
	_edit_area.h = _edit->get_height();
}

const std::string PlayerNameControl::get() const {
	return _name->get();
}

void PlayerNameControl::set(const std::string &name) {
	Config->set(_config_key, name);
	_name->set(name);
	_edit_flag = false;
	invalidate(true);
}

bool PlayerNameControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("%d,%d -> %d,%d,%d,%d", x, y, _dice_area.x, _dice_area.y, _dice_area.w, _dice_area.h));
	if (_dice_area.in(x, y)) {
		if (pressed) 
			return true;
	
		std::string name = Nickname::generate();
		set(name);
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
	return Container::onMouse(button, pressed, x, y);
}

void PlayerNameControl::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	surface.blit(*_dice, x + _dice_area.x, y + _dice_area.y);
	surface.blit(*_edit, x + _edit_area.x, y + _edit_area.y);
}

void PlayerNameControl::get_size(int &w, int &h) const {
	Container::get_size(w, h);
	w = width > 0? width: w + _dice_area.w + _edit_area.w + 10;
}

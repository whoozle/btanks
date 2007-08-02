#include "player_name_control.h"
#include "resource_manager.h"
#include "nickname.h"
#include "config.h"
#include "label.h"

PlayerNameControl::PlayerNameControl(const std::string &label, const std::string &config_key) : 
	_font(ResourceManager->loadFont("medium", true)), _config_key(config_key) {
	std::string name;
	Config->get(config_key, name, Nickname::generate());

	_label = new Label(_font, label);
	_name = new Label(_font, name);

	add(0, 0, _label);
	int sw, sh;
	_label->getSize(sw, sh);
	add(sw, 0, _name);
}

#include "player_name_control.h"
#include "resource_manager.h"
#include "i18n.h"

PlayerNameControl::PlayerNameControl(const std::string &label, const std::string &config_key) : 
	_font(ResourceManager->loadFont("medium", true)), _config_key(config_key) {
}

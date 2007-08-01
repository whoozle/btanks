#ifndef BTANKS_MENU_PLAYER_NAME_CONTROL_H__
#define BTANKS_MENU_PLAYER_NAME_CONTROL_H__

#include "container.h"
#include <string>

namespace sdlx {
	class Font;
}

class Label;

class PlayerNameControl : public Container {
public: 
	PlayerNameControl(const std::string &label, const std::string &config_key);
private: 
	const sdlx::Font * _font;
	Label * _label;
	std::string _config_key;
};

#endif


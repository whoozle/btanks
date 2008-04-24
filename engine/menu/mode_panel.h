#ifndef BTANKS_MENU_MODE_PANEL_H__
#define BTANKS_MENU_MODE_PANEL_H__

#include "container.h"

class Box;
class MapDesc;
class Chooser;
class Checkbox;

class ModePanel : public Container {
public: 
	ModePanel(const int w);

	void tick(const float dt);
	void set(const MapDesc &map);

private: 
	Box *_background;

	typedef std::map<const int, std::string> TimeLimits;
	TimeLimits _time_limits;

	Chooser *_time_limit, *_teams;
	Checkbox * _random_respawn;
};

#endif


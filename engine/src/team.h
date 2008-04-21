#ifndef BTANKS_ENGINE_TEAM_H__
#define BTANKS_ENGINE_TEAM_H__

#include "export_btanks.h"

class Object;

struct BTANKSAPI Team {
	enum ID { None = -1, Red = 0, Green = 1, Blue = 2, Yellow = 3 };

	static const char * get_color(ID t);
	static ID get_team(const Object *o);
};

#endif

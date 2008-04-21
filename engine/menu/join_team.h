#ifndef BTANKS_MENU_JOIN_TEAM_H__
#define BTANKS_MENU_JOIN_TEAM_H__

#include "container.h"
#include "sdlx/surface.h"

class Box;
class JoinTeamControl : public Container {
public: 
	JoinTeamControl();
	void update();
	virtual void render(sdlx::Surface& surface, const int x, const int y) const;
private: 
	int teams;
	Box * _background;
	sdlx::Surface team_logo[4];
};

#endif


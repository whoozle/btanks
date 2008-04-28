#ifndef BTANKS_MENU_JOIN_TEAM_H__
#define BTANKS_MENU_JOIN_TEAM_H__

#include "container.h"
#include "sdlx/surface.h"

namespace sdlx {
	class Font;
}

class Box;
class Label;

class JoinTeamControl : public Container {
public: 
	JoinTeamControl();
	void update();
	virtual void render(sdlx::Surface& surface, const int x, const int y) const;
	
	void left();
	void right();
	
	int get() const { return current_team; }
private: 
	int teams, current_team;
	Box * _background;
	Label * _title;
	sdlx::Surface team_logo[4];
	const sdlx::Surface *join_logo;
};

#endif


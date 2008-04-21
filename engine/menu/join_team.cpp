#include "join_team.h"
#include "box.h"
#include "rt_config.h"
#include "team.h"

#define SQUARE_SIZE 64
#define SQUARE_SPACING 16

JoinTeamControl::JoinTeamControl() {
	teams = RTConfig->teams;
	if (teams < 2 || teams > 4) 
		throw_ex(("CTF teams counter was not set up properly (%d)", teams));

	add(0, 0, _background = new Box("menu/background_box.png", 2 * SQUARE_SPACING + (SQUARE_SIZE + SQUARE_SPACING) * teams, SQUARE_SIZE + 2 * SQUARE_SPACING));

	static Uint8 colors[][4] = {
		{255, 0, 0, 128}, 
		{0, 255, 0, 128}, 
		{0, 0, 255, 128}, 
		{255, 255, 0, 128}, 
	};
	for(int i = 0; i < teams; ++i) {
		team_logo[i].createRGB(SQUARE_SIZE, SQUARE_SIZE, 32);
		team_logo[i].convertAlpha();
		team_logo[i].fill(team_logo[i].mapRGBA(colors[i][0], colors[i][1], colors[i][2], colors[i][3]));
	}
}

void JoinTeamControl::render(sdlx::Surface& surface, const int x, const int y) const {
	int w, h;
	getSize(w, h);
	int xp = (w - (SQUARE_SIZE + SQUARE_SPACING) * teams + SQUARE_SPACING) / 2;
	int yp = (h - (SQUARE_SIZE + SQUARE_SPACING * 2)) / 2;
	for(int i = 0; i < teams; ++i) {
		surface.copyFrom(team_logo[i], x + xp + (SQUARE_SIZE + SQUARE_SPACING) * i, yp);
	}
}

#include "join_team.h"
#include "box.h"
#include "rt_config.h"
#include "team.h"
#include "resource_manager.h"
#include "i18n.h"
#include "math/binary.h"
#include "menu/label.h"

#define SQUARE_SIZE 64
#define SQUARE_SPACING 16

JoinTeamControl::JoinTeamControl() {
	teams = RTConfig->teams;
	if (teams < 2 || teams > 4) 
		throw_ex(("CTF teams counter was not set up properly (%d)", teams));
	_title = new Label("medium", I18n->get("menu", "choose-team"));
	int title_w, title_h;
	_title->getSize(title_w, title_h);

	int w = math::max(SQUARE_SPACING + (SQUARE_SIZE + SQUARE_SPACING) * teams,  title_w + 2 * SQUARE_SPACING), h = SQUARE_SIZE + 2 * SQUARE_SPACING + title_h;
	_background = new Box("menu/background_box_dark.png", w, h);

	add(0, 0, _background);
	
	int mx, my;
	_background->getMargins(mx, my);
	_background->getSize(w, h);

	add((w - title_w) / 2, my, _title);

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
	Container::render(surface, x, y);
	int w, h, mx, my;
	getSize(w, h);
	_background->getMargins(mx, my);
	int title_w, title_h;
	_title->getSize(title_w, title_h);

	int xp = mx + SQUARE_SPACING + (w - 2 * mx - (SQUARE_SIZE + SQUARE_SPACING) * teams - SQUARE_SPACING) / 2;
	int yp = my + SQUARE_SPACING + (h - 2 * my - (SQUARE_SIZE + SQUARE_SPACING * 2)) / 2 + title_h;
	for(int i = 0; i < teams; ++i) {
		surface.copyFrom(team_logo[i], x + xp + (SQUARE_SIZE + SQUARE_SPACING) * i, y + yp);
	}
}

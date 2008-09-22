#include "join_team.h"
#include "box.h"
#include "rt_config.h"
#include "team.h"
#include "resource_manager.h"
#include "i18n.h"
#include "math/binary.h"
#include "menu/label.h"
#include "sdlx/font.h"

#define SQUARE_SIZE 64
#define SQUARE_SPACING 16

JoinTeamControl::JoinTeamControl(): 
current_team(0), join_logo(ResourceManager->load_surface("menu/team_chooser.png")) , _font(ResourceManager->loadFont("medium", true))
	{
	teams = RTConfig->teams;
	if (teams < 2 || teams > 4) 
		throw_ex(("CTF teams counter was not set up properly (%d)", teams));
	_title = new Label("medium", I18n->get("menu", "choose-team"));
	int title_w, title_h;
	_title->get_size(title_w, title_h);

	int w = math::max(SQUARE_SPACING + (SQUARE_SIZE + SQUARE_SPACING) * teams,  title_w + 2 * SQUARE_SPACING), h = SQUARE_SIZE + 2 * SQUARE_SPACING + title_h;
	_background = new Box("menu/background_box_dark.png", w, h);

	add(0, 0, _background);
	
	int mx, my;
	_background->getMargins(mx, my);
	_background->get_size(w, h);

	add((w - title_w) / 2, my, _title);

	static Uint8 colors[][4] = {
		{255, 0, 0, 128}, 
		{0, 255, 0, 128}, 
		{0, 0, 255, 128}, 
		{255, 255, 0, 128}, 
	};
	
	for(int i = 0; i < teams; ++i) {
		team_logo[i].create_rgb(SQUARE_SIZE, SQUARE_SIZE, 32);
		team_logo[i].display_format_alpha();
		team_logo[i].fill(team_logo[i].map_rgba(colors[i][0], colors[i][1], colors[i][2], colors[i][3]));
	}
	memset(team_stats, 0, sizeof(team_stats));
}

#include "player_manager.h"

void JoinTeamControl::tick(const float dt) {
	Container::tick(dt);
	memset(team_stats, 0, sizeof(team_stats));
	int n = PlayerManager->get_slots_count();
	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = PlayerManager->get_slot(i);
		if (slot.team != Team::None) 
			++team_stats[(int)slot.team];
	}
}

void JoinTeamControl::render(sdlx::Surface& surface, const int x, const int y) const {
	Container::render(surface, x, y);

	int w, h, mx, my;
	get_size(w, h);
	_background->getMargins(mx, my);
	int title_w, title_h;
	_title->get_size(title_w, title_h);
	
	int dx = (SQUARE_SIZE - join_logo->get_width()) / 2, dy = (SQUARE_SIZE - join_logo->get_height()) / 2;

	int xp = mx + SQUARE_SPACING + (w - 2 * mx - (SQUARE_SIZE + SQUARE_SPACING) * teams - SQUARE_SPACING) / 2;
	int yp = my + SQUARE_SPACING + (h - 2 * my - (SQUARE_SIZE + SQUARE_SPACING * 2)) / 2 + title_h;
	for(int i = 0; i < teams; ++i) {
		int x0 = x + xp + (SQUARE_SIZE + SQUARE_SPACING) * i, y0 = y + yp;
		surface.blit(team_logo[i], x0, y0);
		
		std::string players = mrt::format_string("%d", team_stats[i]);
		int w = _font->render(NULL, 0, 0, players);
		_font->render(surface, x0 + (SQUARE_SIZE - w) / 2, y0 + (SQUARE_SIZE - _font->get_height()) / 2, players);
		
		if (i == current_team)
			surface.blit(*join_logo, x0 + dx, y0 + dy);
	}
}

void JoinTeamControl::left() {
	if (current_team > 0)
		--current_team;
	invalidate();
}

void JoinTeamControl::right() {
	if (current_team + 1 < teams)
		++current_team;
	invalidate();
}

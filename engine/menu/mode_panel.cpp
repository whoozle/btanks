#include "mode_panel.h"
#include "i18n.h"
#include "map_desc.h"
#include "chooser.h"
#include "checkbox.h"
#include "box.h"
#include "config.h"
#include "label.h"

ModePanel::ModePanel(const int w) : _w(w), _time_limit(NULL), _random_respawn(NULL) {
	_time_limits.insert(std::pair<const int, std::string>(0,   "-:--"));
	_time_limits.insert(std::pair<const int, std::string>(60,  "1:00"));
	_time_limits.insert(std::pair<const int, std::string>(90,  "1:30"));
	_time_limits.insert(std::pair<const int, std::string>(120, "2:00"));
	_time_limits.insert(std::pair<const int, std::string>(180, "3:00"));
	_time_limits.insert(std::pair<const int, std::string>(300, "5:00"));
	_time_limits.insert(std::pair<const int, std::string>(420, "7:00"));
	_time_limits.insert(std::pair<const int, std::string>(600, "9:99"));
}

#include "grid.h"

void ModePanel::set(const MapDesc &map) {
	clear();
	
	_time_limit = NULL;
	_random_respawn = NULL;

	if (map.game_type == GameTypeDeathMatch) {
		add(0, 0, _background = new Box("menu/background_box.png", _w, 80));
		
		int w, h;
		getSize(w, h);
		int mx, my;
		_background->getMargins(mx, my);

		std::vector<std::string> values;

		int tl, pos = 0, idx = 0;
		Config->get("multiplayer.time-limit", tl, 300);

		for(TimeLimits::const_iterator i = _time_limits.begin(); i != _time_limits.end(); ++i, ++idx) {
			values.push_back(i->second);
			if (i->first <= tl)
				pos = idx;
		}
		
		Grid *grid = new Grid(2, 2);
		add(mx, my, grid);
		
		_time_limit = new Chooser("big", values);
		_time_limit->set(pos);
		grid->set(0, 0, _time_limit, Grid::Middle | Grid::Center);
		grid->set(0, 1, new Label("small", I18n->get("menu", "time-limit")), Grid::Middle);
		
		bool rr;
		Config->get("multiplayer.random-respawn", rr, false);

		grid->set(1, 0, _random_respawn = new Checkbox(rr), Grid::Middle | Grid::Center);
		grid->set(1, 1, new Label("small", I18n->get("menu", "random-respawn")), Grid::Middle);
	}
}

void ModePanel::tick(const float dt) {
	if (_time_limit != NULL && _time_limit->changed()) {
		_time_limit->reset();
		int idx = _time_limit->get();
		if (idx >= 0) {
			assert(idx < (int)_time_limits.size());
			TimeLimits::const_iterator i;
			for (i = _time_limits.begin(); idx-- && i != _time_limits.end(); ++i);
			assert(i != _time_limits.end());
			Config->set("multiplayer.time-limit", i->first);
		}
	}
	if (_random_respawn != NULL && _random_respawn->changed()) {
		_random_respawn->reset();
		Config->set("multiplayer.random-respawn", _random_respawn->get());
	}
}

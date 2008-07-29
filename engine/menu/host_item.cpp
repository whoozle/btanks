#include "host_item.h"
#include "label.h"
#include "resource_manager.h"
#include "i18n.h"
#include "rt_config.h"
#include "sdlx/font.h"

HostItem::HostItem() : ping(0), players(0), slots(0), game_type(GameTypeDeathMatch), 
	_line(new Label("small", "")), _font(ResourceManager->loadFont("small", true)), timer(0) {
	add(0, 0, _line);
}

void HostItem::start(float t) {
	timer = t;
}

void HostItem::tick(const float dt) {
	Container::tick(dt);
	if (timer > 0) {
		timer -= dt;
		if (timer < 0) 
			timer = 0;
	}
}


static const char * get_type(const GameType game_type) {
	switch(game_type) {
	case GameTypeDeathMatch: return "deathmatch"; 
	case GameTypeCooperative: return "cooperative";
	case GameTypeRacing: return "racing";
	case GameTypeCTF: return "ctf"; 
	case GameTypeTeamDeathMatch: return "team-deathmatch";
	default: 
		return "**invalid**";
	}
}

void HostItem::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	
	if (timer > 0) {
		int w, h; 
		get_size(w, h);
	
		const char * slash = "|\\-/";
		_font->render(surface, x + w + 4, y, std::string(&slash[((int)(timer * 10)) % 4], 1));
	}
}

void HostItem::update() {
	std::string prefix = slots != 0? mrt::format_string("[%d/%d] ", players, slots) : std::string("[-/-] ");
	std::string mapstr;
	
	if (ping > 0) {
		mapstr = "[";
		if (!map.empty())
			mapstr += mrt::format_string("%s: %s (%s), ", I18n->get("menu", "map").c_str(), map.c_str(), get_type(game_type));
		mapstr += mrt::format_string("%s: %d ms]", I18n->get("menu", "ping").c_str(), ping - 1);
	}
	
	std::string hoststr = name, ip = addr.getAddr(addr.port != RTConfig->port);
	if (hoststr.empty()) {
		hoststr = ip;
	} else if (!ip.empty()) {
		hoststr += " (" + ip + ") ";
	}
	hoststr += "  ";
	_line->set(prefix + hoststr + mapstr);
	timer = 0;
}

#include "campaign_menu.h"
#include "box.h"
#include "finder.h"
#include "i18n.h"
#include "chooser.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "scroll_list.h"
#include "mrt/directory.h"
#include "math/binary.h"
#include "menu.h"
#include "label.h"
#include "game_monitor.h"
#include "game.h"
#include "player_manager.h"
#include "window.h"
#include "player_slot.h"
#include "config.h"

void CampaignMenu::start() {
	int ci = _active_campaign->get();
	const Campaign &campaign = _campaigns[ci];
	const Campaign::Map &map = campaign.maps[map_id[_maps->get()]];
	LOG_DEBUG(("campaign: %s, map: %s", campaign.name.c_str(), map.id.c_str()));
	//ensure world is created 
	Game->clear();
	GameMonitor->loadMap(campaign.name, map.id);
	
	PlayerSlot &slot = PlayerManager->getSlot(0);
	std::string cm;
	Config->get("player.control-method", cm, "keys");
	slot.createControlMethod(cm);

	std::string object, vehicle;
	PlayerManager->getDefaultVehicle(object, vehicle);

	slot.spawnPlayer(object, vehicle);
	PlayerManager->setViewport(0, Window->getSize());
	
	PlayerManager->startServer();	
}

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _w(w), _h(h) {
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty())
		return;

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));
	std::vector<std::string> titles;
	for(size_t i = 0; i < files.size(); ++i) {
		LOG_DEBUG(("campaign[%u]: %s", (unsigned)i, files[i].first.c_str()));
		Campaign c;
		c.base = files[i].first;
		c.init();
		_campaigns.push_back(c);
		titles.push_back(c.title);
	}

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
	int mx, my;
	b->getMargins(mx, my);

	int cw, ch;
	_active_campaign = new Chooser("medium", titles);
	_active_campaign->getSize(cw, ch);
	add(w / 2 - cw / 2, my, _active_campaign);

	int map_w = _w / 2;
	map_view = sdlx::Rect(mx * 2, my * 2 + ch, map_w, 3 * map_w / 4);
	
	_maps = new ScrollList("menu/background_box.png", "medium", w - map_view.w - 6 * mx, map_view.h );
	int sw, sh;
	_maps->getSize(sw, sh);
	add(w - sw - 2 * mx, map_view.y, _maps);

	int xbase, ybase;
	add(xbase = (w - sw - 2 * mx), ybase = (map_view.y + sh + 2 * mx), b = new Box("menu/background_box.png", w - map_view.w - 6 * mx, h - map_view.y - sh - 6 * my));
	b->getSize(bw, bh);
	b->getMargins(mx, my);
	
	Label *label = new Label("medium", I18n->get("menu", "score"));
	add(xbase + mx, ybase + my, label);
	label->getSize(cw, ch);

	_score = new Label("medium", "0");
	add(xbase + mx + cw, ybase + my, _score);
	
	init();
}

void CampaignMenu::init() {
	int ci = _active_campaign->get();
	const Campaign &campaign = _campaigns[ci];

	std::string current_map;
	TRY {
		if (Config->has("campaign." + campaign.name + ".current-map")) {
			Config->get("campaign." + campaign.name + ".current-map", current_map, std::string());
		}
	} CATCH("init", )

	_maps->clear();

	map_id.clear();
	for(size_t i = 0; i < campaign.maps.size(); ++i) {

		const Campaign::Map &map = campaign.maps[i];
		if (!campaign.visible(map))	
			continue;
		
		_maps->append(map.id);
		map_id.push_back((int)i);
		if (map.id == current_map) {
			_maps->set(i);
			map_pos = map_dst = map.position.convert<float>();
		}
	}
	if (map_id.empty())
		throw_ex(("bug in compaign.xml. no map could be played now"));
}

void CampaignMenu::tick(const float dt) {
	int ci = _active_campaign->get();
	if (ci >= (int)_campaigns.size())
		throw_ex(("no compaigns defined"));
	
	const Campaign &campaign = _campaigns[ci];
	{
		int score;
		Config->get("campaign." + campaign.name + ".score", score, 0);
		_score->set(mrt::formatString("%d", score));
	}

	if (_active_campaign->changed()) {
		_active_campaign->reset();
		init();
	}
	
	if (_maps->changed()) {
		_maps->reset();

		int mi = _maps->get();
		Campaign::Map map = campaign.maps[map_id[mi]];
		Config->set("campaign." + campaign.name + ".current-map", map.id);
		map_dst = map.position.convert<float>();
	}
	
	v2<float> map_vel = map_dst - map_pos;
	if (map_vel.quick_length() < 1) {
		map_pos = map_dst;
	} else {
		map_vel.normalize();
		float dist = math::min(map_dst.distance(map_pos), dt * 200);
		map_pos += map_vel * dist;
	}
}
bool CampaignMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	switch(sym.sym) {
	case SDLK_RETURN: 
		start();
		return true;	
	case SDLK_ESCAPE:
		_parent->back();
		return true;	
	default: 
		return false;
	}
}

const bool CampaignMenu::empty() const {
	return _campaigns.empty();
}

void CampaignMenu::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	int ci = _active_campaign->get();
	//sdlx::Rect clip = surface.getClipRect();
	//surface.setClipRect(map_view);
	surface.copyFrom(*_campaigns[ci].map, sdlx::Rect((int)map_pos.x, (int)map_pos.y, map_view.w, map_view.h), map_view.x, map_view.y);
	//surface.setClipRect(clip);
}

Campaign::Campaign() : minimal_score(0), map(NULL) {}

void Campaign::init() {
	map = NULL;
	parseFile(base + "/campaign.xml");
	/*
	mrt::Directory dir;
	dir.open(base + "/maps");
	std::string fname;

	while(!(fname = dir.read()).empty()) {
		std::string map = fname;
		
		mrt::toLower(map);
		if (map.size() < 5 || map.substr(map.size() - 4) != ".tmx")
			continue;
		map = fname.substr(0, fname.size() - 4);
		LOG_DEBUG(("found map: %s", map.c_str()));
		/ *
		MapScanner m;
		TRY {
			m.scan(path + "/" + fname);
		} CATCH("scanning map", {});
		const std::string &comments = I18n->has("maps/descriptions", map)?I18n->get("maps/descriptions", map): 
			I18n->get("maps/descriptions", "(default)");
		maps.push_back(MapList::value_type(path, map, comments, m.object_restriction, m.game_type, m.slots));
		* /
		maps.push_back(fname);
	}	
	dir.close();
	*/
}

void Campaign::start(const std::string &name, Attrs &attr) {
	if (name == "campaign") {
		if (attr["title"].empty())
			throw_ex(("campaign must have title attr"));
		this->name = attr["title"];
		title = I18n->get("campaign", this->name);
		if (attr["map"].empty())
			throw_ex(("campaign must have map attr"));
		map = ResourceManager->loadSurface(attr["map"]);
	} else if (name == "map") {
		if (attr["id"].empty())
			throw_ex(("map must have id attr"));
		if (attr["position"].empty())
			throw_ex(("map must have position attr"));

		Map map;
		map.id = attr["id"];
		map.visible_if = attr["visible"];
		map.position.fromString(attr["position"]);

		LOG_DEBUG(("map: %s, visible: '%s'", map.id.c_str(), map.visible_if.c_str()));

		maps.push_back(map);
	}
}

void Campaign::end(const std::string &name) {}

void Campaign::getStatus(const std::string &map_id, bool &played, bool &won) const {
	std::string mname = "campaign." + name + ".maps." + map_id + ".win";
	//LOG_DEBUG(("mname: %s", mname.c_str()));
	played = Config->has(mname);
	won = false;
	if (played) {
		Config->get(mname, won, false);
	}
	//LOG_DEBUG(("played: %d, won: %d", played, won));
}

#include <vector>

const bool Campaign::visible(const Map &map) const {
	LOG_DEBUG(("visible('%s')", map.id.c_str()));
	if (minimal_score > 0) {
		int score;
		Config->get("campaign." + name + ".score", score, 0);
		if (minimal_score > score)
			return false;
	}
	if (map.visible_if.empty()) 
		return true;

	LOG_DEBUG(("visible attr : %s", map.visible_if.c_str()));
	
	std::vector<std::string> ors;
	mrt::split(ors, map.visible_if, "|");
	for(size_t i = 0; i < ors.size(); ++i) {
		std::string &token = ors[i];
		mrt::trim(token);
		if (token.empty())
			throw_ex(("invalid syntax ('%s')", map.visible_if.c_str()));
		char op = token[0]; 
		std::string map_id = token.substr(1);
		bool played, won;
		getStatus(map_id, played, won);
		//LOG_DEBUG(("op: '%c', arg: %s, played: %s, won: %s", op, map_id.c_str(), played?"yes":"no", won?"yes":"no"));
		
		switch(op) {
			case '-' : 
				if (played && !won)
					return true;
				break;
			case '+' : 
				if (won)
					return true;
				break;
			case '*': 
				if (played)
					return true;
				break;
			default: 
				throw_ex(("invalid operation: '%c' (%s)", op, map.visible_if.c_str()));
		}
	}
	
	return false;
}

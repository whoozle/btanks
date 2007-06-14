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
#include "shop.h"
#include "button.h"
#include "image_view.h"

void CampaignMenu::start() {
	int ci = _active_campaign->get();
	Campaign &campaign = _campaigns[ci];
	const Campaign::Map &map = campaign.maps[map_id[_maps->get()]];
	LOG_DEBUG(("campaign: %s, map: %s", campaign.name.c_str(), map.id.c_str()));
	//ensure world is created 
	Game->clear();
	GameMonitor->loadMap(&campaign, map.id);
	
	PlayerSlot &slot = PlayerManager->getSlot(0);
	std::string cm;
	Config->get("player.control-method", cm, "keys");
	slot.createControlMethod(cm);

	std::string object, vehicle;
	PlayerManager->getDefaultVehicle(object, vehicle);

	slot.spawnPlayer(object, vehicle);
	PlayerManager->setViewport(0, Window->getSize());
	
	PlayerManager->startServer();	
	_invalidate_me = true;
}

CampaignMenu::CampaignMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _w(w), _h(h), _invalidate_me(false) {
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
	_map_view = new ImageView(map_view.w, map_view.h);
	add(map_view.x, map_view.y, _map_view);
	
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

	_b_shop = new Button("medium", I18n->get("menu", "shop"));
	_b_shop->getSize(bw, bh);

	add(2 * mx, h - bh - 2 * my, _b_shop);
	
	_shop = new Shop(w, h);
	add(0, 0, _shop);
	_shop->hide();
	
	init();
}

void CampaignMenu::init() {
	int ci = _active_campaign->get();
	Campaign &campaign = _campaigns[ci];

	std::string current_map;
	TRY {
		if (Config->has("campaign." + campaign.name + ".current-map")) {
			Config->get("campaign." + campaign.name + ".current-map", current_map, std::string());
		}
	} CATCH("init", )

	_shop->init(&campaign);
	_map_view->init(campaign.map);

	_maps->clear();

	map_id.clear();
	_maps->set(0);
	for(size_t i = 0; i < campaign.maps.size(); ++i) {

		const Campaign::Map &map = campaign.maps[i];
		if (!campaign.visible(map))	
			continue;
		
		_maps->append(map.id);
		map_id.push_back((int)i);
		if (map.id == current_map) {
			_maps->set(i);
			_map_view->position = _map_view->destination = map.position.convert<float>();
		}
	}
	if (map_id.empty())
		throw_ex(("bug in compaign.xml. no map could be played now"));
}

void CampaignMenu::tick(const float dt) {
	BaseMenu::tick(dt);
	if (_invalidate_me) {
		init();
		_invalidate_me = false;
	}
	
	int ci = _active_campaign->get();
	if (ci >= (int)_campaigns.size())
		throw_ex(("no compaigns defined"));
	
	const Campaign &campaign = _campaigns[ci];
	_score->set(mrt::formatString("%d", campaign.getCash()));

	if (_active_campaign->changed()) {
		_active_campaign->reset();
		init();
	}
	
	if (_maps->changed()) {
		_maps->reset();

		int mi = _maps->get();
		if (mi < (int)map_id.size()) {
			Campaign::Map map = campaign.maps[map_id[mi]];
			Config->set("campaign." + campaign.name + ".current-map", map.id);
			_map_view->destination = map.position.convert<float>();
		}
	}
	
	if (_b_shop->changed()) {
		_b_shop->reset();
		_shop->hide(false);
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


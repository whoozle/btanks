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
#include "menu/video_control.h"
#include "menu/video_control_disabled.h"
#include "tmx/map.h"
#include "rt_config.h"
#include "menu/grid.h"
#include "medals.h"
#include "image.h"

void CampaignMenu::start() {
	int ci = _active_campaign->get();
	Campaign &campaign = _campaigns[ci];
	const Campaign::Map &map = campaign.maps[map_id[_maps->get()]];
	if (!campaign.visible(map))
		return;

	RTConfig->game_type = GameTypeCooperative;
	LOG_DEBUG(("campaign: %s, map: %s", campaign.name.c_str(), map.id.c_str()));
	//ensure world is created 
	GameMonitor->startGame(&campaign, map.id);
	
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
		LOG_DEBUG(("campaign[%u]: %s %s", (unsigned)i, files[i].first.c_str(), files[i].second.c_str()));
		Campaign c;
		c.init(files[i].first, files[i].second);
		_campaigns.push_back(c);
		titles.push_back(c.title);
	}

	Box *b = new Box("menu/background_box.png", w - 32, h - 32);
	int bw, bh;
	b->get_size(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
	int mx, my;
	b->getMargins(mx, my);

	int cw, ch;
	_active_campaign = new Chooser("medium", titles);
	_active_campaign->get_size(cw, ch);
	add(w / 2 - cw / 2, my, _active_campaign);

	int panel_w = 256, panel_h = 175;

	int map_base = 3 * my + ch;
	_map_view = new ImageView(w - 4 * mx - panel_w, h - 6 * my);
	add(3 * mx, map_base - 8 , _map_view);
	
	_maps = new ScrollList("menu/background_box_dark.png", "medium", panel_w, h - map_base - 4 * my, 5, 138);
	_maps->setAlign(ScrollList::AlignCenter);
	_maps->setHLColor(255, 0, 0, 0x66);
	int sw, sh;
	_maps->get_size(sw, sh);
	add(w - sw - 2 * mx - mx / 2, map_base, _maps);

	int xbase = 2 * mx, ybase = (h - panel_h - 5 * my);
	add(xbase, ybase, score_box = b = new Box("menu/background_box_dark.png", panel_w, panel_h));
	b->getMargins(mx, my);
	
	Grid *grid = new Grid(2, 6);
	score_grid = grid;
		
	grid->set(0, 0, new Label("medium", I18n->get("menu", "score")));
	grid->set(0, 1, _score = new Label("medium", "0"));

	grid->set(1, 0, new Label("medium", I18n->get("menu", "last-score")));
	grid->set(1, 1, _last_score = new Label("medium", "0"));

	grid->set(2, 0, new Label("medium", I18n->get("menu", "best-score")));
	grid->set(2, 1, _best_score = new Label("medium", "0"));

	grid->set(3, 0, new Label("medium", I18n->get("menu", "last-time")));
	grid->set(3, 1, _last_time = new Label("medium", "-:--:--"));

	grid->set(4, 0, new Label("medium", I18n->get("menu", "best-time")));
	grid->set(4, 1, _best_time = new Label("medium", "-:--:--"));

	std::vector<std::string> levels;
	levels.push_back(I18n->get("menu/difficulty", "easy"));
	levels.push_back(I18n->get("menu/difficulty", "normal"));
	levels.push_back(I18n->get("menu/difficulty", "hard"));
	levels.push_back(I18n->get("menu/difficulty", "nightmare"));
	
	grid->set(5, 0, _c_difficulty = new Chooser("medium", levels), Grid::Middle);
	grid->set_span(5, 0, 1, 2);
	grid->set_spacing(2);
	
	add(xbase + mx, ybase + my, grid);

	grid->recalculate();
	grid->get_size(bw, bh);
	b->init("menu/background_box_dark.png", bw + 2 * mx, bh + my);

	_b_shop = new Button("medium", I18n->get("menu", "shop"));
	_b_shop->get_size(bw, bh);

	ybase = h - bh - 2 * my;
	add(2 * mx, ybase, _b_shop);

	_b_medals = new Button("medium", I18n->get("menu", "medals"));
	add(3 * mx + bw, ybase, _b_medals);
	
	_shop = new Shop(w, h);
	add(0, 0, _shop);
	_shop->hide();
	
	medals = new Medals(w, h);
	medals->get_size(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, medals);
	medals->hide();
	
	init();
}

void CampaignMenu::init() {
	_c_difficulty->set(1);

	int ci = _active_campaign->get();
	Campaign &campaign = _campaigns[ci];

	std::string current_map;
	TRY {
		if (Config->has("campaign." + campaign.name + ".current-map")) {
			Config->get("campaign." + campaign.name + ".current-map", current_map, std::string());
		}
		int diff;
		Config->get("campaign." + campaign.name + ".difficulty", diff, 1);
		LOG_DEBUG(("difficulty = %d", diff));

		_c_difficulty->set(diff);
	} CATCH("init", )

	_shop->init(&campaign);
	_map_view->init(campaign.map);

	_maps->clear();

	map_id.clear();
	for(size_t i = 0; i < campaign.maps.size(); ++i) {

		const Campaign::Map &map = campaign.maps[i];
		Control *c = NULL;
		TRY {
		c = campaign.visible(map)? 
				static_cast<Control *>(new VideoControl(campaign.base, map.id)): 
				static_cast<Control *>(new DisabledVideoControl(campaign.base, map.id));
		} CATCH("init", continue; );
		
		_maps->append(c);
		map_id.push_back((int)i);
		if (map.id == current_map) {
			_maps->set(_maps->size() - 1);
			_map_view->set_position(map.position.convert<float>());
		}
	}
	if (map_id.empty())
		throw_ex(("bug in compaign.xml. no map could be played now"));
}

const std::string CampaignMenu::convert_time(const float t) {
	int s = (int)t;
	int m = s / 60;
	int h = m / 60;
	s %= 60;
	m %= 60;
	return mrt::format_string("%d:%02d:%02d", h, m, s);
}


void CampaignMenu::update_time(Label *l, const std::string &name) {
	float t = 0;
	if (Config->has(name))
		Config->get(name, t, 0);
	l->set(t > 0? convert_time(t): std::string("-:--:--"));
}
void CampaignMenu::update_score(Label *l, const std::string &name) {
	int score = 0;
	if (Config->has(name))
		Config->get(name, score, 0);
	l->set(mrt::format_string("%d", score));
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
	_score->set(mrt::format_string("%d", campaign.getCash()));
	medals->set(&campaign);

	if (_active_campaign->changed()) {
		_active_campaign->reset();
		init();
	}
	
	if (_maps->changed()) {
		_maps->reset();

		update_map();
	}
	
	if (Map->loaded() && !_b_shop->hidden())
		_b_shop->hide();

	if (!Map->loaded() && _b_shop->hidden())
		_b_shop->hide(false);

	if (_b_medals->changed()) {
		_b_medals->reset();
		medals->hide(false);
	}
	
	if (_b_shop->changed()) {
		_b_shop->reset();
		_shop->hide(false);
	}
	if (_c_difficulty->changed()) {
		_c_difficulty->reset();
		Config->set("campaign." + campaign.name + ".difficulty", _c_difficulty->get());
	}
}

void CampaignMenu::update_map() {
	int ci = _active_campaign->get();
	if (ci >= (int)_campaigns.size())
		throw_ex(("no compaigns defined"));
	
	const Campaign &campaign = _campaigns[ci];
	int mi = _maps->get();
	if (mi < 0 || mi >= (int)map_id.size())
		return;
	
	Campaign::Map map = campaign.maps[map_id[mi]];
	Config->set(std::string("campaign.") + campaign.name + ".current-map", map.id);
	_map_view->setOverlay(map.map_frame, map.position);
	_map_view->setDestination(map.position.convert<float>());

	std::string mname = std::string("campaign.") + campaign.name + ".maps." + map.id;
	
	update_time(_last_time, mname + ".last-time");
	update_time(_best_time, mname + ".best-time");
	update_score(_last_score, mname + ".last-score");
	update_score(_best_score, mname + ".maximum-score");
	
	score_grid->recalculate();

	int bw, bh, mx, my;
	score_grid->get_size(bw, bh);
	
	score_box->getMargins(mx, my);
	score_box->init("menu/background_box_dark.png", bw + 2 * mx, bh + my);
	score_box->get_size(bw, bh);
	
	int medalx, medaly;
	score_box->get_base(medalx, medaly);
	medalx += bw - mx;
	medaly += bh - my / 2;
	
	for(size_t i = 0; i < medal_icons.size(); ++i) {
		remove(medal_icons[i]);
	}
	medal_icons.clear();
	
	for(size_t i = 0; i < campaign.medals.size(); ++i) {
		const Campaign::Medal & medal = campaign.medals[i];
		if (medal.icon == NULL || !map.got_medal(campaign, medal))
			continue;
		Image *image = new Image(medal.icon);
		medalx -= medal.icon->get_width();
		add(medalx, medaly - medal.icon->get_height(), image, score_grid);
		medal_icons.push_back(image);
	}
}

bool CampaignMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	switch(sym.sym) {
	case SDLK_m: 
		medals->hide(false);
		return true;
	case SDLK_KP_ENTER:
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


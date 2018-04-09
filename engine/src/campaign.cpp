#include "campaign.h"
#include "i18n.h"
#include "config.h"
#include "resource_manager.h"
#include "mrt/chunk.h"
#include "finder.h"
#include "sdlx/surface.h"
#include "game.h"
#include "window.h"

void Campaign::start(const std::string &name, Attrs &attr) {
	if (_preparse && name == "logo") {
		std::string file = attr.get("file", std::string());
		std::string bgcolor = attr.get("background", "000000");
		float duration = attr.get("duration", 3.0f);

		unsigned r = 0, g = 0, b = 0;
		if (bgcolor.size() == 6) {
			sscanf(bgcolor.c_str(), "%02x%02x%02x", &r, &g, &b);
		} else if (bgcolor.size() == 3) {
			sscanf(bgcolor.c_str(), "%1x%1x%1x", &r, &g, &b);
			r *= 16; g *= 16; b *= 16;
		} else {
			LOG_ERROR(("logo %s has invalid background %s", file.c_str(), bgcolor.c_str()));
			bgcolor.clear();
		}
		Uint32 color = Window->get_surface().map_rgb(r, g, b);

		sdlx::Surface * s = NULL;
		try {
			mrt::Chunk data;
			std::string tname = "tiles/" + file;
			Finder->load(data, tname);

			s = new sdlx::Surface;
			s->load_image(data);
			LOG_DEBUG(("adding logo '%s' for %g seconds", file.c_str(), duration));
			Game->add_logo(s, duration, color, true);
		} CATCH("parsing logo entry", delete s)
	} else if (_preparse && name == "campaign") {
		std::string donation = attr.get("donations", "true");
		disable_donations = !donation.empty() && donation[0] == 'f';
		std::string network = attr.get("network", "true");
		disable_network = !network.empty() && network[0] == 'f';
	}

	if (_preparse)
		return;

	if (name == "campaign") {
		if (attr["name"].empty())
			throw_ex(("campaign must have title attr"));
		
		this->name = attr["name"];
		title = I18n->get("campaign", this->name);
		if (attr["map"].empty())
			throw_ex(("campaign must have map attr"));
		map = ResourceManager->load_surface(attr["map"]);
	} else if (name == "map") {
		if (attr["id"].empty())
			throw_ex(("map must have id attr"));
		if (attr["position"].empty())
			throw_ex(("map must have position attr"));

		Map map;
		
		map.id = attr["id"];
		map.visible_if = attr["visible"];
		map.position.fromString(attr["position"]);
		map.map_frame = NULL;
		
		map.no_medals = !attr.get("medals", true);
		map.secret = attr.get("secret", false);

		map.time = attr.get("time", 0);
		map.score = attr.get("score", 0);
		
		TRY {
			map.map_frame = ResourceManager->load_surface("../maps/" + map.id + "_frame.png");
		} CATCH("loading map frame", {});

		LOG_DEBUG(("map: %s, visible: '%s'", map.id.c_str(), map.visible_if.c_str()));

		maps.push_back(map);
	} else if (name == "wares") {
		if (_wares_section)
			throw_ex(("recursive wares section is not allowed"));
		_wares_section = true;
	} else if (name == "item") {
		if (!_wares_section)
			throw_ex(("item must be withing <wares /> tags"));
		
		wares.push_back(ShopItem());
		ShopItem & item = wares.back();
		item.type = attr["type"];
		item.name = attr["name"];
		item.price = attr["price"].empty()?0:atoi(attr["price"].c_str());
		item.max_amount = attr["maximum-amount"].empty()?0:atoi(attr["maximum-amount"].c_str());
		item.dir_speed = attr["cycle-directions"].empty()?0.0f: (float)atof(attr["cycle-directions"].c_str());
		
		item.object = attr["object"];
		item.animation = attr["animation"];
		item.pose = attr["preview-pose"];

		std::string kname = get_config_prefix() + ".wares." + item.name + ".amount";
		//LOG_DEBUG(("querying %s", kname.c_str()));
		if (Config->has(kname)) {
			int am;
			Config->get(kname, am, 0);
			item.amount = am;
		}
		item.validate();

	} else if (name == "medal") {
		Medal medal;
		medal.id = attr.get("id", std::string());
		medal.tile = attr.get("tile", std::string());
		if (medal.id.empty() || medal.tile.empty())
			throw_ex(("medal id and tile must not be empty"));
		std::string icon = attr.get("icon", std::string());
		if (!icon.empty())
			medal.icon = ResourceManager->load_surface(icon);
		medals.push_back(medal);
	} 
}

std::string Campaign::get_config_prefix() const {
	std::string profile;
	Config->get("engine.profile", profile, std::string());
	if (profile.empty())
		throw_ex(("empty profile"));
	
	return "campaign." + profile + "." + this->name;
}


void Campaign::end(const std::string &name) {
	if (name == "wares") {
		LOG_DEBUG(("wares section parsed... %u wares in store.", (unsigned)wares.size()));
		_wares_section = false;
	}
}

void Campaign::getStatus(const std::string &map_id, bool &played, bool &won) const {
	std::string mname = get_config_prefix() + ".maps." + map_id + ".win";
	//LOG_DEBUG(("mname: %s", mname.c_str()));
	played = Config->has(mname);
	won = false;
	if (played) {
		Config->get(mname, won, false);
	}
	//LOG_DEBUG(("played: %d, won: %d", played, won));
}

#include <vector>

const std::pair<bool, bool> Campaign::visible(const Map &map) const {
	//LOG_DEBUG(("visible('%s')", map.id.c_str()));
	if (minimal_score > 0) {
		if (minimal_score > getCash())
			return std::pair<bool, bool>(false, false);
	}
	if (map.visible_if.empty()) 
		return std::pair<bool, bool>(true, false);

	//LOG_DEBUG(("visible attr : %s", map.visible_if.c_str()));
	
	std::vector<std::string> ors;
	mrt::split(ors, map.visible_if, "|");

	bool opened = false;
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
					opened = true;
				break;
			case '+' : 
				if (won)
					opened = true;
				break;
			case '*': 
				if (played)
					opened = true;
				break;
			default: 
				throw_ex(("invalid operation: '%c' (%s)", op, map.visible_if.c_str()));
		}
		if (opened)
			break;
	}
	
	if (opened) {
		bool just_opened = !map.last_opened;
		map.last_opened = true;
		return std::pair<bool, bool>(opened, just_opened);
	}
	
	return std::pair<bool, bool>(false, false);
}

Campaign::Campaign() : minimal_score(0), map(NULL), disable_donations(false), disable_network(false), _wares_section(false) {}

#include "game_monitor.h"
#include "finder.h"
#include "mrt/scoped_ptr.h"
#include "mrt/base_file.h"

void Campaign::init(const std::string &base, const std::string &filename, bool preparse) {
	this->base = base;
	map = NULL;
	_wares_section = false;
	_preparse = preparse;
	scoped_ptr<mrt::BaseFile> ptr(Finder->get_file(filename, "rt"));
	parse_file(*ptr);
	for(size_t i = 0; i < maps.size(); ++i) {
		GameMonitor->useInCampaign(base, maps[i].id);
	}
}

const int Campaign::getCash() const {
	int cash;
	Config->get(get_config_prefix() + ".score", cash, 0);
	return cash;
}

const bool Campaign::buy(ShopItem &item) const {
	int cash = getCash();
	if (cash < item.price)
		return false;

	if (item.amount >= item.max_amount)
		return false;

	LOG_DEBUG(("buying item %s...", item.name.c_str()));
	cash -= item.price;
	++item.amount;

	std::string prefix = get_config_prefix();

	Config->set(prefix + ".score", cash);
	Config->set(prefix + ".wares." + item.name + ".amount", item.amount);
	return true;
}

const bool Campaign::sell(ShopItem &item) const {
	if (item.amount <= 0)
		return false;

	int cash = getCash();
		
	LOG_DEBUG(("selling item %s...", item.name.c_str()));
	cash += item.price * 4 / 5;
	--item.amount;

	std::string prefix = get_config_prefix();

	Config->set(prefix + ".score", cash);
	Config->set(prefix + ".wares." + item.name + ".amount", item.amount);
	return true;
}


void Campaign::ShopItem::validate() {
	if (name.empty())
		throw_ex(("shop item does not have a name"));
	if (price == 0)
		throw_ex(("shop item %s does not have a price", name.c_str()));
	if (amount > max_amount)
		amount = max_amount;
}

const Campaign::ShopItem * Campaign::find(const std::string &name) const {
	for(std::vector<ShopItem>::const_iterator i = wares.begin(); i != wares.end(); ++i) {
		if (i->name == name) 
			return & *i;
	}
	return NULL;
}

void Campaign::clearBonuses() {
	std::string prefix = get_config_prefix();

	for(std::vector<ShopItem>::iterator i = wares.begin(); i != wares.end(); ++i) {
		i->amount = 0;
		std::string kname = prefix + ".wares." + i->name + ".amount";
		if (Config->has(kname)) {
			Config->remove(kname);
		}
	}
}

bool Campaign::Map::got_medal(const Campaign &campaign, const Medal &medal) const {
	if (no_medals)
		return false;
	
	std::string prefix = campaign.get_config_prefix();

	if (medal.id == "elimination") {
		if (score <= 0)
			return false;

		std::string mname = prefix + ".maps." + id + ".maximum-score";
		if (!Config->has(mname))
			return false;

		int bs;
		Config->get(mname, bs, 0);
		return bs >= score;
	} else if (medal.id == "speedrun") {
		if (time <= 0)
			return false;

		std::string mname = prefix + ".maps." + id + ".best-time";
		if (!Config->has(mname))
			return false;
			
		float bt;
		Config->get(mname, bt, 3600);
		return bt <= time;
	} else if (medal.id == "secrets") {
		return secret && campaign.visible(*this).first;
	}

	return false;
}

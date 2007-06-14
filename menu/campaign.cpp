#include "campaign.h"
#include "i18n.h"
#include "config.h"
#include "resource_manager.h"

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
	} else if (name == "wares") {
		if (_wares_section)
			throw_ex(("recursive wares section is not allowed"));
		_wares_section = true;
	} else if (name == "item") {
		wares.push_back(ShopItem());
		ShopItem & item = wares.back();
		item.type = attr["type"];
		item.name = attr["name"];
		item.price = attr["price"].empty()?0:atoi(attr["price"].c_str());
		item.max_amount = attr["maximum-amount"].empty()?0:atoi(attr["maximum-amount"].c_str());
		item.validate();
	}
}

void Campaign::end(const std::string &name) {
	if (name == "wares") {
		LOG_DEBUG(("wares section parsed... %u wares in store.", (unsigned)wares.size()));
		_wares_section = false;
	}
}

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
		if (minimal_score > getCash())
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

Campaign::Campaign() : minimal_score(0), map(NULL), _wares_section(false) {}

void Campaign::init() {
	map = NULL;
	_wares_section = false;
	parseFile(base + "/campaign.xml");
}

const int Campaign::getCash() const {
	int cash;
	Config->get("campaign." + name + ".score", cash, 0);
	return cash;
}

const int Campaign::getAmount(const ShopItem &item) const {
	std::string kname = "campaign." + name + ".wares." + item.name + ".amount";
	if (!Config->has(kname))
		return 0;
	
	int a;
	Config->get(kname, a, 0);
	return a;
}

const bool Campaign::buy(const ShopItem &item) const {
	int cash = getCash();
	if (cash < item.price)
		return false;

	int am = getAmount(item);
	if (am >= item.max_amount)
		return false;

	LOG_DEBUG(("buying item %s...", item.name.c_str()));
	cash -= item.price;
	++am;

	Config->set("campaign." + name + ".score", cash);
	Config->set("campaign." + name + ".wares." + item.name + ".amount", am);
	return true;
}

const bool Campaign::sell(const ShopItem &item) const {
	int am = getAmount(item);
	if (am <= 0)
		return false;

	int cash = getCash();
		
	LOG_DEBUG(("selling item %s...", item.name.c_str()));
	cash += item.price * 4 / 5;
	--am;

	Config->set("campaign." + name + ".score", cash);
	Config->set("campaign." + name + ".wares." + item.name + ".amount", am);
	return true;
}


void Campaign::ShopItem::validate() {
	if (name.empty())
		throw_ex(("shop item does not have a name"));
	if (price == 0)
		throw_ex(("shop item %s does not have a price", name.c_str()));
}

const Campaign::ShopItem * Campaign::find(const std::string &name) const {
	for(std::vector<ShopItem>::const_iterator i = wares.begin(); i != wares.end(); ++i) {
		if (i->name == name) 
			return & *i;
	}
	return NULL;
}

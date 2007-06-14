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

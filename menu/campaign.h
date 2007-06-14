#ifndef BTANKS_CAMPAIGN_H__
#define BTANKS_CAMPAIGN_H__

#include "mrt/xml.h"
#include "math/v2.h"

namespace sdlx {
	class Surface;
}

struct Campaign : protected mrt::XMLParser {
	Campaign();
	std::string base, name, title;
	int minimal_score;
	
	const sdlx::Surface *map;
	
	struct Map {
		std::string id;
		std::string visible_if;
		v2<int> position;
	};
	
	struct ShopItem {
		std::string type, name;
		int price, max_amount;
		void validate();
		ShopItem() : price(0), max_amount(0) {}
	};
	
	std::vector<Map> maps;
	std::vector<ShopItem> wares;
	
	void init();
	const bool visible(const Map &map_id) const;
	const int getCash() const;
	const int getAmount(const ShopItem &item) const;
	const bool buy(const ShopItem &item) const;
	
protected: 
	void getStatus(const std::string &map_id, bool &played, bool &won) const;

	void start(const std::string &name, Attrs &attr);
	void end(const std::string &name);
private: 
	bool _wares_section;
};

#endif

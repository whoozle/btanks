#include "shop.h"
#include "config.h"

Shop::Shop(const int w, const int h) : _cash(0) {
	
}

void Shop::init(const std::string &campaign) {
	_campaign = campaign;
	_prefix = "campaign." + campaign + ".";
	Config->get(_prefix + "score", _cash, 0);
	LOG_DEBUG(("selecting campaign %s, cash: %d", campaign.c_str(), _cash));
}

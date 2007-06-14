#include "shop.h"
#include "config.h"
#include "box.h"
#include "campaign.h"
#include "shop_item.h"
#include "scroll_list.h"

Shop::Shop(const int w, const int h)  {
	Box * b = new Box("menu/background_box.png", w - 32, h - 32);
	int mx, my, bw, bh;
	b->getMargins(mx, my);
	b->getSize(bw, bh);
	int xbase = (w - bw) / 2, ybase = (h - bh) / 2;
	add(xbase, ybase, b);
	
	int sw, sh;
	_wares = new ScrollList("menu/background_box.png", "medium", w - 4 * mx, h - 4 * my, 10);
	_wares->initBG("menu/background_box.png", "menu/highlight_big.png",  w - 4 * mx, h - 4 * my);
	_wares->getSize(sw, sh);
	add(xbase + mx, ybase + my, _wares);
}

void Shop::init(const Campaign &campaign) {
	_campaign = campaign.name;
	_prefix = "campaign." + _campaign + ".";
	LOG_DEBUG(("selecting campaign %s, cash: %d", _campaign.c_str(), campaign.getCash()));

	int w, h;
	getSize(w, h);

	_wares->clear();
	for(size_t i = 0; i < campaign.wares.size(); ++i) {
		_wares->append(new ShopItem(campaign, campaign.wares[i], w));
	}
}

bool Shop::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
		hide();
		return true;
	
	default: 
		return true;
	}
}

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
	_wares = new ScrollList("menu/background_box.png", "medium", w - 4 * mx, h - 4 * my, 20);
	_wares->initBG("menu/background_box.png", "menu/highlight_big.png",  w - 4 * mx, h - 4 * my);
	_wares->getSize(sw, sh);
	add(xbase + mx, ybase + my, _wares);
}

void Shop::init(Campaign *campaign) {
	_campaign = campaign;
	if (_campaign == NULL)
		return;
	
	_prefix = "campaign." + campaign->name + ".";
	LOG_DEBUG(("selecting campaign %s, cash: %d", campaign->name.c_str(), campaign->getCash()));

	int w, h;
	getSize(w, h);

	_wares->clear();
	for(size_t i = 0; i < campaign->wares.size(); ++i) {
		_wares->append(new ShopItem(*campaign, campaign->wares[i], w));
	}
}

void Shop::revalidate() {
	if (_campaign == NULL)
		return;
	
	size_t n = _campaign->wares.size();
	assert((int)n == _wares->size());
	size_t c = _wares->get();
	for(size_t i = 0; i < n; ++i) {
		Control *ctrl = _wares->getItem(i);
		ShopItem *s = dynamic_cast<ShopItem *>(ctrl);
		if (s != NULL) {
			s->revalidate(*_campaign, _campaign->wares[i], i == c);
		}
	}
}

void Shop::tick(const float dt) {
	if (_wares->changed()) {
		_wares->reset();
		revalidate();
	}
}

bool Shop::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_SPACE: 
	case SDLK_LCTRL: 
	case SDLK_RETURN: 
		{
			if (_campaign == NULL)
				return true;
		
			int i = _wares->get();
			if (i >= (int)_campaign->wares.size()) 
				return true;
			Campaign::ShopItem &item = _campaign->wares[i];
			_campaign->buy(item);
			revalidate();
		} return true;
	
	case SDLK_ESCAPE: 
		hide();
		return true;
	
	default: 
		return true;
	}
}

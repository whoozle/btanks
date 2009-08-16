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
	b->get_size(bw, bh);
	int xbase = (w - bw) / 2, ybase = (h - bh) / 2;
	add(xbase, ybase, b);
	
	int sw, sh;
	_wares = new ScrollList("menu/background_box.png", "medium", w - 4 * mx, h - 4 * my, 20);
	_wares->initBG("menu/background_box.png", w - 4 * mx, h - 4 * my, 36);
	_wares->get_size(sw, sh);
	add(xbase + mx, ybase + my, _wares);
}

void Shop::init(Campaign *campaign) {
	_campaign = campaign;
	if (_campaign == NULL)
		return;
	
	std::string profile;
	Config->get("engine.profile", profile, std::string());
	if (profile.empty())
		throw_ex(("empty profile"));

	_prefix = "campaign." + profile + "." + campaign->name + ".";
	LOG_DEBUG(("selecting campaign %s, cash: %d", campaign->name.c_str(), campaign->getCash()));

	int w, h;
	get_size(w, h);

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
	bool do_revalidate = false;
	Container::tick(dt);

	int i = _wares->get();
	
	if (_campaign != NULL && i < (int)_campaign->wares.size()) {
		Campaign::ShopItem &item = _campaign->wares[i];
				
		size_t n = _campaign->wares.size();
		assert((int)n == _wares->size());
		for(size_t i = 0; i < n; ++i) {
			Control *ctrl = _wares->getItem(i);
			ShopItem *s = dynamic_cast<ShopItem *>(ctrl);
			if (s == NULL || !s->changed())
				continue;

			s->reset();
		
			if (s->wasSold()) 
				_campaign->sell(item);
			else
				_campaign->buy(item);
			do_revalidate = true;
		}
	}
	
	if (do_revalidate || _wares->changed()) {
		_wares->reset();
		revalidate();
	}
}

bool Shop::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	bool buy = true;

	switch(sym.sym) {
	case SDLK_MINUS:
	case SDLK_UNDERSCORE:
	case SDLK_KP_MINUS:
		buy = false;
	
	case SDLK_SPACE: 
	case SDLK_LCTRL: 
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
	case SDLK_KP_PLUS:
	case SDLK_PLUS:
	case SDLK_EQUALS:
		{
			if (_campaign == NULL)
				return true;
		
			int i = _wares->get();
			if (i >= (int)_campaign->wares.size()) 
				return true;
			Campaign::ShopItem &item = _campaign->wares[i];
			if (buy) 
				_campaign->buy(item);
			else
				_campaign->sell(item);
				
			revalidate();
		} return true;
	
	case SDLK_ESCAPE: 
		hide();
		return true;
	
	default: 
		return true;
	}
}

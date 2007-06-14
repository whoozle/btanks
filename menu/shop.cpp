#include "shop.h"
#include "config.h"
#include "box.h"
#include "campaign.h"

Shop::Shop(const int w, const int h)  {
	Box * b = new Box("menu/background_box.png", w - 32, h - 32);
	int mx, my, bw, bh;
	b->getMargins(mx, my);
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
}

const int Shop::getCash() const {
	int cash;
	Config->get(_prefix + "score", cash, 0);
	return cash;
}

void Shop::init(const Campaign &campaign) {
	_campaign = campaign.name;
	_prefix = "campaign." + _campaign + ".";
	LOG_DEBUG(("selecting campaign %s, cash: %d", _campaign.c_str(), getCash()));
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

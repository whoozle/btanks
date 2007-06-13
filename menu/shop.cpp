#include "shop.h"
#include "config.h"
#include "box.h"

Shop::Shop(const int w, const int h) : _cash(0) {
	Box * b = new Box("menu/background_box.png", w - 32, h - 32);
	int mx, my, bw, bh;
	b->getMargins(mx, my);
	b->getSize(bw, bh);
	add((w - bw) / 2, (h - bh) / 2, b);
}

void Shop::init(const std::string &campaign) {
	_campaign = campaign;
	_prefix = "campaign." + campaign + ".";
	Config->get(_prefix + "score", _cash, 0);
	LOG_DEBUG(("selecting campaign %s, cash: %d", campaign.c_str(), _cash));
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

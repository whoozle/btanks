#include "join_server_menu.h"
#include "button.h"
#include "mrt/logger.h"
#include "menu.h"
#include "menu_config.h"

JoinServerMenu::JoinServerMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	_back = new Button("big", "BACK");
	int bw, bh;
	_back->getSize(bw, bh);
	add(sdlx::Rect(64, h - 96, bw, bh), _back);
	
	_scan = new Button("big", "SCAN");
	_scan->getSize(bw, bh);
	add(sdlx::Rect(96 + bw, h - 96, bw, bh), _scan);
	
	_join = new Button("big", "JOIN");
	_join->getSize(bw, bh);
	add(sdlx::Rect(w - 64 - bw, h - 96, bw, bh), _join);
}

void JoinServerMenu::join() {
	LOG_DEBUG(("join requested"));
}

void JoinServerMenu::tick(const float dt) {
	Container::tick(dt);
	if (_back->changed()) {
		LOG_DEBUG(("[back] clicked"));
		_back->reset();
		_parent->back();
		MenuConfig->save();
	}
	if (_scan->changed()) {
		_scan->reset();
		LOG_DEBUG(("scan"));
	}
	
	if (_join->changed()) {
		_join->reset();
		join();
	}

}

bool JoinServerMenu::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_RETURN:
		join();
		return true;

	case SDLK_ESCAPE: 
		_parent->back();
		MenuConfig->save();
		return true;

	default: 
		return Container::onKey(sym);
	}
	return false;
}

JoinServerMenu::~JoinServerMenu() {}

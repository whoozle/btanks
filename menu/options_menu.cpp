#include "options_menu.h"
#include "button.h"
#include "menu.h"

OptionsMenu::OptionsMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	_background.init("menu/background_box.png", w - 100, h - 100);
	int bw, bh;

	_b_ok = new Button("big", "OK");
	_b_ok->getSize(bw, bh);

	int yb = 120;
	add(sdlx::Rect(_background.w / 4, h - yb, bw, bh), _b_ok);
	
	_b_back = new Button("big", "BACK");
	_b_back->getSize(bw, bh);
	add(sdlx::Rect(3 * _background.w / 4, h - yb, bw, bh), _b_back);
	
	_bx = ( w - _background.w ) / 2;
	_by = ( h - _background.h ) / 2;
}

void OptionsMenu::getSize(int &w, int &h) {
	w = _background.w;
	h = _background.h;
}

void OptionsMenu::reload() {
	LOG_DEBUG(("reloading options..."));
}

void OptionsMenu::save() {
	LOG_DEBUG(("saving options..."));
}


void OptionsMenu::tick(const float dt) {
	if (_b_ok->changed()) {
		_b_ok->reset();
		_parent->back();
		save();
	} else if (_b_back->changed()) {
		_b_back->reset();
		_parent->back();
		reload();
	}
}

void OptionsMenu::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, _bx, _by);
	Container::render(surface, x, y);	
}

bool OptionsMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym)) 
		return true;

	switch(sym.sym) {

	case SDLK_RETURN:
		_parent->back();
		save();
		return true;

	case SDLK_ESCAPE: 
		_parent->back();
		reload();
		return true;

	default: ;
	}
	return false;
}

OptionsMenu::~OptionsMenu() {}


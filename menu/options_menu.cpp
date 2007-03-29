#include "options_menu.h"
#include "button.h"
#include "menu.h"
#include "control_picker.h"
#include "i18n.h"
#include "label.h"
#include "slider.h"
#include "config.h"


OptionsMenu::OptionsMenu(MainMenu *parent, const int w, const int h) : _parent(parent) {
	_background.init("menu/background_box.png", w - 100, h - 100);
	int bw, bh;

	int mx, my;
	_background.getMargins(mx, my);
	_bx = ( w - _background.w ) / 2;
	_by = ( h - _background.h ) / 2;

	_b_back = new Button("big", I18n->get("menu", "back"));
	_b_back->getSize(bw, bh);

	int yb = 200;
	add(_bx + mx + _background.w / 4 - bw / 2, _by + my +h - yb, _b_back);
	
	_b_ok = new Button("big", I18n->get("menu", "ok"));
	_b_ok->getSize(bw, bh);
	add(_by + my + 3 * _background.w / 4 - bw / 2, _by + my + h - yb, _b_ok);
	
	int width = _background.w - 2 * mx;

	int sw, sh;
	int yp = my * 2 + _by;
	
	sp = new ControlPicker(width, "big", I18n->get("menu", "single-player"), "player.control-method", std::string());
	sp->getSize(sw, sh);
	add(_bx + mx, yp, sp);
	yp += sh + 10;

	sp1 = new ControlPicker(width, "big", I18n->get("menu", "split-player-1"), "player.control-method-1", "split");
	sp1->getSize(sw, sh);
	add(_bx + mx, yp, sp1);
	yp += sh + 10;

	sp2 = new ControlPicker(width, "big", I18n->get("menu", "split-player-2"), "player.control-method-2", "split");
	sp2->getSize(sw, sh);
	add(_bx + mx, yp, sp2);
	yp += sh + 10;
	
	float volume;
	Config->get("sound.volume.music", volume, 1);
	
	Label *l = new Label("medium", I18n->get("menu", "music-volume"));
	Slider *s = new Slider(volume);

	add(_bx + mx, yp, l);
	l->getSize(sw, sh);
	{
		int w, h;
		s->getSize(w, h);
		add(_bx + _background.w / 2, yp + (sh - h) / 2, s);
		if (h > sh) 
			sh = h;
	}
	
	yp += sh + 10;


	Config->get("sound.volume.music", volume, 1);
	
	l = new Label("medium", I18n->get("menu", "fx-volume"));
	s = new Slider(volume);
	add(_bx + mx, yp, l);
	l->getSize(sw, sh);
	{
		int w, h;
		s->getSize(w, h);
		add(_bx + _background.w / 2, yp + (sh - h) / 2, s);
		if (h > sh) 
			sh = h;
	}


	yp += sh + 10;
	
	reload();
}

void OptionsMenu::getSize(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

void OptionsMenu::reload() {
	LOG_DEBUG(("reloading options..."));
	sp->reload();
	sp1->reload();
	sp2->reload();
}

void OptionsMenu::save() {
	LOG_DEBUG(("saving options..."));
	sp->save();
	sp1->save();
	sp2->save();
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


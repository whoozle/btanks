
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "options_menu.h"
#include "button.h"
#include "menu.h"
#include "control_picker.h"
#include "i18n.h"
#include "label.h"
#include "slider.h"
#include "config.h"
#include "sound/mixer.h"
#include "resource_manager.h"
#include "object.h"
#include "redefine_keys.h"
#include "gamepad_setup.h"

OptionsMenu::OptionsMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _shoot(0.5f, false) {
	Mixer->loadSample("shot.ogg");
	_shooter = ResourceManager->createObject("outline");
	
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
	
	
	_b_redefine = new Button("medium_dark", I18n->get("menu", "redefine-keys"));
	_b_redefine->getSize(sw, sh);
	add((w - sw) / 2, yp + 6, _b_redefine);

	yp += sh + 20;
	//volume controls 
	
	float volume;
	Config->get("engine.sound.volume.music", volume, 1);
	
	Label *l = new Label("medium", I18n->get("menu", "music-volume"));
	Slider *s = _music = new Slider(volume);

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


	Config->get("engine.sound.volume.fx", volume, 1);
	
	l = new Label("medium", I18n->get("menu", "fx-volume"));
	s = _fx = new Slider(volume);
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
	
	_keys = new RedefineKeys;
	_keys->getSize(sw, sh);
	add((w - sw) / 2, (h - sh) / 2, _keys);
	_keys->hide();
	
	_gamepad = new GamepadSetup(w, h);
	_gamepad->getSize(sw, sh);
	add((w - sw) / 2, (h - sh) / 2, _gamepad);
	_gamepad->hide();
	
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
	
	float volume;
	Config->get("engine.sound.volume.music", volume, 1);
	_music->set(volume);

	Config->get("engine.sound.volume.fx", volume, 1);
	_fx->set(volume);
	
	_keys->reload();
}

void OptionsMenu::save() {
	LOG_DEBUG(("saving options..."));
	sp->save();
	sp1->save();
	sp2->save();
	
	Config->set("engine.sound.volume.fx", _fx->get());
	Config->set("engine.sound.volume.music", _music->get());
}


void OptionsMenu::tick(const float dt) {
	if (_fx->changed() || _fx->tracking()) {
		_fx->reset();
		Mixer->setFXVolume(_fx->get());
		if (_shoot.tick(dt)) {
			Mixer->setListener(v3<float>::empty, v3<float>::empty);
			Mixer->playSample(_shooter, "shot.ogg", false);
			_shoot.reset();
		}
	}
	if (_music->changed()) {
		_music->reset();
		Mixer->setMusicVolume(_music->get());
	}
	if (_b_ok->changed()) {
		_b_ok->reset();
		_parent->back();
		save();
	} else if (_b_back->changed()) {
		_b_back->reset();
		_parent->back();
		reload();
	}
	if (_b_redefine->changed()) {
		_b_redefine->reset();
		_keys->hide(false);
	}
	Container::tick(dt);
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

	case SDLK_j: 
	case SDLK_g: 
		if (_keys->hidden())
			_gamepad->hide(false);
		return true;

	case SDLK_r: 
		if (_gamepad->hidden())
			_keys->hide(false);
		return true;

	default: ;
	}
	return false;
}

OptionsMenu::~OptionsMenu() {
	if (_shooter)
		delete _shooter;
}


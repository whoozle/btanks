
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/
#include "options_menu.h"
#include "button.h"
#include "chooser.h"
#include "menu.h"
#include "control_picker.h"
#include "i18n.h"
#include "label.h"
#include "slider.h"
#include "config.h"
#include "checkbox.h"
#include "sound/mixer.h"
#include "redefine_keys.h"
#include "gamepad_setup.h"
#include "player_manager.h"
#include "game_monitor.h"
#include "window.h"

OptionsMenu::OptionsMenu(MainMenu *parent, const int w, const int h) : _parent(parent), _shoot(0.5f, false) {
	Mixer->loadSample("shot.ogg");
	
	_background.init("menu/background_box.png", w - 100, h - 100);
	int bw, bh;

	int mx, my;
	_background.getMargins(mx, my);
	_bx = ( w - _background.w ) / 2;
	_by = ( h - _background.h ) / 2;
	int base_x = _bx + 3 * _background.w / 4;

	_b_back = new Button("medium_dark", I18n->get("menu", "back"));
	_b_back->get_size(bw, bh);

	add(_bx + mx + _background.w / 4 - bw / 2, h -  my - bh - _by, _b_back);
	
	_b_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_b_ok->get_size(bw, bh);
	add(_by + my + 3 * _background.w / 4 - bw / 2, h -  my - bh - _by, _b_ok);
	
	int width = _background.w - 2 * mx;

	int sw, sh;
	int yp = my * 2 + _by;
	
	std::vector<std::string> langs;
	langs.push_back(I18n->get("menu/language", "default"));
	{
		I18n->getSupportedLanguages(_langs);
		for(std::set<std::string>::const_iterator i = _langs.begin(); i != _langs.end(); ++i) {
			langs.push_back(I18n->has("menu/language", *i)?I18n->get("menu/language", *i): *i);
		}
	}

	_lang = new Chooser("medium", langs);
	_lang->get_size(sw, sh);
	add(_bx + mx, yp, new Label("medium", I18n->get("menu/language", "language")));
	add(_bx + base_x - sw / 2, yp, _lang);
	yp += sh + 10;

	sp = new ControlPicker(width, "medium", I18n->get("menu", "single-player"), "player.control-method", "keys", std::string());
	sp->get_size(sw, sh);
	add(_bx + mx, yp, sp);
	yp += sh;

	sp1 = new ControlPicker(width, "medium", I18n->get("menu", "split-player-1"), "player.control-method-1", "keys-1", "split");
	sp1->get_size(sw, sh);
	add(_bx + mx, yp, sp1);
	yp += sh;

	sp2 = new ControlPicker(width, "medium", I18n->get("menu", "split-player-2"), "player.control-method-2", "keys-2", "split");
	sp2->get_size(sw, sh);
	add(_bx + mx, yp, sp2);
	yp += sh;
	
	
	_b_redefine = new Button("medium_dark", I18n->get("menu", "redefine-keys"));
	_b_redefine->get_size(sw, sh);
	add((w - sw) / 2, yp + 6, _b_redefine);

	yp += sh + 20;
	//volume controls 
	
	float volume;
	Config->get("engine.sound.volume.music", volume, 1.0f);
	
	Label *l = new Label("medium", I18n->get("menu", "music-volume"));
	Slider *s = _music = new Slider(volume);

	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	{
		int w, h;
		s->get_size(w, h);
		add(_bx + base_x - w / 2, yp + (sh - h) / 2, s);
		if (h > sh) 
			sh = h;
	}
	
	yp += sh + 10;

////////////////////

	Config->get("engine.sound.volume.fx", volume, 1.0f);
	
	l = new Label("medium", I18n->get("menu", "fx-volume"));
	s = _fx = new Slider(volume);
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	{
		int w, h;
		s->get_size(w, h);
		add(_bx + base_x - w / 2, yp + (sh - h) / 2, s);
		if (h > sh) 
			sh = h;
	}
	
	yp += sh + 10;

/////////////////

	Config->get("engine.sound.volume.ambience", volume, 0.5f);

	l = new Label("medium", I18n->get("menu", "ambience-volume"));
	s = _ambient = new Slider(volume);
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	{
		int w, h;
		s->get_size(w, h);
		add(_bx + base_x - w / 2, yp + (sh - h) / 2, s);
		if (h > sh) 
			sh = h;
	}
	
	yp += sh + 10;
	

	int screen_w, screen_h;
	Config->get("engine.window.width", screen_w, 800);
	Config->get("engine.window.height", screen_h, 600);

	{
		std::vector<std::string> res;
		bool standard = false;
		for(unsigned i = 0; i < Window->resolutions.size(); ++i) {
			if (screen_w == Window->resolutions[i].w && screen_h == Window->resolutions[i].h) 
				standard = true;
			res.push_back(mrt::format_string("%ux%u", Window->resolutions[i].w, Window->resolutions[i].h));
		}
		if (!standard) 
			res.push_back(mrt::format_string("%ux%u", screen_w, screen_h));

		_c_res = new Chooser("medium", res);
	}
	

	l = new Label("medium", I18n->get("menu", "screen-resolution"));
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	{
		int w, h;
		_c_res->get_size(w, h);
		add(_bx + base_x - w / 2, yp + (sh - h) / 2, _c_res);
		if (h > sh) 
			sh = h;
	}
	yp += sh + 10;

	TRY {
		_c_res->set(mrt::format_string("%dx%d", screen_w, screen_h));
	} CATCH("default resolution setup", );

	l = new Label("medium", I18n->get("menu", "fullscreen-mode"));
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	
	_fsmode = new Checkbox();
	{
		int w, h;
		_fsmode->get_size(w, h);
		add(_bx + base_x - w / 2 + 48, yp + (sh - h) / 2, _fsmode);
		if (h > sh) 
			sh = h;
	}
	yp += sh + 10;

	l = new Label("medium", I18n->get("menu", "do-not-show-donation-screen"));
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	
	_donate = new Checkbox();
	{
		int w, h;
		_donate->get_size(w, h);
		add(_bx + base_x - w / 2 + 48, yp + (sh - h) / 2, _donate);
		if (h > sh) 
			sh = h;
	}
	yp += sh + 10;

	l = new Label("medium", I18n->get("menu", "enable-fog-of-war"));
	add(_bx + mx, yp, l);
	l->get_size(sw, sh);
	
	_fog_of_war = new Checkbox();
	{
		int w, h;
		_fog_of_war->get_size(w, h);
		add(_bx + base_x - w / 2 + 48, yp + (sh - h) / 2, _fog_of_war);
		if (h > sh) 
			sh = h;
	}
	yp += sh + 10;


	//dialogs

	_keys = new RedefineKeys;
	_keys->get_size(sw, sh);
	add((w - sw) / 2, (h - sh) / 2, _keys);
	_keys->hide();
	
	_gamepad = new GamepadSetup(w, h);
	_gamepad->get_size(sw, sh);
	add((w - sw) / 2, (h - sh) / 2, _gamepad);
	_gamepad->hide();
	

	
	reload();
}

void OptionsMenu::get_size(int &w, int &h) const {
	w = _background.w;
	h = _background.h;
}

void OptionsMenu::reload() {
	LOG_DEBUG(("reloading options..."));
	sp->reload();
	sp1->reload();
	sp2->reload();
	
	float volume;
	Config->get("engine.sound.volume.music", volume, 1.0f);
	_music->set(volume);

	Config->get("engine.sound.volume.fx", volume, 1.0f);
	_fx->set(volume);

	Config->get("engine.sound.volume.ambience", volume, 0.5f);
	_ambient->set(volume);
	
	_keys->reload();
	
	std::string lang;
	if (Config->has("engine.language"))
		Config->get("engine.language", lang, std::string());

	//rewrite it to the simplier and extensible manner.
	if (lang.empty()) {
		_lang->set(0);
	} else {
		int idx = 1;
		for(std::set<std::string>::const_iterator i = _langs.begin(); i != _langs.end(); ++i, ++idx) {
			if (*i == lang) {
				_lang->set(idx);
				break;
			}
		}
	}

	TRY {
		int w, h;
		Config->get("engine.window.width", w, 800);
		Config->get("engine.window.height", h, 600);
		_c_res->set(mrt::format_string("%dx%d", w, h));
	} CATCH("default resolution setup", );

	bool fs;
	Config->get("engine.window.fullscreen", fs, false);
	_fsmode->set(fs);
	float donate;
	Config->get("engine.donate-screen-duration", donate, 1.5f);
	_donate->set(donate <= 0);
	bool fog;
	Config->get("engine.fog-of-war.enabled", fog, false);
	_fog_of_war->set(fog);
}

#include "window.h"

void OptionsMenu::save() {
	LOG_DEBUG(("saving options..."));
	sp->save();
	sp1->save();
	sp2->save();
	
	Config->set("engine.sound.volume.fx", _fx->get());
	Config->set("engine.sound.volume.music", _music->get());
	Config->set("engine.sound.volume.ambience", _ambient->get());
	
	bool need_restart = false;
	TRY {
		int idx = _lang->get();
		if (idx < 0 || idx > (int)_langs.size())
			throw_ex(("language index %d is invalid", idx));
		
		std::string lang;
		if (idx > 0) {
			std::set<std::string>::iterator lang_i = _langs.begin();
			for(int i = 1; i < idx; ++i) 
				++lang_i;
			lang = *lang_i;
		}
		
		std::string old_lang;
		if (Config->has("engine.language"))
			Config->get("engine.language", old_lang, std::string());
		
		if (old_lang != lang) {
			if (!lang.empty())
				Config->set("engine.language", lang);
			else 
				Config->remove("engine.language");
			need_restart = true;
		}
	} CATCH("saving language", );
	
	TRY {
		int screen_w, screen_h;
		Config->get("engine.window.width", screen_w, 800);
		Config->get("engine.window.height", screen_h, 600);
		std::vector<std::string> res;
		mrt::split(res, _c_res->getValue(), "x", 2);
		res.resize(2);
		int w, h;
		w = atoi(res[0].c_str());
		h = atoi(res[1].c_str());
		LOG_DEBUG(("parsed window size: %dx%d", w, h));
		
		if (w > 0 && h > 0 && (w != screen_w || h != screen_h)) {
			Config->set("engine.window.width", w);
			Config->set("engine.window.height", h);
			need_restart = true;
		}

	//this doesnt work without restart.
	/*
		Window->deinit();
		SDL_Quit();
		Window->initSDL();
		Window->createMainWindow();
	*/
	} CATCH("setting video mode", );

	bool fsmode;
	Config->get("engine.window.fullscreen", fsmode, false);
	if (fsmode != _fsmode->get()) {
		Config->set("engine.window.fullscreen", _fsmode->get());
		need_restart = true;
	}
	Config->set("engine.donate-screen-duration", (_donate->get())?0.0f:1.5f);
	Config->set("engine.fog-of-war.enabled", _fog_of_war->get());
	
	PlayerManager->update_controls();

	if (need_restart) 
		GameMonitor->displayMessage("messages", "restart-game", 2.0f);
}


void OptionsMenu::tick(const float dt) {
	if (_fx->changed() || _fx->tracking()) {
		_fx->reset();
		Mixer->setFXVolume(_fx->get());
		if (_shoot.tick(dt)) {
			Mixer->setListener(v3<float>(), v3<float>(), 64);
			Mixer->playSample(NULL, "shot.ogg", false);
			_shoot.reset();
		}
	}
	if (_music->changed()) {
		_music->reset();
		Mixer->setMusicVolume(_music->get());
	}
	if (_ambient->changed()) {
		_ambient->reset();
		Mixer->setAmbienceVolume(_ambient->get());
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

void OptionsMenu::render(sdlx::Surface &surface, const int x, const int y) const {
	_background.render(surface, _bx, _by);
	Container::render(surface, x, y);	
}

bool OptionsMenu::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym)) 
		return true;

	switch(sym.sym) {

	case SDLK_KP_ENTER:
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
		if (sdlx::Joystick::getCount() && _keys->hidden()) {
			_gamepad->reload();
			_gamepad->hide(false);
		}
			
		return true;

	case SDLK_r: 
		if (_gamepad->hidden())
			_keys->hide(false);
		return true;

	default: ;
	}
	return false;
}

OptionsMenu::~OptionsMenu() {}

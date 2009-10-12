
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
#include "hud.h"
#include "config.h"
#include "sdlx/font.h"
#include "player_manager.h"
#include "player_slot.h"
#include "object.h"
#include "tmx/map.h"
#include "src/player_manager.h"
#include "src/resource_manager.h"
#include "menu/box.h"
#include "game_monitor.h"
#include "finder.h"
#include "mrt/random.h"
#include "math/binary.h"
#include "special_zone.h"
#include "rt_config.h"
#include "sdlx/font.h"

static Uint32 index2color(const sdlx::Surface &surface, const unsigned idx, const Uint8 a) {
	unsigned rgb = idx & 7;
	unsigned rgb_div = (idx & 0x38) >> 3;
	unsigned r = ((rgb & 1) != 0)?255:0;
	unsigned g = ((rgb & 4) != 0)?255:0;
	unsigned b = ((rgb & 2) != 0)?255:0;
	if (rgb_div & 1) 
		r /= 2;
	if (rgb_div & 4) 
		g /= 2;
	if (rgb_div & 2) 
		b /= 2;
	
	return surface.map_rgba(r, g, b, a);
}

static Uint32 team2color(const sdlx::Surface &surface, const unsigned idx, const Uint8 a) {
	switch(idx) {
		case 0: //red team
			return surface.map_rgba(255, 0, 0, a);
		case 1: //green team
			return surface.map_rgba(0, 255, 0, a);
		case 2: //blue team
			return surface.map_rgba(0, 0, 255, a);
		case 3: //yellow team
			return surface.map_rgba(255, 255, 0, a);
		default: 
			return 0;
	}
}

void Hud::initMap() {
	_radar.free();
	_radar_bg.free();

	Config->get("hud.radar.enable", _enable_radar, true);
	
	_map_mode = MapSmall;
	
	_pointer = NULL;
	_pointer_dir = -1;
	if (RTConfig->game_type == GameTypeRacing) {
		_pointer = ResourceManager->load_surface("pointer.png");
	}
}

void Hud::generateRadarBG(const sdlx::Rect &viewport) {
	assert(Map->loaded());

	std::set<int> layers;
	Map->get_zBoxes(layers);

	GET_CONFIG_VALUE("hud.radar.zoom", int, zoom, 2);
	GET_CONFIG_VALUE("hud.radar.inverse", bool, hri, false);
	const Matrix<int>& matrix = Map->get_impassability_matrix(0);
	
	_radar_bg.create_rgb(zoom * matrix.get_width(), zoom * matrix.get_height(), 32);
	_radar_bg.display_format_alpha();
	_radar_bg.lock();
	LOG_DEBUG(("rendering radar..."));

	int n = 0;
	/*
	int cx = 0, cy = 0;
	if (Map->torus()) {
		const v2<int> pfs = Map->getPathTileSize();
		cx = viewport.x / pfs.x;
		cy = viewport.y / pfs.y;
	}
	*/
	for(std::set<int>::iterator i = layers.begin(); i != layers.end(); ++i, ++n) {
		const Matrix<int>& matrix = Map->get_impassability_matrix((*i) * 2000);

		//update radar;
		const int h = matrix.get_height(), w = matrix.get_width();
		for(int ry = 0; ry < h; ++ry) 
			for(int rx = 0; rx < w; ++rx) {
				int v = matrix.get((ry + h) % h, (rx + w) % w);
				if (v < 0 || v > 100) 
					v = 100;
				if (hri)
					v = 100 - v;
			
				for(int yy = 0; yy < zoom; ++yy) 
					for(int xx = 0; xx < zoom; ++xx) {
					Uint8 r, g, b, a;
					Uint8 rc, gc, bc, ac;
					
					_radar_bg.get_rgba(_radar_bg.get_pixel(rx*zoom + xx, ry*zoom + yy), r, g, b, a);
					_radar_bg.get_rgba(index2color(_radar_bg, n + 4, (128 + v) / layers.size()), rc, gc, bc, ac);
					
					Uint32 color = _radar_bg.map_rgba(r + v * rc / 100 / layers.size(), g + v * gc / 100 / layers.size(), b + v * bc / 100 / layers.size(), a + (128 + v) / layers.size());
					_radar_bg.put_pixel(rx*zoom + xx, ry*zoom + yy, color);
				}
			}
	}
	_radar_bg.unlock();
	_radar_bg.set_alpha(0, 0);
}

void Hud::renderStats(sdlx::Surface &surface) {
	if (RTConfig->game_type == GameTypeTeamDeathMatch || RTConfig->game_type == GameTypeCTF)
		renderTeamStats(surface);
	else 
		renderPlayerStats(surface); 
}

void Hud::renderTeamStats(sdlx::Surface &surface) {
	unsigned slots = PlayerManager->get_slots_count(), teams = RTConfig->teams;
	
	int max_w = 0;
	std::map<const Team::ID, int> team_frags;
	
	for(unsigned p = 0; p < slots; ++p) {
		PlayerSlot &slot = PlayerManager->get_slot(p);
		if (slot.empty() || slot.team == Team::None)
			continue;
			
		team_frags[slot.team] += slot.frags;
	}
	
	for(int team = 0; team < RTConfig->teams; ++team) {
		int w = _font->render(NULL, 0, 0, Team::get_color((Team::ID)team));
		if (w > max_w)
			max_w = w;
	}

	Box background;
	const int item_h = 10 + _font->get_height() ;
	background.init("menu/background_box.png", max_w + 96, item_h * teams + 2 * item_h);
	int mx, my;
	background.getMargins(mx, my);
	
	int xp = (surface.get_width() - background.w) / 2;
	int yp = (surface.get_height() - background.h) / 2;

	background.render(surface, xp, yp);

	xp += mx;
	yp += (background.h - item_h * teams) / 2 + _font->get_height() / 4;

	int box_h = _font->get_height();
	int box_w2 = _font->get_width();
	int box_w1 = box_w2 * 3 / 4;
	
	for(int team = 0; team < RTConfig->teams; ++team) {
		surface.fill_rect(sdlx::Rect(xp, yp, box_w1, box_h), team2color(surface, (unsigned)team, 255));
		_font->render(surface, xp + box_w2, yp, Team::get_color((Team::ID)team));
		std::string score = mrt::format_string("%d", team_frags[(Team::ID)team]);
		int sw = _font->render(NULL, 0, 0, score);
		_font->render(surface, xp + background.w - 2 * mx - sw, yp, score);
		yp += item_h;
	}
}

void Hud::renderPlayerStats(sdlx::Surface &surface) {
	unsigned active_slots = 0, slots = PlayerManager->get_slots_count();
	
	int nick_w = 0;
	
	for(unsigned p = 0; p < slots; ++p) {
		PlayerSlot &slot = PlayerManager->get_slot(p);
		if (slot.empty())
			continue;
		++active_slots;
		Object *o = slot.getObject();
		int w = _font->render(NULL, 0, 0, mrt::format_string("%s (%s)", slot.name.c_str(), o? o->animation.c_str():"dead"));
		if (w > nick_w)
			nick_w = w;
	}
	
	if (active_slots == 0)
		return;
	
	Box background;
	const int item_h = 10 + _font->get_height() ;
	
	
	background.init("menu/background_box.png", nick_w + 96, item_h * active_slots + 2 * item_h);
	int mx, my;
	background.getMargins(mx, my);
	
	int xp = (surface.get_width() - background.w) / 2;
	int yp = (surface.get_height() - background.h) / 2;

	background.render(surface, xp, yp);

	xp += mx;
	yp += (background.h - item_h * active_slots) / 2 + _font->get_height() / 4;

	int box_h = _font->get_height();
	int box_w2 = _font->get_width();
	int box_w1 = box_w2 * 3 / 4;
	
	for(unsigned p = 0; p < slots; ++p) {
		PlayerSlot &slot = PlayerManager->get_slot(p);
		if (slot.empty())
			continue;
		surface.fill_rect(sdlx::Rect(xp, yp, box_w1, box_h), index2color(surface, p + 1, 255));
		const Object * o = slot.getObject();
		_font->render(surface, xp + box_w2, yp, mrt::format_string("%s (%s)", slot.name.c_str(), o? o->animation.c_str():"dead"));
		std::string score = mrt::format_string("%d", slot.frags);
		int sw = _font->render(NULL, 0, 0, score);
		_font->render(surface, xp + background.w - 2 * mx - sw, yp, score);
		yp += item_h;
	}
	
}



void Hud::renderRadar(const float dt, sdlx::Surface &window, const std::vector<v3<int> > &specials, const std::vector<v3<int> > &flags, const sdlx::Rect &viewport) {
	if (!Map->loaded()) {
		_radar.free();
		_radar_bg.free();
		return;
	}

	if (_map_mode == MapNone || !_enable_radar)
		return;
		
	if (!_radar.isNull() && !_update_radar.tick(dt)) {
		const int x = window.get_width() - _radar.get_width(), y = _background->get_height();
		window.blit(_radar, x, y);
		return;
	}
	
	if (_radar_bg.isNull())
		generateRadarBG(viewport); //needed for destructable layers. 
		
	v2<int> radar_size;
	
	if (_map_mode == MapSmall) {
		radar_size.x = math::min(window.get_width() / 8, _radar_bg.get_width());
		radar_size.y = math::min(window.get_height() / 8, _radar_bg.get_height());
	} else {
		radar_size.x = _radar_bg.get_width();
		radar_size.y = _radar_bg.get_height();
	}
	
	if (_radar.isNull()) {
		_radar.create_rgb(radar_size.x, radar_size.y, 32);
		_radar.display_format_alpha();
	}

	const int x = window.get_width() - _radar.get_width(), y = _background->get_height();
	v2<int> msize = Map->get_size();

	v2<int> radar_shift;
	if (_map_mode == MapSmall || Map->torus()) {
		radar_shift.x = viewport.x + viewport.w / 2 - msize.x / 2 - msize.x * (_radar.get_width() - _radar_bg.get_width()) / 2 / _radar_bg.get_width();
		radar_shift.y = viewport.y + viewport.h / 2 - msize.y / 2 - msize.y * (_radar.get_height() - _radar_bg.get_height()) / 2 / _radar_bg.get_height();
		Map->validate(radar_shift);
	}

	if (Map->torus()) {
	/* 2x2 split
	[12]
	[34]
	*/
		v2<int> split = radar_shift;
		//LOG_DEBUG(("split: %d %d %d %d", split.x, split.y, viewport.x, viewport.y));
		split *= v2<int>(_radar_bg.get_width(), _radar_bg.get_height());
		split /= msize;
		//int split_x = (viewport.w - viewport.x) * _radar_bg.get_width() / msize.x, split_y = (viewport.h - viewport.y) * _radar_bg.get_width() / msize.x;

		_radar.fill(_radar.map_rgba(0,0,0,255));
		sdlx::Rect src1(split.x - _radar_bg.get_width(), split.y - _radar_bg.get_height(), _radar_bg.get_width(), _radar_bg.get_height());
		sdlx::Rect src2(split.x, split.y - _radar_bg.get_height(), _radar_bg.get_width(), _radar_bg.get_height());
		sdlx::Rect src3(split.x - _radar_bg.get_width(), split.y, _radar_bg.get_width(), _radar_bg.get_height());
		sdlx::Rect src4(split.x, split.y, _radar_bg.get_width(), _radar_bg.get_height());
		_radar.blit(_radar_bg, src1, 0, 0);
		_radar.blit(_radar_bg, src2, 0, 0);
		_radar.blit(_radar_bg, src3, 0, 0);
		_radar.blit(_radar_bg, src4, 0, 0);
	} else {
		if (radar_shift.x < 0) 
			radar_shift.x = 0;
		if (radar_shift.y < 0) 
			radar_shift.y = 0;
		v2<int> radar_map_size = radar_size * msize / v2<int>(_radar_bg.get_width(), _radar_bg.get_height());

		if (radar_shift.x + radar_map_size.x > msize.x)
			radar_shift.x = msize.x - radar_map_size.x;

		if (radar_shift.y + radar_map_size.y > msize.y)
			radar_shift.y = msize.y - radar_map_size.y;

		v2<int> shift = radar_shift * v2<int>(_radar_bg.get_width(), _radar_bg.get_height()) / msize;
		sdlx::Rect src(shift.x, shift.y, _radar.get_width(), _radar.get_height());
		_radar.blit(_radar_bg, src, 0, 0);
	}
		
	//LOG_DEBUG(("radar shift: %d %d", radar_shift.x, radar_shift.y));

	_radar.lock();
	
	size_t n = PlayerManager->get_slots_count();
	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = PlayerManager->get_slot(i);
		const Object *obj = slot.getObject();
		if (obj == NULL) 
			continue;
		
		v2<int> pos;
		obj->get_center_position(pos);
		pos -= radar_shift;
		Map->validate(pos);

		_radar.put_pixel(pos.x * _radar_bg.get_width() / msize.x, pos.y * _radar_bg.get_height() / msize.y, index2color(_radar, i + 1, 255));
		_radar.put_pixel(pos.x * _radar_bg.get_width() / msize.x, pos.y * _radar_bg.get_height() / msize.y + 1, index2color(_radar, i + 1, 200));
		_radar.put_pixel(pos.x * _radar_bg.get_width() / msize.x, pos.y * _radar_bg.get_height() / msize.y - 1, index2color(_radar, i + 1, 200));
		_radar.put_pixel(pos.x * _radar_bg.get_width() / msize.x + 1, pos.y * _radar_bg.get_height() / msize.y, index2color(_radar, i + 1, 200));
		_radar.put_pixel(pos.x * _radar_bg.get_width() / msize.x - 1, pos.y * _radar_bg.get_height() / msize.y, index2color(_radar, i + 1, 200));
	}
	
	static bool blink;
	
	blink = !blink;
	if (blink) {
	//format me
	n = specials.size();
	for(size_t i = 0; i < n; ++i) {
		v3<int> pos = specials[i];
		{
			v2<int> p(pos.x, pos.y);
			p -= radar_shift;
			Map->validate(p);
			pos.x = p.x; pos.y = p.y;
		}
		
		Uint32 color[2];
		color[0] = index2color(_radar, i + 1, 255);
		color[1] = index2color(_radar, i + 1, 200);
		for(int b = 0; b < 2; ++b) {
			_radar.put_pixel(b + pos.x * _radar_bg.get_width() / msize.x, pos.y * _radar_bg.get_height() / msize.y, color[b]);
			for(int l = 1; l <= 2; ++l) {
				_radar.put_pixel(b + pos.x * _radar_bg.get_width() / msize.x + l, pos.y * _radar_bg.get_height() / msize.y + l, color[b]);
				_radar.put_pixel(b + pos.x * _radar_bg.get_width() / msize.x - l, pos.y * _radar_bg.get_height() / msize.y - l, color[b]);
				_radar.put_pixel(b + pos.x * _radar_bg.get_width() / msize.x + l, pos.y * _radar_bg.get_height() / msize.y - l, color[b]);
				_radar.put_pixel(b + pos.x * _radar_bg.get_width() / msize.x - l, pos.y * _radar_bg.get_height() / msize.y + l, color[b]);
			}
		}
	}
	

	n = flags.size();
	if (n > 2) 
		n = 2;

	for(size_t i = 0; i < n; ++i) {
		v3<int> pos = flags[i];
		{
			v2<int> p(pos.x, pos.y);
			p -= radar_shift;
			Map->validate(p);
			pos.x = p.x; pos.y = p.y;
		}
		
		Uint32 color[2] = { _radar.map_rgb(255, 0, 0), _radar.map_rgb(0, 255, 0), };
		_radar.put_pixel(0 + pos.x * _radar_bg.get_width() / msize.x, 0 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(1 + pos.x * _radar_bg.get_width() / msize.x, 0 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(2 + pos.x * _radar_bg.get_width() / msize.x, 0 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(0 + pos.x * _radar_bg.get_width() / msize.x, 1 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(1 + pos.x * _radar_bg.get_width() / msize.x, 1 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(0 + pos.x * _radar_bg.get_width() / msize.x, 2 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(1 + pos.x * _radar_bg.get_width() / msize.x, 2 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(2 + pos.x * _radar_bg.get_width() / msize.x, 2 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
		_radar.put_pixel(0 + pos.x * _radar_bg.get_width() / msize.x, 3 + pos.y * _radar_bg.get_height() / msize.y, color[i]);
	}

	} //blink
	
	_radar.unlock();
	
	window.blit(_radar, x, y);
}

void Hud::renderMod(const Object *obj, sdlx::Surface &window, int &xp, int &yp, const std::string &mod_name, const int icon_w, const int icon_h) const {
	if (!obj->has(mod_name))
		return;
			
	const Object *mod = obj->get(mod_name);
	int count = mod->getCount();
	if (count == 0) {
		xp += icon_w;
		xp += _font->render(window, xp, yp, "  ");
		return;
	}
			
	std::string name = "mod:";
	name += mod->getType();
	//LOG_DEBUG(("icon name = '%s'", name.c_str()));
	IconMap::const_iterator i = _icons_map.find(name);
	if (i == _icons_map.end()) {
		xp += icon_w;
		xp += _font->render(window, xp, yp, "  ");
		return;
	}
	
	const int font_dy = (icon_h - _font->get_height()) / 2;

	sdlx::Rect src(icon_w * i->second, 0, icon_w, icon_h);
	window.blit(*_icons, src, xp, yp);
	xp += icon_w;
	if (count > 0)
		xp += _font->render(window, xp, yp + font_dy, mrt::format_string("%-2d", count));
	else 
		xp += _font->render(window, xp, yp, "  ");
	window.blit(*_splitter, xp, yp);
	xp += _splitter->get_width();
}



void Hud::render(sdlx::Surface &window) const {
	
	window.blit(*_background, 0, 0);
	
	size_t n = PlayerManager->get_slots_count();

	GET_CONFIG_VALUE("hud.icon.width", int, icon_w, 16);
	GET_CONFIG_VALUE("hud.icon.height", int, icon_h, 24);
	
	const int font_dy = (icon_h - _font->get_height()) / 2;

	int c = 0;
	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = PlayerManager->get_slot(i);
		if (!slot.visible)
			continue;

		++c;
		
		const Object *obj = slot.getObject();
	
		GET_CONFIG_VALUE("hud.margin.x", int, xm, 3);
		GET_CONFIG_VALUE("hud.margin.y", int, ym, 3);

		int xp = slot.viewport.x + xm;
		int yp = slot.viewport.y + ym;

		{
			std::string score = mrt::format_string("$%d", slot.score);
			int tw = _font->render(NULL, 0, 0, score);
			_font->render(window, xp + slot.viewport.w - xm * 2- tw, yp + font_dy, score);
		}

		do {
			const int n = slot.spawn_limit;
			if (n <= 0) 
				break;
			
			IconMap::const_iterator ic = _icons_map.find("special:lives");
			if (ic == _icons_map.end())
				break;

			sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
			
			window.blit(*_icons, src, xp, yp);
			xp += icon_w;

			if (n > 5) {
				xp += _font->render(window, xp, yp + font_dy, mrt::format_string("%d ", n));
			} else {
				for(int i = 0; i < n - 1; ++i) {
					window.blit(*_icons, src, xp, yp);
					xp += icon_w;
				}
			}
			xp += icon_w / 2;
			
		} while(0);

		if (obj == NULL)
			continue;


		std::string hp = mrt::format_string("HP%-2d ", obj->hp);

		xp += _font->render(window, xp, yp + font_dy, hp);	
		
		renderMod(obj, window, xp, yp, "mod", icon_w, icon_h);
		renderMod(obj, window, xp, yp, "alt-mod", icon_w, icon_h);
				
		IconMap::const_iterator a = _icons_map.lower_bound("effect:");
		bool any_effect = false;
		int old_xp = xp;
		for(IconMap::const_iterator ic = a; ic != _icons_map.end(); ++ic) {
			if (ic->first.compare(0, 7, "effect:") != 0) 
				break;
			const std::string name = ic->first.substr(7);
			
			//LOG_DEBUG(("%s %s", ic->first.c_str(), name.c_str()));
			
			if (obj->has_effect(name)) {
				sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
				window.blit(*_icons, src, xp, yp);
				xp += icon_w;
			
				float effect_rt = obj->get_effect_timer(name);
				int rm;
				if (effect_rt < 10) {
					rm = (int)(10 * effect_rt);
				} else {
					rm = (int)effect_rt;
				}
				
				if (rm >= 0) {
					xp += _font->render(window, xp, yp + font_dy, mrt::format_string("%-2d ", rm));
				}
				any_effect = true;
			}
		}
		if (xp - old_xp < 4 * icon_w) {
			xp = old_xp + 4 * icon_w;
		}

		if (any_effect) {
			window.blit(*_splitter, xp, yp);
			xp += _splitter->get_width();
		}
		
		do {
			if (slot.frags == 0) 
				break;
			
			IconMap::const_iterator ic = _icons_map.find("special:frag");
			if (ic == _icons_map.end())
				break;

			sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
			window.blit(*_icons, src, xp, yp);
			xp += icon_w;
			xp += _font->render(window, xp, yp + font_dy, mrt::format_string("%-2d ", slot.frags));
			
		} while(0);

		xp = slot.viewport.x + xm;
		yp = slot.viewport.y + _background->get_height();

		if (_pointer != NULL) {
			const SpecialZone &zone = PlayerManager->get_next_checkpoint(slot);
			v2<float> pos;
			obj->get_position(pos);
			pos = v2<float>(zone.position.x, zone.position.y)  + zone.size.convert<float>() / 2 - pos;
			pos.normalize();
			//LOG_DEBUG(("direction: %g, %g", pos.x, pos.y));
			_pointer_dir = pos.get_direction(8) - 1;
			if (_pointer_dir >= 0) {
				int h = _pointer->get_height();
				sdlx::Rect src(_pointer_dir * h, 0, h, h);
				window.blit(*_pointer, src, xp, yp);
			}
		}
	}
	
	if (c >= 2) {
		//fixme: add more split screen modes ? 
		//fixme: just draw splitter centered. 
		window.blit(*_screen_splitter, (window.get_width() - _screen_splitter->get_width()) / 2, 0);
	}
}

void Hud::renderSplash(sdlx::Surface &window) const {
	if (_splash == NULL) {
		window.fill(window.map_rgb(239, 239, 239));
		return;
	}
	int spx = (window.get_width() - _splash->get_width()) / 2;
	int spy = (window.get_height() - _splash->get_height()) / 2;
	
	window.blit(*_splash, spx, spy);
}

#include "i18n.h"

const bool Hud::renderLoadingBar(sdlx::Surface &window, const float old_progress, const float progress, const char * what, const bool render_splash) const {
	assert(old_progress >= 0 && old_progress <= 1.0);
	assert(progress >= 0 && progress <= 1.0);

	GET_CONFIG_VALUE("hud.loading-bar.position", float, yf, 2.0f/3);
	GET_CONFIG_VALUE("hud.loading-bar.border-size", int, border, 3);
	
	int y = (int)(window.get_height() * yf);
	int x = (window.get_width() - _loading_border->get_width()) / 2;
	
	int w = (int) (progress * (_loading_border->get_width() - 2 * border));
	int w_old = (int) (old_progress * (_loading_border->get_width() - 2 * border));
	if (w == w_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;
	}

	int n = w / _loading_item->get_width(), n_old = w_old / _loading_item->get_width();
	if (n == n_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;	
	}

	if (render_splash)
		renderSplash(window);
	
	window.blit(*_loading_border, x, y);

	for(int i = 0; i < n; ++i) {
		window.blit(*_loading_item, border + x + i * _loading_item->get_width(), y + border);
	}

	if (what != NULL) {
		std::string status = what;
		if (I18n->has("loading", status)) {
			int dy = (_loading_border->get_height() - _small_font->get_height()) / 2;
			_small_font->render(window, x + border + dy, y + dy, I18n->get("loading", status));
		} else LOG_WARN(("unknown loading status message: '%s'", what));
	};
/*	w -= n * _loading_item.get_width();
	sdlx::Rect src(0, 0, w, _loading_item.get_height());
	window.blit(_loading_item, src, border + x + i * _loading_item.get_width(), y + border);
*/
	return true;
}

static void find_splashes(std::vector<std::string> &splashes, const std::string &prefix) {
	splashes.clear();
	std::vector<std::string> path;
	Finder->getPath(path);
	for(size_t i = 0; i < path.size(); ++i) {
		std::vector<std::string> files; 
		Finder->enumerate(files, path[i], "tiles");
		for(size_t j = 0; j < files.size(); ++j) {
			if (files[j].compare(0, prefix.size(), prefix) != 0)
				continue;
			//LOG_DEBUG(("found splash: %s", files[j].c_str()));
			splashes.push_back(files[j]);
		}
	}
}

Hud::Hud(const int w, const int h) :  _pointer(NULL), _pointer_dir(-1), _update_radar(true), _map_mode(MapSmall) {
	init_map_slot.assign(this, &Hud::initMap, Map->load_map_final_signal);
	on_destroy_map_slot.assign(this, &Hud::on_destroy_map, Map->destroyed_cells_signal);

	_background = ResourceManager->load_surface("hud/hud_line.png");
	_loading_border = ResourceManager->load_surface("hud/loading_border.png");
	_loading_item = ResourceManager->load_surface("hud/loading_item.png");
	_icons = ResourceManager->load_surface("hud/hud_icons.png");
	_splitter = ResourceManager->load_surface("hud/hud_splitter.png");
	_screen_splitter = ResourceManager->load_surface("hud/split_line.png");
	
	_font = ResourceManager->loadFont("medium", true);
	_big_font = ResourceManager->loadFont("big", true);
	_small_font = ResourceManager->loadFont("small", true);
	
	LOG_DEBUG(("searching splash... %dx%d", w, h));
	int sw = 0;
	int splash_sizes[] = { 1280 };
	for(unsigned si = 0; si < sizeof(splash_sizes) / sizeof(splash_sizes[0]); ++si) {
		sw = splash_sizes[si];
		if (w >= sw) {
			break;
		}
	}
	LOG_DEBUG(("using splash width %d", sw));
	std::vector<std::string> files;
	find_splashes(files, mrt::format_string("xsplash_%d_", sw));
	if (files.empty()) {
		find_splashes(files, mrt::format_string("splash_%d_", sw));
	}
	
	if (!files.empty()) {
		_splash = ResourceManager->load_surface(files[mrt::random(files.size())], w, 0);
	} else {
		_splash = NULL;
	}

	GET_CONFIG_VALUE("hud.radar-update-interval", float, ru, 0.2f);
	_update_radar.set(ru);
	
	_icons_map.clear();
	int i = 0;
	_icons_map["mod:missiles:guided"] = i++;
	_icons_map["mod:missiles:smoke"] = i++;
	_icons_map["mod:missiles:dumb"] = i++;
	_icons_map["mod:missiles:nuke"] = i++;
	_icons_map["mod:missiles:boomerang"] = i++;
	_icons_map["mod:missiles:stun"] = i++;
	_icons_map["effect:dirt"] = i++;
	_icons_map["effect:ricochet"] = i++;
	_icons_map["effect:dispersion"] = i++;
	_icons_map["effect:machinegunner"] = i;
	_icons_map["mod:machinegunner"] = i++;
	_icons_map["mod:mines:regular"] = i++;
	_icons_map["special:frag"] = i++;
	_icons_map["effect:thrower"] = i;
	_icons_map["mod:thrower"] = i++;
	_icons_map["special:lives"] = i++;
	_icons_map["mod:mines:nuke"] = i++;
	_icons_map["effect:slowdown"] = i++;
	_icons_map["effect:speedup"] = i++;
	_icons_map["effect:invulnerability"] = i++;
}

Hud::~Hud() {}

void Hud::on_destroy_map(const std::set<v3<int> > & cells) {
	_radar_bg.free();
}

void Hud::toggleMapMode() {
	bool same_size = !_radar.isNull() && !_radar_bg.isNull() && 
		_radar.get_width() == _radar_bg.get_width() && _radar.get_height() == _radar_bg.get_height();
	
	switch(_map_mode) {
		case MapNone: 
			_map_mode = same_size?MapFull:MapSmall; break;
		case MapSmall:
			_map_mode = same_size?MapNone:MapFull; break;
		case MapFull:
		default: 
			_map_mode = MapNone;
	}
	LOG_DEBUG(("toggling map mode(%d)", (int)_map_mode));
	_radar.free();
}


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
	
	return surface.mapRGBA(r, g, b, a);
}

void Hud::initMap() {
	_radar.free();
	_radar_bg.free();
}

void Hud::generateRadarBG(const sdlx::Rect &viewport) {
	assert(Map->loaded());

	std::set<int> layers;
	Map->getZBoxes(layers);

	GET_CONFIG_VALUE("hud.radar.zoom", int, zoom, 2);
	const Matrix<int>& matrix = Map->getImpassabilityMatrix(0);
	
	_radar_bg.createRGB(zoom * matrix.getWidth(), zoom * matrix.getHeight(), 32);
	_radar_bg.convertAlpha();
	_radar_bg.lock();
	//LOG_DEBUG(("rendering radar..."));

	int n = 0;
	for(std::set<int>::iterator i = layers.begin(); i != layers.end(); ++i, ++n) {
		const Matrix<int>& matrix = Map->getImpassabilityMatrix((*i) * 2000);

		//update radar;
		for(int ry = 0; ry < matrix.getHeight(); ++ry) 
			for(int rx = 0; rx < matrix.getWidth(); ++rx) {
				int v = matrix.get(ry, rx);
				if (v < 0 || v > 100) 
					v = 100;
			
				for(int yy = 0; yy < zoom; ++yy) 
					for(int xx = 0; xx < zoom; ++xx) {
					Uint8 r, g, b, a;
					Uint8 rc, gc, bc, ac;
					
					_radar_bg.getRGBA(_radar_bg.getPixel(rx*zoom + xx, ry*zoom + yy), r, g, b, a);
					_radar_bg.getRGBA(index2color(_radar_bg, n + 4, (128 + v) / layers.size()), rc, gc, bc, ac);
					
					Uint32 color = _radar_bg.mapRGBA(r + v * rc / 100 / layers.size(), g + v * gc / 100 / layers.size(), b + v * bc / 100 / layers.size(), a + (128 + v) / layers.size());
					_radar_bg.putPixel(rx*zoom + xx, ry*zoom + yy, color);
				}
			}
	}
	_radar_bg.unlock();
	_radar_bg.setAlpha(0, 0);
}

void Hud::renderStats(sdlx::Surface &surface) {
	unsigned active_slots = 0, slots = PlayerManager->getSlotsCount();
	
	for(unsigned p = 0; p < slots; ++p) {
		PlayerSlot &slot = PlayerManager->getSlot(p);
		if (slot.id == -1)
			continue;
		++active_slots;
	}
	
	Box background;
	const int item_h = 10 + _font->getHeight() ;
	
	
	background.init("menu/background_box.png", 300, item_h * active_slots + 2 * item_h);
	int mx, my;
	background.getMargins(mx, my);
	
	int xp = (surface.getWidth() - background.w) / 2;
	int yp = (surface.getHeight() - background.h) / 2;

	background.render(surface, xp, yp);

	xp += mx;
	yp += (background.h - item_h * active_slots) / 2 + _font->getHeight() / 4;

	int box_h = _font->getHeight();
	int box_w2 = _font->getWidth();
	int box_w1 = box_w2 * 3 / 4;
	
	for(unsigned p = 0; p < slots; ++p) {
		PlayerSlot &slot = PlayerManager->getSlot(p);
		if (slot.id == -1)
			continue;
		surface.fillRect(sdlx::Rect(xp, yp, box_w1, box_h), index2color(surface, p + 1, 255));
		_font->render(surface, xp + box_w2, yp, mrt::formatString("%s", slot.animation.c_str()));
		std::string score = mrt::formatString("%d", slot.frags);
		int sw = _font->render(NULL, 0, 0, score);
		_font->render(surface, xp + background.w - 2 * mx - sw, yp, score);
		yp += item_h;
	}
	
}



void Hud::renderRadar(const float dt, sdlx::Surface &window, const std::vector<v3<int> > &specials, const sdlx::Rect &viewport) {
	if (!Map->loaded()) {
		_radar.free();
		_radar_bg.free();
		return;
	}
	
	if (!_radar.isNull() && !_update_radar.tick(dt)) {
		const int x = window.getWidth() - _radar.getWidth(), y = _background->getHeight();
		window.copyFrom(_radar, x, y);
		return;
	}
	
	generateRadarBG(viewport); //needed for destructable layers. 
	
	if (_radar.isNull()) {
		_radar.createRGB(_radar_bg.getWidth(), _radar_bg.getHeight(), 32);
		_radar.convertAlpha();
	}

	const int x = window.getWidth() - _radar.getWidth(), y = _background->getHeight();

	v2<int> msize = Map->getSize();
	size_t n = PlayerManager->getSlotsCount();

	_radar.copyFrom(_radar_bg, 0, 0);
	_radar.lock();
	
	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = PlayerManager->getSlot(i);
		const Object *obj = slot.getObject();
		if (obj == NULL) 
			continue;
		
		v2<int> pos;
		obj->getCenterPosition(pos);
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 255));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y + 1, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y - 1, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x + 1, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x - 1, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 200));
	}
	
	static bool blink;
	
	blink = !blink;
	if (blink) {
	//format me
	n = specials.size();
	for(size_t i = 0; i < n; ++i) {
		const v3<int> &pos = specials[i];
		Uint32 color[2];
		color[0] = index2color(_radar, i + 1, 255);
		color[1] = index2color(_radar, i + 1, 200);
		for(int b = 0; b < 2; ++b) {
			_radar.putPixel(b + pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y, color[b]);
			for(int l = 1; l <= 2; ++l) {
				_radar.putPixel(b + pos.x * _radar.getWidth() / msize.x + l, pos.y * _radar.getHeight() / msize.y + l, color[b]);
				_radar.putPixel(b + pos.x * _radar.getWidth() / msize.x - l, pos.y * _radar.getHeight() / msize.y - l, color[b]);
				_radar.putPixel(b + pos.x * _radar.getWidth() / msize.x + l, pos.y * _radar.getHeight() / msize.y - l, color[b]);
				_radar.putPixel(b + pos.x * _radar.getWidth() / msize.x - l, pos.y * _radar.getHeight() / msize.y + l, color[b]);
			}
		}
	}
	
	}
	
	_radar.unlock();
	window.copyFrom(_radar, x, y);
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
	
	const int font_dy = (icon_h - _font->getHeight()) / 2;

	sdlx::Rect src(icon_w * i->second, 0, icon_w, icon_h);
	window.copyFrom(*_icons, src, xp, yp);
	xp += icon_w;
	if (count > 0)
		xp += _font->render(window, xp, yp + font_dy, mrt::formatString("%-2d", count));
	else 
		xp += _font->render(window, xp, yp, "  ");
	window.copyFrom(*_splitter, xp, yp);
	xp += _splitter->getWidth();
}



void Hud::render(sdlx::Surface &window) const {
	
	window.copyFrom(*_background, 0, 0);
	
	size_t n = PlayerManager->getSlotsCount();

	GET_CONFIG_VALUE("hud.icon.width", int, icon_w, 16);
	GET_CONFIG_VALUE("hud.icon.height", int, icon_h, 24);
	
	const int font_dy = (icon_h - _font->getHeight()) / 2;

	int c = 0;
	for(size_t i = 0; i < n; ++i) {
		const PlayerSlot &slot = PlayerManager->getSlot(i);
		if (!slot.visible)
			continue;

		++c;
		
		const Object *obj = slot.getObject();
	
		GET_CONFIG_VALUE("hud.margin.x", int, xm, 3);
		GET_CONFIG_VALUE("hud.margin.y", int, ym, 3);

		int xp = slot.viewport.x + xm;
		int yp = slot.viewport.y + ym;

		{
			std::string score = mrt::formatString("$%d", slot.score);
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
			
			window.copyFrom(*_icons, src, xp, yp);
			xp += icon_w;

			if (n > 5) {
				xp += _font->render(window, xp, yp + font_dy, mrt::formatString("%d ", n));
			} else {
				for(int i = 0; i < n - 1; ++i) {
					window.copyFrom(*_icons, src, xp, yp);
					xp += icon_w;
				}
			}
			xp += icon_w / 2;
			
		} while(0);

		if (obj == NULL)
			continue;


		std::string hp = mrt::formatString("HP%-2d ", obj->hp);

		xp += _font->render(window, xp, yp + font_dy, hp);	
		
		renderMod(obj, window, xp, yp, "mod", icon_w, icon_h);
		renderMod(obj, window, xp, yp, "alt-mod", icon_w, icon_h);
				
		IconMap::const_iterator a = _icons_map.lower_bound("effect:");
		bool any_effect = false;
		int old_xp = xp;
		for(IconMap::const_iterator ic = a; ic != _icons_map.end(); ++ic) {
			if (ic->first.substr(0, 7) != "effect:") 
				break;
			const std::string name = ic->first.substr(7);
			
			//LOG_DEBUG(("%s %s", ic->first.c_str(), name.c_str()));
			
			if (obj->isEffectActive(name)) {
				sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
				window.copyFrom(*_icons, src, xp, yp);
				xp += icon_w;
			
				float effect_rt = obj->getEffectTimer(name);
				int rm;
				if (effect_rt < 10) {
					rm = (int)(10 * effect_rt);
				} else {
					rm = (int)effect_rt;
				}
				
				if (rm >= 0) {
					xp += _font->render(window, xp, yp + font_dy, mrt::formatString("%-2d ", rm));
				}
				any_effect = true;
			}
		}
		if (xp - old_xp < 4 * icon_w) {
			xp = old_xp + 4 * icon_w;
		}

		if (any_effect) {
			window.copyFrom(*_splitter, xp, yp);
			xp += _splitter->getWidth();
		}
		
		do {
			if (slot.frags == 0) 
				break;
			
			IconMap::const_iterator ic = _icons_map.find("special:frag");
			if (ic == _icons_map.end())
				break;

			sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
			window.copyFrom(*_icons, src, xp, yp);
			xp += icon_w;
			xp += _font->render(window, xp, yp + font_dy, mrt::formatString("%-2d ", slot.frags));
			
		} while(0);
	}
	
	if (c >= 2) {
		//fixme: add more split screen modes ? 
		//fixme: just draw splitter centered. 
		window.copyFrom(*_screen_splitter, (window.getWidth() - _screen_splitter->getWidth()) / 2, 0);
	}
}

void Hud::renderSplash(sdlx::Surface &window) const {
	int spx = (window.getWidth() - _splash->getWidth()) / 2;
	int spy = (window.getHeight() - _splash->getHeight()) / 2;
	
	window.copyFrom(*_splash, spx, spy);
}


const bool Hud::renderLoadingBar(sdlx::Surface &window, const float old_progress, const float progress, const bool render_splash) const {
	assert(old_progress >= 0 && old_progress <= 1.0);
	assert(progress >= 0 && progress <= 1.0);

	GET_CONFIG_VALUE("hud.loading-bar.position", float, yf, 2.0/3);
	GET_CONFIG_VALUE("hud.loading-bar.border-size", int, border, 3);
	
	int y = (int)(window.getHeight() * yf);
	int x = (window.getWidth() - _loading_border->getWidth()) / 2;
	
	int w = (int) (progress * (_loading_border->getWidth() - 2 * border));
	int w_old = (int) (old_progress * (_loading_border->getWidth() - 2 * border));
	if (w == w_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;
	}

	int n = w / _loading_item->getWidth(), n_old = w_old / _loading_item->getWidth();
	if (n == n_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;	
	}

	if (render_splash)
		renderSplash(window);
	
	window.copyFrom(*_loading_border, x, y);

	for(int i = 0; i < n; ++i) {
		window.copyFrom(*_loading_item, border + x + i * _loading_item->getWidth(), y + border);
	}
/*	w -= n * _loading_item.getWidth();
	sdlx::Rect src(0, 0, w, _loading_item.getHeight());
	window.copyFrom(_loading_item, src, border + x + i * _loading_item.getWidth(), y + border);
*/
	return true;
}

Hud::Hud(const int w, const int h) : _update_radar(true) {
	Map->load_map_final_signal.connect(sigc::mem_fun(this, &Hud::initMap));

	_background = ResourceManager->loadSurface("hud/hud_line.png");
	_loading_border = ResourceManager->loadSurface("hud/loading_border.png");
	_loading_item = ResourceManager->loadSurface("hud/loading_item.png");
	_icons = ResourceManager->loadSurface("hud/hud_icons.png");
	_splitter = ResourceManager->loadSurface("hud/hud_splitter.png");
	_screen_splitter = ResourceManager->loadSurface("hud/split_line.png");
	
	_font = ResourceManager->loadFont("medium", true);
	_big_font = ResourceManager->loadFont("big", true);
	
	LOG_DEBUG(("searching splash... %dx%d", w, h));
	int sw = 0;
	int splash_sizes[] = { 1280, 1152, 1024, 800 };
	for(unsigned si = 0; si < sizeof(splash_sizes) / sizeof(splash_sizes[0]); ++si) {
		sw = splash_sizes[si];
		if (w >= sw) {
			break;
		}
	}
	LOG_DEBUG(("using splash %d", sw));
	int idx;
	std::vector<int> indexes;
	for(idx = 1; idx <= 9; ++idx) {
		std::string fname = Finder->find(mrt::formatString("tiles/splash_%d_%d.jpg", sw, idx), false);
		if (!fname.empty())
			indexes.push_back(idx);
	}
	idx = indexes[mrt::random(indexes.size())];
	_splash = ResourceManager->loadSurface(mrt::formatString("splash_%d_%d.jpg", sw, idx));

	GET_CONFIG_VALUE("hud.radar-update-interval", float, ru, 0.2);
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
	_icons_map["mod:thrower"] = i++;
	_icons_map["special:lives"] = i++;
}

Hud::~Hud() {}


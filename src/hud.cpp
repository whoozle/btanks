
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

void Hud::renderRadar(const float dt, sdlx::Surface &window) {
	if (!Map->loaded()) {
		_radar.free();
		return;
	}
	
	if (!_radar.isNull() && !_update_radar.tick(dt)) {
		const int x = window.getWidth() - _radar.getWidth(), y = _background.getHeight();
		window.copyFrom(_radar, x, y);
		return;
	}
		
	
	Matrix<int> matrix; 
	Map->getImpassabilityMatrix(matrix);
	GET_CONFIG_VALUE("hud.radar.zoom", int, zoom, 3);

	if (_radar.isNull() || (zoom * matrix.getWidth() != _radar.getWidth() || zoom * matrix.getHeight() != _radar.getHeight())) {
		LOG_DEBUG(("creating radar surface..."));
		_radar.createRGB(zoom * matrix.getWidth(), zoom * matrix.getHeight(), 32);
		_radar.convertAlpha();
	}
	//LOG_DEBUG(("rendering radar..."));
	const int x = window.getWidth() - _radar.getWidth(), y = _background.getHeight();
	_radar.lock();
	//update radar;
	for(int ry = 0; ry < matrix.getHeight(); ++ry) 
		for(int rx = 0; rx < matrix.getWidth(); ++rx) {
			int v = matrix.get(ry, rx);
			if (v < 0 || v > 100) 
				v = 100;
			
			for(int yy = 0; yy < zoom; ++yy) 
				for(int xx = 0; xx < zoom; ++xx) {
				_radar.putPixel(rx*zoom + xx, ry*zoom + yy, _radar.mapRGBA(0, v * 255 / 100, 0, 128 + v));
			}
		}

	v3<int> msize = Map->getSize();
	size_t n = PlayerManager->getSlotsCount();
	
	for(size_t i = 0; i < n; ++i) {
		PlayerSlot &slot = PlayerManager->getSlot(i);
		const Object *obj = slot.getObject();
		if (obj == NULL) 
			continue;
		
		v3<int> pos;
		obj->getPosition(pos);
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 255));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y + 1, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x, pos.y * _radar.getHeight() / msize.y - 1, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x + 1, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 200));
		_radar.putPixel(pos.x * _radar.getWidth() / msize.x - 1, pos.y * _radar.getHeight() / msize.y, index2color(_radar, i + 1, 200));
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
		xp += _font.render(window, xp, yp, "  ");
		return;
	}
			
	std::string name = "mod:";
	name += mod->getType();
	//LOG_DEBUG(("icon name = '%s'", name.c_str()));
	IconMap::const_iterator i = _icons_map.find(name);
	if (i == _icons_map.end()) {
		xp += icon_w;
		xp += _font.render(window, xp, yp, "  ");
		return;
	}
				
	sdlx::Rect src(icon_w * i->second, 0, icon_w, icon_h);
	window.copyFrom(_icons, src, xp, yp);
	xp += icon_w;
	if (count > 0)
		xp += _font.render(window, xp, yp, mrt::formatString("%-2d", count));
	else 
		xp += _font.render(window, xp, yp, "  ");
	window.copyFrom(_splitter, xp, yp);
	xp += _splitter.getWidth();
}



void Hud::render(sdlx::Surface &window) const {
	
	window.copyFrom(_background, 0, 0);
	
	size_t n = PlayerManager->getSlotsCount();

	//only one visible player supported
	for(size_t i = 0; i < n; ++i) {
		const PlayerSlot &slot = PlayerManager->getSlot(i);
		if (!slot.visible)
			continue;
		const Object *obj = slot.getObject();
	
		std::string hp = mrt::formatString("HP%-2d ", (obj)?obj->hp:0);

		if (obj == NULL)
			continue;

		GET_CONFIG_VALUE("hud.icon.width", int, icon_w, 16);
		GET_CONFIG_VALUE("hud.icon.height", int, icon_h, 24);

		int xp = slot.viewport.x;

		int yp = slot.viewport.y + 3;

		xp += _font.render(window, xp, yp, hp);	
		
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
				window.copyFrom(_icons, src, xp, yp);
				xp += icon_w;
			
				int rm = (int)(10 * obj->getEffectTimer(name));
				if (rm >= 0) {
					xp += _font.render(window, xp, yp, mrt::formatString("%-2d ", rm));
				}
				any_effect = true;
			}
		}
		if (xp - old_xp < 4 * icon_w) {
			xp = old_xp + 4 * icon_w;
		}

		if (any_effect) {
			window.copyFrom(_splitter, xp, yp);
			xp += _splitter.getWidth();
		}
		
		do {
			if (slot.frags == 0) 
				break;
			
			IconMap::const_iterator ic = _icons_map.find("special:frag");
			if (ic == _icons_map.end())
				break;

			sdlx::Rect src(icon_w * ic->second, 0, icon_w, icon_h);
			window.copyFrom(_icons, src, xp, yp);
			xp += icon_w;
			xp += _font.render(window, xp, yp, mrt::formatString("%-2d ", slot.frags));
			
		} while(0);
	}
}

void Hud::renderSplash(sdlx::Surface &window) const {
	int spx = (window.getWidth() - _splash.getWidth()) / 2;
	int spy = (window.getHeight() - _splash.getHeight()) / 2;
	
	window.copyFrom(_splash, spx, spy);
}


const bool Hud::renderLoadingBar(sdlx::Surface &window, const float old_progress, const float progress) const {
	assert(old_progress >= 0 && old_progress <= 1.0);
	assert(progress >= 0 && progress <= 1.0);


	GET_CONFIG_VALUE("hud.loading-bar.position", float, yf, 2.0/3);
	GET_CONFIG_VALUE("hud.loading-bar.border-size", int, border, 3);
	
	int y = (int)(window.getHeight() * yf);
	int x = (window.getWidth() - _loading_border.getWidth()) / 2;
	
	int w = (int) (progress * (_loading_border.getWidth() - 2 * border));
	int w_old = (int) (old_progress * (_loading_border.getWidth() - 2 * border));
	if (w == w_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;
	}

	renderSplash(window);
	window.copyFrom(_loading_border, x, y);

	int i, n = w / _loading_item.getWidth(), n_old = w_old / _loading_item.getWidth();
	if (n == n_old) {
		//LOG_DEBUG(("skip same frame"));
		return false;	
	}
	for(i = 0; i < n; ++i) {
		window.copyFrom(_loading_item, border + x + i * _loading_item.getWidth(), y + border);
	}
/*	w -= n * _loading_item.getWidth();
	sdlx::Rect src(0, 0, w, _loading_item.getHeight());
	window.copyFrom(_loading_item, src, border + x + i * _loading_item.getWidth(), y + border);
*/
	return true;
}


Hud::Hud(const int w, const int h) : _update_radar(true) {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_background.loadImage(data_dir + "/tiles/hud_line.png");
	_loading_border.loadImage(data_dir + "/tiles/loading_border.png");
	_loading_item.loadImage(data_dir + "/tiles/loading_item.png");
	_icons.loadImage(data_dir + "/tiles/hud_icons.png");
	_splitter.loadImage(data_dir + "/tiles/hud_splitter.png");
	
	_font.load(data_dir + "/font/medium.png", sdlx::Font::AZ09);
	
	LOG_DEBUG(("searching splash... %dx%d", w, h));
	int sw = 0;
	int splash_sizes[] = { 1280, 1152, 1024, 800 };
	for(unsigned i = 0; i < sizeof(splash_sizes) / sizeof(splash_sizes[0]); ++i) {
		sw = splash_sizes[i];
		if (w >= sw) {
			break;
		}
	}
	LOG_DEBUG(("using splash %d", sw));
	_splash.loadImage(mrt::formatString("%s/tiles/splash_%d.png", data_dir.c_str(), sw));

	GET_CONFIG_VALUE("hud.radar-update-interval", float, ru, 0.1);
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
	_icons_map["mod:machinegunner"] = i++;
	_icons_map["mod:mines:regular"] = i++;
	_icons_map["special:frag"] = i++;
}

Hud::~Hud() {}


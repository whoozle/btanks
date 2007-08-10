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
#include "box.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "config.h"
#include <assert.h>

Box::Box(const std::string &tile, int w, int h) {
	init(tile, std::string(), w, h);
}

Box::Box(const std::string &tile, const std::string &highlight, int w, int h) {
	init(tile, highlight, w, h);
}

void Box::init(const std::string &tile, int _w, int _h) {
	init(tile, std::string(), _w, _h);
}

void Box::getSize(int &rw, int &rh) const {
	rw = w;
	rh = h;
}

#define TILE_SIZE 8

void Box::init(const std::string &tile, const std::string &highlight, int _w, int _h) {
	_highlight = (!highlight.empty())? ResourceManager->loadSurface(highlight): NULL;
		
	_surface = ResourceManager->loadSurface(tile);
	x1 = _surface->getWidth() / 3;
	x2 = _surface->getWidth() - x1;

	y1 = _surface->getHeight() / 3;
	y2 = _surface->getHeight() - y1;
	
	w = _w - x1 * 2;
	if (w < 0) 
		w = 0;

	h = _h - y1 * 2;
	if (h < 0) 
		h = 0;
		
	int cw = _surface->getWidth() - x1 * 2;
	int ch = _surface->getHeight() - y1 * 2;

	xn = w? ((w - 1) / cw + 1): 0;
	yn = h? ((h - 1) / cw + 1): 0;
	
	w = xn * cw + x1 * 2;
	h = yn * ch + y1 * 2;
	
	//16x blending optimization.
	_filler.createRGB(cw * TILE_SIZE, cw * TILE_SIZE, 32);
	_filler.convertAlpha();

	_filler_l.createRGB(cw, cw * TILE_SIZE, 32);
	_filler_l.convertAlpha();

	_filler_r.createRGB(cw, cw * TILE_SIZE, 32);
	_filler_r.convertAlpha();

	_filler_u.createRGB(cw * TILE_SIZE, cw, 32);
	_filler_u.convertAlpha();

	_filler_d.createRGB(cw * TILE_SIZE, cw, 32);
	_filler_d.convertAlpha();

	sdlx::Surface * foo = const_cast<sdlx::Surface *>(_surface);
	assert(foo != NULL);
	foo->setAlpha(0,0);

	sdlx::Rect u (x1,	0,	x2 - x1,	 					y1);
	sdlx::Rect cl(0,	y1, x1, 							y2 - y1);
	sdlx::Rect c (x1,	y1, x2 - x1,	 					y2 - y1);
	sdlx::Rect cr(x2,	y1, _surface->getWidth() - x2, 		y2 - y1);
	sdlx::Rect d (x1,	y2, x2 - x1,	 					_surface->getHeight() - y2);
	GET_CONFIG_VALUE("menu.debug-background-code", bool, dbc, false);
	if (dbc) {
		_filler.fill(_filler.mapRGBA(0, 255, 255, 64));
		_filler_u.fill(_filler.mapRGBA(255, 0, 0, 64));
		_filler_d.fill(_filler.mapRGBA(0, 255, 0, 64));
		_filler_l.fill(_filler.mapRGBA(0, 0, 255, 64));
		_filler_r.fill(_filler.mapRGBA(255, 255, 0, 64));
	} else {
		for(int y = 0; y < TILE_SIZE; ++y) {
			_filler_l.copyFrom(*_surface, cl, 0, y * c.h);
			_filler_r.copyFrom(*_surface, cr, 0, y * c.h);
			_filler_u.copyFrom(*_surface, u, y * c.w, 0);
			_filler_d.copyFrom(*_surface, d, y * c.w, 0);
			for(int x = 0; x < TILE_SIZE; ++x) {
				_filler.copyFrom(*_surface, c, x * c.w, y * c.h);
			}
		}
	}
	
	foo->setAlpha(255);
}

void Box::render(sdlx::Surface &surface, const int x0, const int y0) {
	assert(_surface != NULL);
	
	sdlx::Rect ul(0,	0,	x1,								y1);
	sdlx::Rect u (x1,	0,	x2 - x1,	 					y1);
	sdlx::Rect ur(x2,	0,	_surface->getWidth() - x2,		y1);

	sdlx::Rect cl(0,	y1, x1, 							y2 - y1);
	sdlx::Rect c (x1,	y1, x2 - x1,	 					y2 - y1);
	sdlx::Rect cr(x2,	y1, _surface->getWidth() - x2, 		y2 - y1);

	sdlx::Rect dl(0,	y2, x1, 							_surface->getHeight() - y2);
	sdlx::Rect d (x1,	y2, x2 - x1,	 					_surface->getHeight() - y2);
	sdlx::Rect dr(x2,	y2, _surface->getWidth() - x2,	 	_surface->getHeight() - y2);
	
	int y = y0;
	
	//upper line
	int x = x0;
	surface.copyFrom(*_surface, ul, x, y);
	x += ul.w;
	int i;
	
	const int txn = xn - (xn % TILE_SIZE), tyn = yn - (yn % TILE_SIZE);
	
	for(i = 0; i < txn; i += TILE_SIZE,  x += u.w * TILE_SIZE) 
		surface.copyFrom(_filler_u, x, y);

	for(; i < xn; ++i, x += u.w) 
		surface.copyFrom(*_surface, u, x, y);
	
	surface.copyFrom(*_surface, ur, x, y);
	y += u.h;

	//center box
	int j;
	
	//optimized patter blitting (TILE_SIZExTILE_SIZE)
	for(j = 0; j < tyn; j += TILE_SIZE, y += c.h * TILE_SIZE) {
		x = x0;
		surface.copyFrom(_filler_l, x, y);
		x += cl.w;

		for(i = 0; i < txn; i += TILE_SIZE, x += c.w * TILE_SIZE) 
			surface.copyFrom(_filler, x, y);

		for(; i < xn; ++i, x += c.w) {
			for(int j2 = 0; j2 < TILE_SIZE; ++j2) {
				surface.copyFrom(*_surface, c, x, y + j2 * c.h);
			}
		}
		
		surface.copyFrom(_filler_r, x, y);
	}



	for(; j < yn; ++j) {
		x = x0;
		surface.copyFrom(*_surface, cl, x, y);
		x += cl.w;

		for(int i = 0; i < xn; ++i, x += c.w) 
			surface.copyFrom(*_surface, c, x, y);
		
		surface.copyFrom(*_surface, cr, x, y);
		y += c.h;
	}

	
	//lower line
	x = x0;
	surface.copyFrom(*_surface, dl, x, y);
	x += dl.w;

	for(i = 0; i < txn; i += TILE_SIZE, x += d.w * TILE_SIZE) 
		surface.copyFrom(_filler_d, x, y);

	for(; i < xn; ++i, x += d.w) 
		surface.copyFrom(*_surface, d, x, y);
	surface.copyFrom(*_surface, dr, x, y);
	
}

void Box::copyTo(sdlx::Surface &surface, const int x, const int y) {
	//terrible terrible hack. do not try it at home.
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0,0);
	render(surface, x, y);
	const_cast<sdlx::Surface *>(_surface)->setAlpha(0);
}


void Box::renderHL(sdlx::Surface &surface, const int x, const int y) {
	const sdlx::Surface *bg = _highlight;
	if (bg == NULL)
		throw_ex(("highlight background was not loaded."));
	
	const int bg_w = bg->getWidth(), bg_h = bg->getHeight();
	const int bg_n = this->w / (bg_w / 3);
	const int bg_y = y - bg_h / 2 - 1;
	int bg_x = x;
			
	sdlx::Rect src(0, 0, bg_w/3, bg_h);
	surface.copyFrom(*bg, src, bg_x, bg_y);
	bg_x += bg_w / 3;
	src.x = bg_w / 3;
	
	for(int i = 0; i < bg_n - 2; ++i) {
		surface.copyFrom(*bg, src, bg_x, bg_y);
		bg_x += bg_w / 3;
	}
	
	src.x = 2 * bg_w / 3;
	surface.copyFrom(*bg, src, bg_x, bg_y);
	bg_x += bg_w / 3;
}

void Box::getMargins(int &v, int &h) const {
	v = x1;
	h = y1;
}

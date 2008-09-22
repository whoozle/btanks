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
#include "box.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "config.h"
#include <assert.h>

Box::Box(const std::string &tile, int w, int h) {
	init(tile, w, h);
}

Box::Box(const std::string &tile, int w, int h, int hl_h) {
	init(tile, w, h);
}

void Box::get_size(int &rw, int &rh) const {
	rw = w;
	rh = h;
}

#define TILE_SIZE 8

void Box::set_background(const std::string &tile) {
	int w, h;
	get_size(w, h);
	init(tile, w, h, _highlight.isNull()?0: _highlight.get_height());
}

void Box::init(const std::string &tile, int _w, int _h, int hl_h) {
	bg_tile = tile;
	_highlight.free();
	if (tile.empty()) {
		_surface = NULL;
		w = _w; 
		h = _h;
		x1 = x2 = 16; 
		y1 = y2 = 32; 
		xn = yn = 1;
		if (hl_h > 0) {
			_highlight.create_rgb(w, hl_h, 32);
			_highlight.display_format_alpha();
			_highlight.fill(_highlight.map_rgba(255, 255, 255, 77));
		}
		return;
	}
		
	_surface = ResourceManager->load_surface(tile);
	x1 = _surface->get_width() / 3;
	x2 = _surface->get_width() - x1;

	y1 = _surface->get_height() / 3;
	y2 = _surface->get_height() - y1;
	
	w = _w - x1 * 2;
	if (w < 0) 
		w = 0;

	h = _h - y1 * 2;
	if (h < 0) 
		h = 0;
		
	int cw = _surface->get_width() - x1 * 2;
	int ch = _surface->get_height() - y1 * 2;

	xn = w? ((w - 1) / cw + 1): 0;
	yn = h? ((h - 1) / cw + 1): 0;
	
	w = xn * cw + x1 * 2;
	h = yn * ch + y1 * 2;

	
	//16x blending optimization.
	_filler.create_rgb(cw * TILE_SIZE, cw * TILE_SIZE, 32);
	_filler.display_format_alpha();

	_filler_l.create_rgb(cw, cw * TILE_SIZE, 32);
	_filler_l.display_format_alpha();

	_filler_r.create_rgb(cw, cw * TILE_SIZE, 32);
	_filler_r.display_format_alpha();

	_filler_u.create_rgb(cw * TILE_SIZE, cw, 32);
	_filler_u.display_format_alpha();

	_filler_d.create_rgb(cw * TILE_SIZE, cw, 32);
	_filler_d.display_format_alpha();

	sdlx::Surface * foo = const_cast<sdlx::Surface *>(_surface);
	assert(foo != NULL);
	foo->set_alpha(0,0);

	sdlx::Rect u (x1,	0,	x2 - x1,	 					y1);
	sdlx::Rect cl(0,	y1, x1, 							y2 - y1);
	sdlx::Rect c (x1,	y1, x2 - x1,	 					y2 - y1);
	sdlx::Rect cr(x2,	y1, _surface->get_width() - x2, 		y2 - y1);
	sdlx::Rect d (x1,	y2, x2 - x1,	 					_surface->get_height() - y2);
	GET_CONFIG_VALUE("menu.debug-background-code", bool, dbc, false);
	if (dbc) {
		_filler.fill(_filler.map_rgba(0, 255, 255, 64));
		_filler_u.fill(_filler.map_rgba(255, 0, 0, 64));
		_filler_d.fill(_filler.map_rgba(0, 255, 0, 64));
		_filler_l.fill(_filler.map_rgba(0, 0, 255, 64));
		_filler_r.fill(_filler.map_rgba(255, 255, 0, 64));
	} else {
		for(int y = 0; y < TILE_SIZE; ++y) {
			_filler_l.blit(*_surface, cl, 0, y * c.h);
			_filler_r.blit(*_surface, cr, 0, y * c.h);
			_filler_u.blit(*_surface, u, y * c.w, 0);
			_filler_d.blit(*_surface, d, y * c.w, 0);
			for(int x = 0; x < TILE_SIZE; ++x) {
				_filler.blit(*_surface, c, x * c.w, y * c.h);
			}
		}
	}
	
	foo->set_alpha(255);

	if (hl_h > 0) {
		_highlight.create_rgb(w, hl_h, 32);
		_highlight.display_format_alpha();
		_highlight.fill(_highlight.map_rgba(255, 255, 255, 77));
	}
}

void Box::render(sdlx::Surface &surface, const int x0, const int y0) const {
	if (_surface == NULL)
		return;
	
	sdlx::Rect ul(0,	0,	x1,								y1);
	sdlx::Rect u (x1,	0,	x2 - x1,	 					y1);
	sdlx::Rect ur(x2,	0,	_surface->get_width() - x2,		y1);

	sdlx::Rect cl(0,	y1, x1, 							y2 - y1);
	sdlx::Rect c (x1,	y1, x2 - x1,	 					y2 - y1);
	sdlx::Rect cr(x2,	y1, _surface->get_width() - x2, 		y2 - y1);

	sdlx::Rect dl(0,	y2, x1, 							_surface->get_height() - y2);
	sdlx::Rect d (x1,	y2, x2 - x1,	 					_surface->get_height() - y2);
	sdlx::Rect dr(x2,	y2, _surface->get_width() - x2,	 	_surface->get_height() - y2);
	
	int y = y0;
	
	//upper line
	int x = x0;
	surface.blit(*_surface, ul, x, y);
	x += ul.w;
	int i;
	
	const int txn = xn - (xn % TILE_SIZE), tyn = yn - (yn % TILE_SIZE);
	
	for(i = 0; i < txn; i += TILE_SIZE,  x += u.w * TILE_SIZE) 
		surface.blit(_filler_u, x, y);

	for(; i < xn; ++i, x += u.w) 
		surface.blit(*_surface, u, x, y);
	
	surface.blit(*_surface, ur, x, y);
	y += u.h;

	//center box
	int j;
	
	//optimized patter blitting (TILE_SIZExTILE_SIZE)
	for(j = 0; j < tyn; j += TILE_SIZE, y += c.h * TILE_SIZE) {
		x = x0;
		surface.blit(_filler_l, x, y);
		x += cl.w;

		for(i = 0; i < txn; i += TILE_SIZE, x += c.w * TILE_SIZE) 
			surface.blit(_filler, x, y);

		for(; i < xn; ++i, x += c.w) {
			for(int j2 = 0; j2 < TILE_SIZE; ++j2) {
				surface.blit(*_surface, c, x, y + j2 * c.h);
			}
		}
		
		surface.blit(_filler_r, x, y);
	}



	for(; j < yn; ++j) {
		x = x0;
		surface.blit(*_surface, cl, x, y);
		x += cl.w;

		for(int i = 0; i < xn; ++i, x += c.w) 
			surface.blit(*_surface, c, x, y);
		
		surface.blit(*_surface, cr, x, y);
		y += c.h;
	}

	
	//lower line
	x = x0;
	surface.blit(*_surface, dl, x, y);
	x += dl.w;

	for(i = 0; i < txn; i += TILE_SIZE, x += d.w * TILE_SIZE) 
		surface.blit(_filler_d, x, y);

	for(; i < xn; ++i, x += d.w) 
		surface.blit(*_surface, d, x, y);
	surface.blit(*_surface, dr, x, y);
	
}

void Box::copyTo(sdlx::Surface &surface, const int x, const int y) {
	//terrible terrible hack. do not try it at home.
	const_cast<sdlx::Surface *>(_surface)->set_alpha(0,0);
	render(surface, x, y);
	const_cast<sdlx::Surface *>(_surface)->set_alpha(0);
}


void Box::renderHL(sdlx::Surface &surface, const int x, const int y) const {
	if (_highlight.isNull())
		throw_ex(("highlight background was not created."));
	
	const int bg_w = _highlight.get_width(), bg_h = _highlight.get_height();
	const int bg_n = this->w / (bg_w / 3);
	const int bg_y = y - bg_h / 2 - 1;
	int bg_x = x;
			
	sdlx::Rect src(0, 0, bg_w/3, bg_h);
	surface.blit(_highlight, src, bg_x, bg_y);
	bg_x += bg_w / 3;
	src.x = bg_w / 3;
	
	for(int i = 0; i < bg_n - 2; ++i) {
		surface.blit(_highlight, src, bg_x, bg_y);
		bg_x += bg_w / 3;
	}
	
	src.x = 2 * bg_w / 3;
	surface.blit(_highlight, src, bg_x, bg_y);
	bg_x += bg_w / 3;
}

void Box::getMargins(int &v, int &h) const {
	v = x1;
	h = y1;
}

void Box::setHLColor(int r, int g, int b, int a) {
	if (_highlight.isNull())
		return;
	_highlight.fill(_highlight.map_rgba(r, g, b, a));
}

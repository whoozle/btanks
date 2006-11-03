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

#include "credits.h"
#include "config.h"
#include "math/abs.h"
#include "math/minmax.h"

Credits::Credits() {
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.load(data_dir + "/font/big.png", sdlx::Font::AZ09, false);
	
	int fh = _font.getHeight();
	std::vector<std::string> lines; 
	lines.push_back("BATTLE TANKS");
	lines.push_back("");
	lines.push_back("PROGRAMMING");
	lines.push_back("VLADIMIR MENSHAKOV AKA MEGATH");
	lines.push_back("");
	lines.push_back("GRAPHICS");
	lines.push_back("ALEXANDER WAGNER AKA METHOS");
	lines.push_back("");
	lines.push_back("LEVEL DESIGN");
	lines.push_back("VLADIMIR ZHURAVLEV AKA VZ");
	lines.push_back("");
	
	lines.push_back("GAME DESIGN");
	lines.push_back("NETIVE MEDIA GROUP 2006");

	_h = fh * lines.size();

	size_t max_w = 0;
	for(std::vector<std::string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		if (i->size() > max_w)
			max_w = i->size();
	}
	_w = max_w * fh;
	_surface.createRGB(_w, _h, 24);
	_surface.convertAlpha();
	
	LOG_DEBUG(("credits %dx%d", _w, _h));
	
	for(size_t i = 0; i < lines.size(); ++i) {	
		const std::string &str = lines[i];
		_font.render(_surface, (_w - str.size() * fh) / 2, i * fh, str);
	}
	_velocity.x = 2;
	_velocity.y = 3;
	_velocity.normalize();
}

void Credits::render(const float dt, sdlx::Surface &surface) {
	_position += _velocity * dt * 150;
	int xmargin = math::max(_w - surface.getWidth(), 50);
	int ymargin = math::max(_h - surface.getHeight(), 50);
	
	if (_position.x < -xmargin)
		_velocity.x = math::abs(_velocity.x);
	if (_position.x + _w > surface.getWidth() + xmargin)
		_velocity.x = - math::abs(_velocity.x);

	if (_position.y < -ymargin)
		_velocity.y = math::abs(_velocity.y);
	if (_position.y + _h > surface.getHeight() + ymargin)
		_velocity.y = -math::abs(_velocity.y);
	
	surface.copyFrom(_surface, (int)_position.x, (int)_position.y);
}

Credits::~Credits() {}

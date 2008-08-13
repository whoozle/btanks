
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

#include <ctype.h>
#include "tooltip.h"

#include "mrt/logger.h"
#include "menu/box.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/font.h"
#include "mrt/utf8_utils.h"
#include "i18n.h"

#include <math.h>
#include <assert.h>
#include <deque>

Tooltip::Tooltip(const std::string &area, const std::string &message, const bool use_background, const int w)  : 
area(area), message(message), _use_background(use_background) {
	std::string text, _text = I18n->get(area, message);
	bool space = true;
	size_t i;
	for(i = 0; i < _text.size(); ++i) {
		const int c = _text[i];
		const bool c_space = c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
		//LOG_DEBUG(("%d '%c': %s %s", c, c, space?"true":"false", c_space?"true":"false"));
		if (space) {
			if (c_space)
				continue;
			space = false;
			text += c;
		} else {
			if (c_space) {
				space = true;
				text += ' ';
			} else 
				text += c;
		}
	}
	
	//LOG_DEBUG(("trimmed string : '%s'", text.c_str()));
		
	std::vector<int> lens;
	std::vector<std::string> words;
	mrt::split(words, text, " ");
	lens.resize(words.size());

	//std::string lens_dump;
	size_t sum = 0;
	for(i = 0; i < words.size(); ++i) {
		unsigned int l = mrt::utf8_length(words[i]);
		lens[i] = l;
		sum += l;
		//lens_dump += mrt::format_string("%s<<%s>>%u", (i == 0)?"":", ", words[i].c_str(), l);
	}
	//LOG_DEBUG(("sum: %u, words: %s", sum, lens_dump.c_str()));
	GET_CONFIG_VALUE("engine.tooltip-speed", float, td, 20);
	_time = ((float)mrt::utf8_length(_text)) / td;

	int cell = (int)(sqrt(sum / 2.0) + 0.5);
	int xsize = cell * 2;
	//LOG_DEBUG(("approx size : %dx%d", xsize, cell * 3));

	const sdlx::Font *font = ResourceManager->loadFont("small", false);
	assert(font != NULL);

	int mx = 0, my = 0;
	if (_use_background) {
		_background.init("menu/background_box.png", 200, 60);
		_background.getMargins(mx, my);
	}
	
	int line_h = font->get_height() + 2;

	int width = 0;
	std::deque<size_t> lines;

	for(i = 0; i < words.size(); ) {
		int l, line_size = 0;
		for(l = 0; (w != 0?line_size < w: l < xsize) && i < words.size(); l += lens[i], ++i) {
			int lw = font->render(NULL, 0, 0, words[i] + " ");
			if (w != 0 && lw + line_size > w) 
				break;
			line_size += lw;
		}
		if (line_size > width)
			width = line_size;
		lines.push_back(i);
	}


	//LOG_DEBUG(("line width: %d, lines: %u", width, lines.size()));
	if (_use_background) {
		_background.init("menu/background_box.png", width +  mx, line_h * lines.size() +  my);
		_surface.create_rgb(_background.w, _background.h, 32, SDL_SRCALPHA);
	} else {
		_surface.create_rgb(w, line_h * (lines.size() + 2/*magic! */), 32, SDL_SRCALPHA);
	}
	_surface.display_format_alpha();
	
	int yp = my - (use_background? 2:0);
	i = 0;
	while(!lines.empty()) {
		int xp = mx - (use_background? 2:0);
		size_t n = lines.front();
		while(i < n) {
			xp += font->render(_surface, xp, yp, words[i] + " ");
			++i;
		} 
		yp += line_h;
		lines.pop_front();
	}
}

void Tooltip::render(sdlx::Surface &surface, const int x, const int y) const {
	if (_use_background)
		_background.render(surface, x, y);
	surface.blit(_surface, x, y);
}

void Tooltip::get_size(int &w, int &h) const {
	w = _surface.get_width();
	h = _surface.get_height();
}


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

Tooltip::Tooltip(const std::string &area, const std::string &message, const bool use_background, int w)  : 
area(area), message(message) {
	init(I18n->get(area, message), use_background, w);
}

Tooltip::Tooltip(const std::string &area, const std::string &message, const std::string &text, const bool use_background, int w) : 
area(area), message(message) {
	init(text, use_background, w);
}

void Tooltip::init(const std::string &_text, const bool use_background, int width) {
	_use_background = use_background;
	std::string text;
	bool space = true;
	size_t i;
	for(i = 0; i < _text.size(); ++i) {
		const char c = _text[i];
		const bool c_space = c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v' || c == '\f';
		if (c == '\\' && i + 1 < _text.size() && _text[i + 1] == 'n') {
			//\n hack :)
			++i;
			if (space) {
				text += "\n ";
			} else {
				text += " \n ";
				space = true;
			}
			continue;
		}
		
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
	
	GET_CONFIG_VALUE("engine.tooltip-speed", float, td, 20);
	_time = ((float)mrt::utf8_length(text)) / td;

	std::vector<std::string> words;
	mrt::split(words, text, " ");
	std::vector<int> lens;
	lens.resize(words.size());

	const sdlx::Font * font = ResourceManager->loadFont("small", false);
	int line_h = font->get_height();

	int total = 0, nl_n = 0;
	for(size_t i = 0; i < words.size(); ++i) {
		const std::string &word = words[i];
		if (word == "\n") {
			nl_n += line_h;
			continue;
		}
			
		lens[i] = font->render(NULL, 0, 0, word + " ");
		total += lens[i] * line_h;
	}
	
	if (width == 0) {
		width = (int)round(sqrt(total * 2 / 3.0f + nl_n * nl_n / 4.0f));
	}
	
	std::vector<std::string> lines;

	int line_w = 0, real_width = 0;
	std::string line;
	for(size_t i = 0; i < words.size(); ++i) {
		const std::string &word = words[i];
		const int len = lens[i];
		line_w += len;
		
		bool nl = line_w >= width || word == "\n" || i + 1 == words.size();
		
		if (nl) {
			lines.push_back(line + word);
			if (line_w > real_width)
				real_width = line_w;
			line_w = 0;
			line.clear();
		} else {
			line += word + " ";
		}
	}
	
	//LOG_DEBUG(("line width: %d, lines: %u", width, lines.size()));
	int xp = 0, yp = 0;
	int height = line_h * lines.size();
	if (_use_background) {
		const sdlx::Surface *bg = ResourceManager->load_surface("menu/background_box.png");
		int mx = bg->get_width() / 3, my =  bg->get_height() / 3;
		_background.init("menu/background_box.png", real_width + mx * 2, height + my * 2);
		_surface.create_rgb(_background.w, _background.h, 32, SDL_SRCALPHA);
		xp = (_background.w - real_width) / 2;
		yp = (_background.h - height) / 2;
	} else {
		_surface.create_rgb(real_width, height, 32, SDL_SRCALPHA);
	}
	_surface.display_format_alpha();
	
	for(size_t i = 0; i < lines.size(); ++i) {
		font->render(_surface, xp, yp + i * line_h, lines[i]);
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

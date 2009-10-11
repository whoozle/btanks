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

#include "credits.h"
#include "config.h"
#include "math/unary.h"
#include "math/binary.h"
#include "sound/mixer.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/font.h"

Credits::Credits() : _w(0), _h(0) {
	Mixer->playSample(NULL, "menu/select.ogg", false);
	GET_CONFIG_VALUE("engine.credits-tune", std::string, tune, "glory.ogg");
	Mixer->play(tune, true);
	
	_font = ResourceManager->loadFont("big", false);
	_medium_font = ResourceManager->loadFont("medium", false);
	
	int fh = _font->get_height(), mfh = _medium_font->get_height();
	
	std::vector<std::string> lines, lines2; 

	lines.push_back("");
	lines.push_back("");
	lines.push_back("BATTLE TANKS");
	lines.push_back("");

	lines.push_back("PROGRAMMING");
	lines.push_back("VLADIMIR 'WHOOZLE' MENSHAKOV");
	lines.push_back("");

	lines.push_back("GRAPHICS");
	lines.push_back("ALEXANDER 'METHOS' WAGNER");
	lines.push_back("");

	lines.push_back("LEVEL DESIGN");
	lines.push_back("VLADIMIR 'VZ' ZHURAVLEV");
	lines.push_back("");

	lines.push_back("TOOLS");
	lines.push_back("VLADIMIR 'GOLD' GOLDOBIN");
	lines.push_back("");

	lines.push_back("SOMETHING RESEMBLING MUSIC");
	lines.push_back("VLADIMIR 'PETROVICH' VOLKOV");
	lines.push_back("");

	lines.push_back("SOUND EFFECTS");
	lines.push_back("LEONID 'DARK MATTER' VOLKOV");
	lines.push_back("");
	
	lines.push_back("GAME DESIGN");
	lines.push_back("NETIVE MEDIA GROUP 2006-2008");

	lines.push_back("");
	lines.push_back("");
	
	lines2.push_back("THE CREDITS HAVE BEEN COMPLETED IN AN ENTIRELY DIFFERENT STYLE");
	lines2.push_back("AT GREAT EXPENSE AND AT THE LAST MINUTE");
	lines2.push_back("BY A TEAM OF FORTY OR FIFTY WELL-TRAINED LLAMAS.");
	lines2.push_back("");
	lines2.push_back("");

	_h = fh * lines.size() + mfh * lines2.size();

	//copy-paste ninja was here ;)
	for(std::vector<std::string>::const_iterator i = lines.begin(); i != lines.end(); ++i) {
		unsigned w = _font->render(NULL, 0, 0, *i);
		if (w > _w)
			_w = w;
	}
	for(std::vector<std::string>::const_iterator i = lines2.begin(); i != lines2.end(); ++i) {
		unsigned w = _medium_font->render(NULL, 0, 0, *i);
		if (w > _w)
			_w = w;
	}
	_surface.create_rgb(_w, _h, 24);
	_surface.display_format_alpha();
	
	LOG_DEBUG(("credits %dx%d", _w, _h));
	
	for(size_t i = 0; i < lines.size(); ++i) {	
		const std::string &str = lines[i];
		int w = _font->render(NULL, 0, 0, str);
		_font->render(_surface, (_w - w) / 2, i * fh, str);
	}
	for(size_t i = 0; i < lines2.size(); ++i) {
		const std::string &str = lines2[i];
		int w = _medium_font->render(NULL, 0, 0, str);
		_medium_font->render(_surface, (_w - w) / 2, lines.size() * fh + i * mfh, str);
	}
	//copy-paste ninja has been done its evil deed and vanishes.
	_velocity.x = 2;
	_velocity.y = 3;
	_velocity.normalize();
}

void Credits::render(const float dt, sdlx::Surface &surface) {
	_position += _velocity * dt * 150;
	int xmargin = math::max((int)_w - surface.get_width(), 96);
	int ymargin = math::max((int)_h - surface.get_height(), 96);
	
	if (_position.x < -xmargin)
		_velocity.x = math::abs(_velocity.x);
	if (_position.x + _w > surface.get_width() + xmargin)
		_velocity.x = - math::abs(_velocity.x);

	if (_position.y < -ymargin)
		_velocity.y = math::abs(_velocity.y);
	if (_position.y + _h > surface.get_height() + ymargin)
		_velocity.y = -math::abs(_velocity.y);
	
	surface.fill(surface.map_rgb(0x10, 0x10, 0x10));
	surface.blit(_surface, (int)_position.x, (int)_position.y);
}

Credits::~Credits() {
	Mixer->playSample(NULL, "menu/return.ogg", false);
	Mixer->play();
}


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
#include <vector>

#include "sdlx/surface.h"
#include "sdlx/font.h"

#include "console.h"
#include "config.h"
#include "window.h"
#include "version.h"
#include "finder.h"
#include "resource_manager.h"
#include "mrt/utf8_utils.h"

IMPLEMENT_SINGLETON(Console, IConsole);

bool IConsole::onKey(const SDL_keysym sym, const bool pressed) {
	if (!pressed)
		return false;
	
	GET_CONFIG_VALUE("engine.enable-console", bool, ec, false);
	if (!ec) {
		_active = false; // if engine.enable-console set to false, console wont disappear
		return false;
	}

	if (!_active) {	
		if (sym.sym == SDLK_BACKQUOTE) {
			_active = true;
			return true;
		}
		return false;
	}

	delete _buffer.back().second;
	_buffer.back().second = NULL;
	
	switch(sym.sym) {

	case SDLK_ESCAPE:
	case SDLK_BACKQUOTE:
		_active = false;
		break;
	
	case SDLK_UP: _pos -= 4;
	case SDLK_DOWN: _pos += 2;
		if (_pos < 1)
			_pos = 1;
		if (_pos >= (int)(_buffer.size())) 
			_pos = _buffer.size() - 1;
		_buffer.back().first = (_pos < (int)(_buffer.size() - 1))?_buffer[_pos].first:">";
		break;

	case SDLK_BACKSPACE: {
		mrt::utf8_backspace(_buffer.back().first, _buffer.back().first.size() - 1);
		if (_buffer.back().first.empty())
			_buffer.back().first = ">";
		break;
	}			

	case SDLK_RETURN: {
			//LOG_DEBUG(("string: %s", _buffer.back().first.c_str()));
			std::vector<std::string> cmd;
			mrt::split(cmd, _buffer.back().first.substr(1), " ", 2);
			if (cmd[0].empty()) {
				print("moo :)");
				break;		
			}
			//LOG_DEBUG(("emit %s('%s')", cmd[0].c_str(), cmd[1].c_str()));
			std::string r = on_command.emit(cmd[0], cmd[1]);
			if (r.empty())
				r = mrt::formatString("unknown command '%s'", cmd[0].c_str());
			print(r);
			_pos = _buffer.size() - 1;
		}
		break;

	default: {
		std::string &line = _buffer.back().first;
		if (sym.unicode >= SDLK_SPACE)
			mrt::utf8_add_wchar(line, sym.unicode);
		}
	} 
	return true;
}

void IConsole::print(const std::string &msg) {
	_buffer.push_back(Buffer::value_type(msg, NULL));
	_buffer.push_back(Buffer::value_type(">", NULL));	
}

IConsole::IConsole() : _active(false), _pos(0) {}

void IConsole::init() {
	GET_CONFIG_VALUE("engine.enable-console", bool, ec, false);
	if (!ec) {
		_active = false; // if engine.enable-console set to false, console wont disappear
		return;
	}

	_font = ResourceManager->loadFont("small", false);

	LOG_DEBUG(("loading background..."));
	_background.init("menu/background_box.png", 600, 240);
	
	_buffer.push_back(Buffer::value_type(mrt::formatString("BattleTanks. version: %s", getVersion().c_str()), NULL));
	_buffer.push_back(Buffer::value_type(std::string(">"), NULL));
	LOG_DEBUG(("connecting signal..."));
	Window->key_signal.connect(sigc::mem_fun(this, &IConsole::onKey));	
}

void IConsole::render(sdlx::Surface &window) {
	if (!_active)
		return;
	int w = _background.w, h =  _background.h;
	int x = window.getWidth() - w, y = window.getHeight() - h;

	const int y_margin = 8;
	const int x_margin = 8;
	
	_background.render(window, x, y);
	window.setClipRect(sdlx::Rect(x, y + y_margin, w, h - 2 * y_margin));
	int ch = 0;
	for(Buffer::iterator i = _buffer.begin(); i != _buffer.end(); ++i) {
		if (i->second == NULL) {
			i->second = new sdlx::Surface;
			_font->render(*i->second, i->first);
			i->second->convertAlpha();
		}
		ch += i->second->getHeight();
	}

	y = window.getHeight() - ch - y_margin;
	x += x_margin; 
	
	for(Buffer::iterator i = _buffer.begin(); i != _buffer.end(); ++i) {
		window.copyFrom(*i->second, x, y);
		y += i->second->getHeight();
	}
	window.resetClipRect();
}

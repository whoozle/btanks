#include "console.h"
#include "config.h"
#include "game.h"
#include "sdlx/color.h"
#include <vector>
#include "version.h"

bool Console::onKey(const SDL_keysym sym) {
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
		std::string &line = _buffer.back().first;
		if (line.size() > 1)
			line.resize(line.size() - 1);
		
		break;
	}			

	case SDLK_RETURN: {
			//LOG_DEBUG(("string: %s", _buffer.back().first.c_str()));
			std::vector<std::string> cmd;
			mrt::split(cmd, _buffer.back().first.substr(1), " ", 2);
			LOG_DEBUG(("emit %s('%s')", cmd[0].c_str(), cmd[1].c_str()));
			std::string r = on_command.emit(cmd[0], cmd[1]);
			if (r.empty())
				r = mrt::formatString("unknown command '%s'", cmd[0].c_str());
			_buffer.push_back(Buffer::value_type(r, NULL));
			_buffer.push_back(Buffer::value_type(std::string(">"), NULL));
			_pos = _buffer.size() - 1;
		}
		break;

	default: {
		std::string &line = _buffer.back().first;
		line += (char)sym.sym;	
		}
	} 
	return true;
}

Console::Console() : _active(false), _pos(0) {
	LOG_DEBUG(("loading font..."));
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	_font.open(data_dir + "/font/Verdana.ttf", 12);

	LOG_DEBUG(("loading background..."));
	_background.loadImage(data_dir + "/tiles/console_background.png");
	
	_buffer.push_back(Buffer::value_type(mrt::formatString("BattleTanks. version: %s", getVersion().c_str()), NULL));
	_buffer.push_back(Buffer::value_type(std::string(">"), NULL));
	Game->key_signal.connect(sigc::mem_fun(this, &Console::onKey));	
}

void Console::render(sdlx::Surface &window) {
	if (!_active)
		return;
	int w = _background.getWidth(), h =  _background.getHeight();
	int x = window.getWidth() - w, y = window.getHeight() - h;

	const int y_margin = 8;
	const int x_margin = 8;
	
	window.copyFrom(_background, x, y);
	window.setClipRect(sdlx::Rect(x, y + y_margin, w, h - 2 * y_margin));
	int ch = 0;
	for(Buffer::iterator i = _buffer.begin(); i != _buffer.end(); ++i) {
		if (i->second == NULL) {
			i->second = new sdlx::Surface;
			_font.renderBlended(*i->second, i->first, sdlx::Color(0, 200, 0));
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

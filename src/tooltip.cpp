#include "tooltip.h"
#include "mrt/logger.h"
#include <ctype.h>
#include "menu/box.h"

Tooltip::Tooltip(const std::string &_text) {
	std::string text;
	bool space = true;
	for(size_t i = 0; i < _text.size(); ++i) {
		const int c = _text[i];
		const bool c_space = isspace(c) != 0;
		if (space) {
			if (c_space)
				continue;
			space = false;
			text += c;
		} else {
			if (!c_space)
				space = false;
			text += c;
		}
	}
	
	LOG_DEBUG(("trimmed string : '%s'", text.c_str()));
		
	std::vector<int> lens;
	std::vector<std::string> words;
	mrt::split(words, text, " ");
	lens.resize(words.size());

	std::string lens_dump;
	for(size_t i = 0; i < words.size(); ++i) {
		unsigned int l = words[i].size();
		lens[i] = l;
		lens_dump += mrt::formatString("%s%u", (i == 0)?"":", ", l);
	}
	LOG_DEBUG(("words: %s", lens_dump.c_str()));
	
	Box background;
	background.init("menu/background_box.png", 200, 60);
	_surface.createRGB(background.w, background.h, SDL_SRCALPHA);
	_surface.convertAlpha();
	background.copyTo(_surface, 0, 0);
}

void Tooltip::render(sdlx::Surface &surface, const int x, const int y) {
	surface.copyFrom(_surface, x, y);
}

void Tooltip::getSize(int &w, int &h) {
	w = _surface.getWidth();
	h = _surface.getHeight();
}

#include "tooltip.h"
#include "mrt/logger.h"
#include <ctype.h>

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
	
	_background.init("menu/background_box.png", 200, 60);
}

void Tooltip::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
}

void Tooltip::getSize(int &w, int &h) {
	w = _background.w;
	h = _background.h;
}

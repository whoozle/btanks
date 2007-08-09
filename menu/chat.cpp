#include "chat.h"
#include "resource_manager.h"
#include "menu/text_control.h"
#include "sdlx/font.h"

Chat::Chat(const size_t lines) : _font(ResourceManager->loadFont("small", true)), nick_w(0), lines(lines) {
	add(4, 0, _input = new TextControl("small"));
}

void Chat::render(sdlx::Surface &surface, const int x, const int y) {
	int h = _font->getHeight();
	for(size_t i = 0; i < text.size(); ++i) {
		_font->render(surface, x + 4, y + h * i, text[i].first);
		_font->render(surface, x + 4 + nick_w, y + h * i, text[i].second);
	}
	if (!hidden())
		Container::render(surface, x, y);
}

void Chat::layout() {
	int xp = 4;
	int yp = _font->getHeight() * text.size();
	setBase(_input, xp, yp);
}


void Chat::addMessage(const std::string &nick, const std::string &m) {
	//LOG_DEBUG(("addMessage('%s', '%s')", nick.c_str(), m.c_str()));
	const std::string n = "<" + nick + ">";
	text.push_back(std::pair<std::string, std::string>(n, m));
	if (text.size() > lines)
		text.erase(text.begin());
	
	size_t nw = _font->render(NULL, 0, 0, n);
	if (nw > nick_w)
		nick_w = nw;
	layout();
}


bool Chat::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_RETURN: 
		last_message = _input->get();
					
	case SDLK_ESCAPE: 
		if (sym.sym == SDLK_ESCAPE) 
			last_message.clear();
		
		_input->set(std::string());
		invalidate(true);
		return true;
	default: 
		Container::onKey(sym);
	}
	return true;
}


#include "chat.h"
#include "resource_manager.h"
#include "menu/text_control.h"
#include "sdlx/font.h"
#include "player_slot.h"
#include "team.h"

Chat::Chat() :nick_w(0), lines(10) {
	//GET_CONFIG_VALUE("multiplayer.chat.lines-number", int, lines, 6);
	_font[0] = ResourceManager->loadFont("small", true);
	for(int t = 0; t < 4; ++t)
		_font[t + 1] = ResourceManager->loadFont(mrt::format_string("small_%s", Team::get_color((Team::ID)t)), true);
	 
	add(4, 0, _input = new TextControl("small"));
}

void Chat::clear() {
	text.clear();
	nick_w = 0;
	_input->set(std::string());
	last_message.clear();
	hide();
	layout();
}

void Chat::render(sdlx::Surface &surface, const int x, const int y) const {
	int ybase = 0;
	for(Text::const_iterator i = text.begin(); i != text.end(); ++i) {
		const Line &line = *i;
		if (!line.nick.empty()) {
			line.font->render(surface, x + 4, y + ybase, line.nick);
			line.font->render(surface, x + 4 + nick_w, y + ybase, line.message);
		} else {
			line.font->render(surface, x + 4, y + ybase, line.message);
		}
		ybase += line.font->get_height();
	}
	if (!hidden())
		Container::render(surface, x, y);
}

void Chat::layout() {
	int xp = 4;
	int yp = 0;
	nick_w = 0;
	for(Text::const_iterator i = text.begin(); i != text.end(); ++i) {
		const Line &line = *i;
		if (!line.nick.empty()) { 
			int w = line.font->render(NULL, 0, 0, line.nick);
			if (w > nick_w)
				nick_w = w;
		}
		yp += line.font->get_height();
	}
	_input->set_base(xp, yp);
}

void Chat::addAction(const std::string &m) {
	Line line(std::string(), "*" + m, _font[0]);
	text.push_back(line);
	
	if (text.size() > lines)
		text.erase(text.begin());
	
	layout();
}



void Chat::add_message(const PlayerSlot &slot, const std::string &m) {
	const std::string n = "<" + slot.name + ">";
	int idx = (int)slot.team + 1;
	assert(idx >= 0 && idx < 5);

	Line line(n, m, _font[idx]);
	text.push_back(line);
	
	if (text.size() > lines)
		text.erase(text.begin());
	
	layout();
}


bool Chat::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_KP_ENTER:
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

void Chat::tick(const float dt) {
	Container::tick(dt);

	bool do_layout = false;
	float max = 10;
	for(std::deque<Line>::iterator i = text.begin(); i != text.end();) {
		i->t += dt;
		if (i->t >= max) {
			i = text.erase(i);
			do_layout = true;
		} else {
			++i;
		}
	}
	if (do_layout)
		layout();
}

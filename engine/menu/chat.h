#ifndef BTANKS_CHAT_H__
#define BTANKS_CHAT_H__

#include "container.h"
#include <deque>
#include <string>

namespace sdlx {
	class Font;
}

class TextControl;

class Chat : public Container {
public:
	Chat(const size_t lines);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	void addMessage(const std::string &nick, const std::string &text);
	const std::string get() const { return last_message; }
	void clear();
	
private: 
	void layout();
	
	const sdlx::Font *_font;
	TextControl *_input;
	std::deque<std::pair<std::string, std::string> > text;
	size_t nick_w, lines;
	std::string last_message;
};


#endif


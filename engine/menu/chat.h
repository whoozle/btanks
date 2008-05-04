#ifndef BTANKS_CHAT_H__
#define BTANKS_CHAT_H__

#include "container.h"
#include <deque>
#include <string>

namespace sdlx {
	class Font;
}

class TextControl;
class PlayerSlot;

class Chat : public Container {
public:
	Chat(const size_t lines);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onKey(const SDL_keysym sym);
	void addMessage(const PlayerSlot &slot, const std::string &text);
	void addAction(const std::string &text);
	const std::string get() const { return last_message; }
	void clear();
	
private: 
	void layout();
	
	const sdlx::Font *_font[5];
	TextControl *_input;
	struct Line {
		Line() : font(NULL) {}
		Line(const std::string &nick, const std::string &message, const sdlx::Font *font): 
			nick(nick), message(message), font(font) {}
		
		std::string nick, message;
		const sdlx::Font *font;
	};
	typedef std::deque<Line> Text;
	Text text;
	int nick_w;
	size_t lines;
	std::string last_message;
};


#endif


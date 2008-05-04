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
	Chat();
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onKey(const SDL_keysym sym);
	void tick(const float dt);
	const std::string get() const { return last_message; }

	void addMessage(const PlayerSlot &slot, const std::string &text);
	void addAction(const std::string &text);
	void clear();
	
	
private: 
	void layout();
	
	const sdlx::Font *_font[5];
	TextControl *_input;
	struct Line {
		Line() : font(NULL), t(0) {}
		Line(const std::string &nick, const std::string &message, const sdlx::Font *font): 
			nick(nick), message(message), font(font), t(0) {}
		
		std::string nick, message;
		const sdlx::Font *font;
		float t;
	};
	typedef std::deque<Line> Text;
	Text text;
	int nick_w;
	size_t lines;
	std::string last_message;
};


#endif


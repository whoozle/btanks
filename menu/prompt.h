#ifndef BTANKS_MENU_PROMPT_H__
#define BTANKS_MENU_PROMPT_H__

#include "container.h"
#include "box.h"
#include "sdlx/rect.h"
#include <string>

class TextControl;
class Button;

class Prompt : public Container {
public: 
	Prompt(const int w, const int h, TextControl *text);
	void getSize(int &w , int &h) const;

	void set(const std::string &value);
	const std::string &get() const;

	virtual void render(sdlx::Surface& surface, const int x, const int y);
	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	~Prompt();
private: 
	Box _background;
	sdlx::Rect _text_rect;
	Button *_b_ok, *_b_back;
	TextControl * _text;
	std::string value;
};


#endif


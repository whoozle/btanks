#ifndef BTANKS_MENU_PROMPT_H__
#define BTANKS_MENU_PROMPT_H__

#include "container.h"
#include "box.h"
#include <string>

class TextControl;

class Prompt : public Container {
public: 
	Prompt(const int w, const int h, TextControl *text);
	void getSize(int &w , int &h) const;

	void set(const std::string &value);
	const std::string &get() const;

	virtual void render(sdlx::Surface& surface, const int x, const int y);
	virtual void tick(const float dt);

private: 
	Box _background;
	TextControl * _text;
};


#endif


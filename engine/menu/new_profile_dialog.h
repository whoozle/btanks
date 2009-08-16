#ifndef BTANKS_MENU_NEW_PROFILE_DIALOG_H__
#define BTANKS_MENU_NEW_PROFILE_DIALOG_H__

#include "container.h"
#include "text_control.h"

class Button;

class NewProfileDialog : public Container {
public: 
	NewProfileDialog();
	const std::string &get() const { return _name->get(); }
	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
private: 
	TextControl * _name;
	Button *_ok;
};

#endif


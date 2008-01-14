#ifndef BTANKS_EDITOR_OPEN_MAP_DIALOG_H__
#define BTANKS_EDITOR_OPEN_MAP_DIALOG_H__

#include "menu/container.h"
#include "menu/box.h"
#include <map>

class Button;
class Chooser;
class Prompt;
class NumberControl;

class OpenMapDialog : public Container {
public: 
	OpenMapDialog();
	
	virtual void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	
	void load();
	void getMap(std::string &dir, std::string &name) const;

private: 
	Chooser *c_base, *c_map;
	Button  *b_ok, *b_back, *b_new;
	Prompt  *p_name;
	NumberControl *n_width, *n_height;
	std::string base, map;
	std::multimap<const std::string, std::string> _maps;
	std::map<const std::string, Chooser *> _map_chooser;
};

#endif


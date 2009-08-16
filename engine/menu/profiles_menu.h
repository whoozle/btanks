#ifndef BTANKS_PROFILES_MENU_H__
#define BTANKS_PROFILES_MENU_H__

#include "container.h"
#include <vector>
#include <string>

class ScrollList;
class Button;
class NewProfileDialog;

class ProfilesMenu : public Container {
public:
	ProfilesMenu(const int w, const int h);
	void init();
	virtual void tick(const float dt);
	void save();

private:
	virtual bool onKey(const SDL_keysym sym);
	std::vector<std::string> _ids;
	ScrollList * _list;
	NewProfileDialog * _new_profile;
	Button * _ok, * _add, * _remove;
};

#endif

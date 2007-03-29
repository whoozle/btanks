#ifndef BTANKS_OPTIONS_MENU_H__
#define BTANKS_OPTIONS_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"
#include "box.h"
#include "alarm.h"


class Button;
class Slider;
class MainMenu;
class ControlPicker;
class Object;

class OptionsMenu : public BaseMenu {
public:
	OptionsMenu(MainMenu *parent, const int w, const int h);
	~OptionsMenu();
	
	void getSize(int &w, int &h) const;
	void tick(const float dt);
	
	void reload();
	void save();
	
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &dst, const int x, const int y);

private: 
	MainMenu *_parent;
	ControlPicker *sp, *sp1, *sp2;

	Box _background;
	int _bx, _by;
	Button *_b_ok, *_b_back;
	Slider *_fx, *_music;

	Object *_shooter; //hack to allow mixer playing sample
	Alarm _shoot;
};

#endif


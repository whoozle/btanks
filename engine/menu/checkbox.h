#ifndef BTANKS_MENU_CHECKBOX_H__
#define BTANKS_MENU_CHECKBOX_H__

#include "control.h"

class BTANKSAPI Checkbox : public Control {
public: 
	Checkbox(const bool state = false);
	const bool get() const;
	void set(const bool value); 

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
private: 
	bool _state;
	const sdlx::Surface *_checkbox;
};

#endif


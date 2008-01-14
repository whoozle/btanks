#ifndef BTANKS_MENU_RESIZE_DIALOG_H__
#define BTANKS_MENU_RESIZE_DIALOG_H__

#include "menu/container.h"
class NumberControl;

class ResizeDialog : public Container {
public: 
	ResizeDialog();
	const bool get(int &left, int &right, int &up, int &down) const;
	void show();
private: 
	virtual bool onKey(const SDL_keysym sym);
	void resize();
	
	mutable bool done;
	NumberControl * c_l, * c_r, * c_u, * c_d;
};

#endif


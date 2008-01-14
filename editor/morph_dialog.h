#ifndef BTANKS_EDITOR_MORPH_DIALOG_H__
#define BTANKS_EDITOR_MORPH_DIALOG_H__

#include "menu/scroll_list.h"

class MorphDialog  : public ScrollList {
public: 
	MorphDialog(const int w, const int h);
	void initMap();
	virtual bool onKey(const SDL_keysym sym); 
};

#endif


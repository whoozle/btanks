#ifndef BTANKS_EDITOR_MORPH_DIALOG_H__
#define BTANKS_EDITOR_MORPH_DIALOG_H__

#include "menu/scroll_list.h"
#include "sl08/sl08.h"

class MorphDialog  : public ScrollList {
public: 
	MorphDialog(const int w, const int h);
	void initMap();
	sl08::slot0<void, MorphDialog> init_map_slot;
	virtual bool onKey(const SDL_keysym sym); 
};

#endif


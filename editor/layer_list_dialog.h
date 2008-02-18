#ifndef BTANKS_LAYER_LIST_DIALOG_H__
#define BTANKS_LAYER_LIST_DIALOG_H__

#include "menu/scroll_list.h"
#include "sl08/sl08.h"

class LayerItem;
class Prompt;

class LayerListDialog : public ScrollList {
public: 
	LayerListDialog(const int w, const int h);
	virtual bool onKey(const SDL_keysym sym); 
	virtual void tick(const float dt);
	
	const bool active() const;
	
	const LayerItem * getCurrentItem() const;
	const LayerItem * getItem(const int idx) const;

private: 
	void initMap();
	sl08::slot0<void, LayerListDialog> init_map_slot;
	Prompt *_new_layer;
};

#endif


#ifndef BTANKS_MENU_LAYER_ITEM_H__
#define BTANKS_MENU_LAYER_ITEM_H__

#include "menu/container.h"

class Layer;
class Checkbox;

class LayerItem : public Container {
public: 
	int z;
	Layer *layer;
	LayerItem(const int z, Layer *layer);

	virtual void tick(const float dt);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	Checkbox *_c_show, *_c_solo;
};

#endif

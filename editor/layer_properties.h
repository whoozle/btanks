#ifndef BTANKS_EDITOR_LAYER_PROPS_H__
#define BTANKS_EDITOR_LAYER_PROPS_H__

#include "container.h"

class LayerProperties : public Container {
public:
	LayerProperties();
private: 
	TextControl *name, *z, *ai_hint, *hp;
	Checkbox *pierceable, *hidden, *visible_if_damaged, *invisible_if_damaged;
	ScrollList *damage_for;
};

#endif



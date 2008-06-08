#include "layer_item.h"
#include "mrt/logger.h"
#include "tmx/layer.h"
#include "menu/label.h"
#include "menu/checkbox.h"
#include <assert.h>

LayerItem::LayerItem(const int z, Layer *layer) : z(z), layer(layer) {
	assert(layer != NULL);
	
	int cw, ch;
	const std::string name = mrt::format_string("%3d: %s", z, layer->name.c_str());
	int xp = 0;
	
	_c_show = new Checkbox(layer->visible);
	_c_show->get_size(cw, ch);
	add(xp, 0, _c_show);
	xp += cw + 4;

	_c_solo = new Checkbox();
	_c_show->get_size(cw, ch);
	add(xp, 0, _c_solo);
	xp += cw + 4;
	
	int max_h = ch;
	
	Label *l = new Label("small", name);
	l->get_size(cw, ch);
	add(xp, (max_h - ch) / 2, l);
	xp += cw + 4;
}

bool LayerItem::onMouse(const int button, const bool pressed, const int x, const int y) {
	bool r = Container::onMouse(button, pressed, x, y);
	if (r && in(_c_solo, x, y)) {
		return false;
	}
	return r;
}


void LayerItem::tick(const float dt) {
	Container::tick(dt);
	if (_c_show->changed()) {
		_c_show->reset();
		layer->visible = _c_show->get();
		//LOG_DEBUG(("layer %s %s", _layer->name.c_str(), _layer->visible?"visible":"invisible"));
	}
	if (_c_solo->changed()) {
		_c_solo->reset();
		layer->solo = _c_solo->get();	
	}
}

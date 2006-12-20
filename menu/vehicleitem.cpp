#include "vehicleitem.h"
#include "config.h"

#include "resource_manager.h"

VehicleItem::VehicleItem(sdlx::TTF &font, const std::string &name, const std::string &subkey) :
 MenuItem(font, name, "text", std::string(), std::string()), _active(false), _subkey(subkey) {
	//load map.
	mrt::toUpper(_subkey);

	_items.push_back("tank");
	_items.push_back("launcher");
	_items.push_back("shilka");
		
	std::sort(_items.begin(), _items.end());

	std::string vehicle;
	Config->get("menu.default-vehicle-" + subkey, vehicle, "launcher");
	for(_index = 0; _index < _items.size(); ++_index) {
		if (_items[_index] == vehicle)
			break;
	}
	if (_index >= _items.size())
		_index = 0;
	updateValue();
}

#include "animation_model.h"
#include <assert.h>

void VehicleItem::render(sdlx::Surface &dst, const int x, const int y) {
	if (_active) {
		const std::string name = "green-" + _items[_index];
		const Animation *a = ResourceManager.get_const()->getAnimation(name);
		assert(a != NULL);
		
		const sdlx::Surface * surface = ResourceManager.get_const()->getSurface(a->surface);
		const sdlx::CollisionMap *cmap = NULL;
		ResourceManager->checkSurface(a->surface, surface, cmap);
		int w, h;
		getSize(w, h);
		sdlx::Rect src(0, 0, a->tw, a->th);
		dst.copyFrom(*surface, src, x + w / 2 - a->tw / 2 , y - a->th - 10);
	}
	
	MenuItem::render(dst, x, y);
}

void VehicleItem::onClick() {
	_active = true;
}

void VehicleItem::finish() {
	_active = false;
}

void VehicleItem::updateValue() {
	_text = _value = _items[_index];
	std::string n(name);
	mrt::toUpper(n);
	mrt::toUpper(_text);
	_text = n + " : " + _text;
	MenuItem::render();

	Config->set("menu.default-vehicle-" + _subkey, _value);
}


const bool VehicleItem::onKey(const Uint8 type, const SDL_keysym sym) {
	if (!_active || type != SDL_KEYDOWN)
		return false;
	
	switch(sym.sym) {
	case SDLK_RIGHT: 
		_index = (_index + 1) % _items.size();
		updateValue();
		break;
	case SDLK_LEFT: 
		_index = (_index + _items.size() - 1) % _items.size();
		updateValue();
		break;
	case SDLK_ESCAPE: 
		finish();
		break;
	case SDLK_RETURN: 
		finish();
		break;
	default: ;	
	}
	return true;
}

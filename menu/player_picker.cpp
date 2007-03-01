#include "player_picker.h"
#include "container.h"
#include "sdlx/font.h"
#include "resource_manager.h"
#include "chooser.h"
#include "label.h"
#include "map_desc.h"
#include "menu_config.h"
#include "config.h"

class SlotLine : public Container {
	Chooser *_type, *_vehicle;
public : 
	int h, ch;
	std::string map, variant;
	int slot;
	SlotConfig config;
	
	SlotLine(const std::string &map, const std::string &variant, const int i, const SlotConfig &config) : 
	map(map), variant(variant), slot(i), config(config) {
		
		_font = ResourceManager->loadFont("medium", true);
		h = _font->getHeight();
		int w = _font->getWidth();

		std::vector<std::string> options;
		options.push_back("?");

		if (variant =="split") {
			options.push_back("PLAYER-1");
			options.push_back("PLAYER-2");			
			options.push_back("AI");
		} else {
			options.push_back("PLAYER");
			options.push_back("AI");
		}

		_type = new Chooser("medium", options);
		if(!config.type.empty())
			_type->set(config.type);
		_vehicle = new Chooser("menu/vehicles.png", 5);
		
		int cw;
		_type->getSize(cw, ch);

		add(sdlx::Rect(0, (ch - h) / 3, w, h), new Label(_font, mrt::formatString("%d", i + 1)));


		sdlx::Rect p1;
		p1.x = w * 2;
		//p1.y = (_font->getHeight() - ch) / 2;
		p1.w = cw;
		p1.h = ch;
		if (ch > h) 
			h = ch;
		
		add(p1, _type);
		
		sdlx::Rect p2;
		p2.x = p1.x + p1.w + _font->getWidth();

		int vcw, vch;
		_vehicle->getSize(vcw, vch);
		if (vch > h) 
			h = vch;
		p2.w = vcw; p2.h = vch;

		add(p2, _vehicle);
	}

	void tick(const float dt) {
		if (_type->changed()) {
			_type->reset();
			config.type = _type->getValue();
			MenuConfig->update(map, variant, slot, config);
			_changed = true;
			//LOG_DEBUG(("type changed"));
		}
		if (_vehicle->changed()) {
			_vehicle->reset();
			_changed = true;
			//LOG_DEBUG(("vehicle changed"));
		}
	}

private: 
	const sdlx::Font *_font;
};

PlayerPicker::PlayerPicker(const int w, const int h)  {
	_background.init("menu/background_box.png", w, h);
	_vehicles = ResourceManager->loadSurface("menu/vehicles.png");
}

const std::string PlayerPicker::getVariant() const {
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	return split?"split":std::string();
}

void PlayerPicker::tick(const float dt) {
	bool validate = false;
	for(size_t i = 0; i < _slots.size(); ++i) {
		SlotLine *slot = _slots[i];
		if (slot->changed()) {
			slot->reset();
			validate = true;
		}
	}
	if (validate) {
		LOG_DEBUG(("validation!"));
	}
	Container::tick(dt);
}

void PlayerPicker::set(const MapDesc &map) {
	clear();
	int mx, my;
	_background.getMargins(mx, my);
	_object = map.object;

	std::vector<SlotConfig> config;

	std::string variant = getVariant();
	
	MenuConfig->fill(map.name, variant, config);
	config.resize(map.slots);

	_slots.clear();
	
	for(int i = 0; i < map.slots; ++i) {
		SlotLine *line = new SlotLine(map.name, variant, i, config[i]);
		_slots.push_back(line);
		sdlx::Rect pos(mx, my + i * (line->h + 6), _background.w - 2 * mx, line->h);
		add(pos, line);
	}
}

void PlayerPicker::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	
	int mx, my;
	_background.getMargins(mx, my);
	Container::render(surface, x, y);
}

bool PlayerPicker::onKey(const SDL_keysym sym) {
	return Container::onKey(sym);
}

bool PlayerPicker::onMouse(const int button, const bool pressed, const int x, const int y)  {
	return Container::onMouse(button, pressed, x, y);
}

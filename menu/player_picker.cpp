#include "player_picker.h"
#include "container.h"
#include "sdlx/font.h"
#include "resource_manager.h"
#include "chooser.h"
#include "label.h"
#include "map_desc.h"

class SlotLine : public Container {
public : 
	int h, ch;
	
	SlotLine(const int i) {
		_font = ResourceManager->loadFont("medium", true);
		h = _font->getHeight();
		int w = _font->getWidth();

		std::vector<std::string> options;
		options.push_back("?");
		options.push_back("PLAYER");
		options.push_back("AI");

		Chooser *ic = new Chooser("medium", options);
		Chooser *vc = new Chooser("menu/vehicles.png", 5);
		
		int cw;
		ic->getSize(cw, ch);

		add(sdlx::Rect(0, (ch - h) / 3, w, h), new Label(_font, mrt::formatString("%d", i)));


		sdlx::Rect p1;
		p1.x = w * 2;
		//p1.y = (_font->getHeight() - ch) / 2;
		p1.w = cw;
		p1.h = ch;
		if (ch > h) 
			h = ch;
		
		add(p1, ic);
		
		sdlx::Rect p2;
		p2.x = p1.x + p1.w + _font->getWidth();

		int vcw, vch;
		vc->getSize(vcw, vch);
		if (vch > h) 
			h = vch;
		p2.w = vcw; p2.h = vch;

		add(p2, vc);
	}

private: 
	const sdlx::Font *_font;
};

PlayerPicker::PlayerPicker(const int w, const int h)  : _slots(0) {
	_background.init("menu/background_box.png", w, h);
	_vehicles = ResourceManager->loadSurface("menu/vehicles.png");
}

void PlayerPicker::set(const MapDesc &map) {
	clear();
	int mx, my;
	_background.getMargins(mx, my);
	_slots = map.slots;
	_object = map.object;
	for(int i = 0; i < _slots; ++i) {
		SlotLine *line = new SlotLine(i + 1);
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

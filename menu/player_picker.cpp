#include "player_picker.h"
#include "container.h"
#include "sdlx/font.h"
#include "resource_manager.h"

class SlotLine : public Container {
public : 
	int h;
	
	SlotLine(const int i) : _label(mrt::formatString("%d", i)) {
		_font = ResourceManager->loadFont("medium", true);
		h = _font->getHeight();
	}
	virtual void render(sdlx::Surface &surface, const int x, const int y) {
		int xp = _font->render(surface, x, y, _label);
		Container::render(surface, x + xp, y);
	}

private: 
	const sdlx::Font *_font;
	std::string _label;
};

PlayerPicker::PlayerPicker(const int w, const int h, const int slots)  : _slots(slots) {
	_background.init("menu/background_box.png", w, h);
	int mx, my;
	_background.getMargins(mx, my);
	
	for(int i = 0; i < _slots; ++i) {
		SlotLine *line = new SlotLine(i + 1);
		sdlx::Rect pos(mx, my + i * (line->h + 6), w - mx, line->h);
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
	return false;
}

bool PlayerPicker::onMouse(const int button, const bool pressed, const int x, const int y)  {
	return false;
}

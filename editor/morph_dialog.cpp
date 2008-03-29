#include "morph_dialog.h"
#include "tmx/map.h"
#include "tmx/generator.h"

MorphDialog::MorphDialog(const int w, const int h) : ScrollList("menu/background_box_dark.png", "small", w, h) {
	init_map_slot.assign(this, &MorphDialog::initMap, Map->load_map_signal);
}

void MorphDialog::initMap() {
	LOG_DEBUG(("refresh boxes list"));
	clear();
	std::deque<std::pair<std::string, std::string> > boxes;
	Map->getGenerator()->getPrimaryBoxes(boxes);
	for(size_t i = 0; i < boxes.size(); ++i) {
		append(boxes[i].first + ", " + boxes[i].second);
	}
	reset();
}

bool MorphDialog::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_ESCAPE: 
		hide();
		return true;
		
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
		hide();
		invalidate();
		return true;

	default: 
		return Container::onKey(sym);
	}
}

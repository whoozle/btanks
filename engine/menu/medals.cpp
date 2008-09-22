#include "medals.h"
#include "box.h"
#include "game_monitor.h"
#include "resource_manager.h"
#include "campaign.h"

Medals::Medals(int w, int h) : campaign(NULL) {
	_modal = true;
	add(0, 0, new Box("menu/background_box_dark.png", w, h));
}

void Medals::hide(const bool hide) {
	Container::hide(hide);
	if (hide) {
		if (campaign != NULL) {
			LOG_DEBUG(("unloading resources"));
			for(size_t i = 0; i < campaign->medals.size(); ++i) {
				ResourceManager->unload_surface(campaign->medals[i].tile);
			}
			tiles.clear();
		}
		return;
	}
	if (campaign == NULL) 
		throw_ex(("campaign == NULL"));

	tiles.resize(campaign->medals.size());
	for(size_t i = 0; i < tiles.size(); ++i) {
		tiles[i] = ResourceManager->load_surface(campaign->medals[i].tile);
	}
}

bool Medals::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
	case SDLK_RETURN: 
		hide();
		return true;
	default: 
		return true;
	}
}
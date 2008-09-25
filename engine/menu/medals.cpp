#include "medals.h"
#include "box.h"
#include "game_monitor.h"
#include "resource_manager.h"
#include "campaign.h"
#include "label.h"
#include "tooltip.h"
#include "image.h"
#include <assert.h>
#include "config.h"

Medals::Medals(int w, int h) : _w(w), _h(h), campaign(NULL), active(0) {
	_modal = true;
	add(0, 0, background = new Box("menu/background_box_dark.png", w, h));
	
	title = new Label("big", std::string());
	add(0, 0, title);

	numbers = new Label("big", "?/?");
	add(0, 0, numbers);

	hint = NULL;
}

void Medals::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
}

void Medals::hide(const bool hide) {
	Container::hide(hide);
	if (hide) {
		if (campaign != NULL) {
			LOG_DEBUG(("unloading resources"));
			for(size_t i = 0; i < campaign->medals.size(); ++i) {
				ResourceManager->unload_surface(campaign->medals[i].tile);
			}
			for(size_t i = 0; i < tiles.size(); ++i) {
				remove(tiles[i]);
			}
			tiles.clear();
		}
		return;
	}
	if (campaign == NULL) 
		throw_ex(("campaign == NULL"));

	tiles.resize(campaign->medals.size());
	for(size_t i = 0; i < tiles.size(); ++i) {
		tiles[i] = new Image;
		tiles[i]->set(ResourceManager->load_surface(campaign->medals[i].tile));
		add(0, 0, tiles[i], background);
	}
	update();
}

void Medals::set(const Campaign * c) { 
	if (campaign == c)
		return;
	
	campaign = c;
	update();
}

void Medals::get_medals(const std::string &id, int &now, int &total) const {
	now = 0; total = 0;
	if (id == "elimination") {
		for(size_t i = 0; i < campaign->maps.size(); ++i) {
			const Campaign::Map &map = campaign->maps[i];
			if (map.no_medals || map.score <= 0)
				continue;

			++total;
			std::string mname = "campaign." + campaign->name + ".maps." + map.id + ".maximum-score";
			if (!Config->has(mname))
				continue;

			int bs;
			Config->get(mname, bs, 0);
			if (bs <= map.score)
				++now;
		}		
	} else if (id == "speedrun") {
		for(size_t i = 0; i < campaign->maps.size(); ++i) {
			const Campaign::Map &map = campaign->maps[i];
			if (map.no_medals || map.time <= 0)
				continue;

			++total;
			std::string mname = "campaign." + campaign->name + ".maps." + map.id + ".best-time";
			if (!Config->has(mname))
				continue;
			
			float bt;
			Config->get(mname, bt, 3600);
			if (bt <= map.time)
				++now;
		}
	} else if (id == "secrets") {
		for(size_t i = 0; i < campaign->maps.size(); ++i) {
			const Campaign::Map &map = campaign->maps[i];
			if (!map.secret)
				continue;
			++total;
			if (campaign->visible(map)) 
				++now;
		}		
	}
}

void Medals::update() {
	if (tiles.empty())
		return;

	assert(campaign != NULL);
	
	int idx = active;
	if (idx >= (int)tiles.size()) {
		idx = tiles.size() - 1;
	}
	const Campaign::Medal &medal = campaign->medals[idx];
	title->set("campaign/medals", medal.id);

	int iw, ih;
	for(int i = 0; i < (int)tiles.size(); ++i) {
		Image * image = tiles[i];
		image->get_size(iw, ih);
		int d = (i - idx + tiles.size()) % tiles.size();
		if (d > (int)tiles.size() / 2)
			d -= tiles.size();
		//LOG_DEBUG(("%d: %d", d, _w / 2 + d * _w / 2));
		image->set_base(_w / 2 + d * _w / 2 - iw / 2, _h / 2 - ih / 2);
	}

	int bw, bh;
	title->get_size(bw, bh);
	title->set_base((_w - bw) / 2, _h / 2 - ih / 2 - bh);

	int now, total;
	get_medals(medal.id, now, total);
	numbers->set(mrt::format_string("%d/%d", now, total));

	numbers->get_size(bw, bh);
	numbers->set_base((_w - bw) / 2, _h / 2 + ih / 2 - bh);
	
	if (hint != NULL) {
		remove(hint);
	}

	hint = new Tooltip("campaign/medals", medal.id + "-hint", true, 320);
	hint->get_size(bw, bh);
	add((_w - bw) / 2, _h / 2 + ih / 2 + 32, hint);
	
	invalidate(true);
}

bool Medals::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
	case SDLK_RETURN: 
		hide();
		return true;

	case SDLK_LEFT: 
		active -= 2;
	case SDLK_RIGHT: 
		++active;
		if (active < 0)
			active += tiles.size();
		if (active >= (int)tiles.size())
			active -= tiles.size();
		update();
		return true;
	default: 
		return true;
	}
}

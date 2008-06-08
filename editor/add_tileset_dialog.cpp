#include "add_tileset_dialog.h"
#include "tmx/tileset_list.h"
#include "mrt/fs_node.h"

AddTilesetDialog::AddTilesetDialog(const int w, const int h) : 
ScrollList("menu/background_box_dark.png", "small", w, h) {}

const bool AddTilesetDialog::init(const std::string &fname, TilesetList &tilesets, const std::vector<std::string> &all_tilesets) {
	_tileset.clear();
	
	std::string dir = mrt::FSNode::get_dir(fname);
	LOG_DEBUG(("map file: %s base dir: %s", fname.c_str(), dir.c_str()));
	
	clear();
	_tilesets.clear();
	
	bool found = false;
	for(size_t i = 0; i < all_tilesets.size(); ++i) {
		const std::string tileset = mrt::FSNode::relative_path(dir, all_tilesets[i]);
		//LOG_DEBUG(("tileset: %s", tileset.c_str()));
		if (!tilesets.exists(tileset)) {
			_tilesets.push_back(tileset);
			append(mrt::FSNode::get_filename(tileset, false));
			found = true;
		}
	}

	return found;
}

bool AddTilesetDialog::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
		{
			//adding tileset.
			_tileset = _tilesets[get()];
			LOG_DEBUG(("adding tileset #%d (%s)", get(), _tileset.c_str()));
		}
	case SDLK_ESCAPE: 
		hide();
		return true;
		
	default: 
		return ScrollList::onKey(sym);
	}
}

const std::string AddTilesetDialog::getTileset() { 
	std::string r = _tileset; 
	_tileset.clear(); 
	return r; 
}

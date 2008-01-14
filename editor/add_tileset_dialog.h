#ifndef ADD_TILESET_DIALOG_H__
#define ADD_TILESET_DIALOG_H__

#include "menu/scroll_list.h"
#include <vector>
#include <string>

class TilesetList;

class AddTilesetDialog : public ScrollList {
public: 
	AddTilesetDialog(const int w, const int h);

	const bool init(const std::string &map_fname, TilesetList &tilesets, const std::vector<std::string> &all_tilesets);
	virtual bool onKey(const SDL_keysym sym);

	const std::string getTileset();
	
private:
	std::string _tileset;
	std::vector<std::string> _tilesets;
};

#endif


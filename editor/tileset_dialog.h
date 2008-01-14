#ifndef BTANKS_EDITOR_TILESET_DIALOG_H__
#define BTANKS_EDITOR_TILESET_DIALOG_H__

#include "menu/container.h"
#include <vector>
#include <string>
#include <map>

#include "sdlx/surface.h"
#include "tmx/tileset_list.h"
#include "base_brush.h"

class Box;
class ScrollList;
class AddTilesetDialog;

class TilesetDialog : public Container {
public:
	TilesetDialog(const int w, const int h);
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const { w = _w; h = _h; }
	
	BaseBrush *getBrush();
	const bool tileset_added() { bool r = _tileset_added; _tileset_added = false; return r; }

private: 
	void set(const int tileset);
	void initMap();
	virtual void tick(const float dt);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	int _w, _h;
	
	std::vector<std::string> _all_tilesets;
	TilesetList _tilesets;
	const sdlx::Surface *_current_tileset;
	int _current_tileset_idx, _current_tileset_gid;
	
	Box *_tileset_bg;
	ScrollList *_sl_tilesets;
	v2<float> _vel, _pos;

	//brush stuff
	bool _selecting, _selected;
	v2<int> _brush_1, _brush_2;
	sdlx::Surface _brush;
	Brush _editor_brush;

	//map stuff
	std::string _fname;
	v2<int> _tile_size;
	
	AddTilesetDialog *_add_tileset;
	bool _tileset_added;
};

#endif


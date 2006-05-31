#include "tilemanager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"

IMPLEMENT_SINGLETON(TileManager, ITileManager)

ITileManager::ITileManager() {
	LOG_DEBUG(("ITileManager ctor"));
}


void ITileManager::set(long id, sdlx::Surface *s) {
	sdlx::Surface *& tile = _tiles[id];	
	if (tile != NULL) 
		delete tile;
	tile = s;
}

void ITileManager::clear() {
	for(TileMap::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		delete i->second;
	}
	_tiles.clear();
}

ITileManager::~ITileManager() {
	clear();
}

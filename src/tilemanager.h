#ifndef __BT_TILEMANAGER_H__
#define __BT_TILEMANAGER_H__

#include "mrt/singleton.h"
#include <map>
#error do not use this code now

namespace sdlx {
	class Surface;
}

class ITileManager {
public:
	ITileManager();
	~ITileManager();
	DECLARE_SINGLETON(ITileManager);
	
	void set(long id, sdlx::Surface *s);
	void clear();
private:
	typedef std::map<const long, sdlx::Surface *> TileMap;
	TileMap _tiles;
};

SINGLETON(TileManager, ITileManager);

#endif


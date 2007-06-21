#include "sdlx/c_map.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

int main() {
	sdlx::CollisionMap map1, map2;
	map1.create(7, 7, 1);
	LOG_DEBUG(("map dump: %s", map1.dump().c_str()));
	map2.create(7, 7, 1);
	for(int y = -8; y < 8; ++y) 
		for(int x = -8; x < 8; ++x) {
			sdlx::Rect pixel1(0, 0, 1, 1), pixel2(3, 3, 1, 1);
			bool b = map1.collides(pixel1, &map2, pixel2, x, y, false);
			LOG_DEBUG(("%d,%d: %s", x, y, b?"true":"false"));
		}
}

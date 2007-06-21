#include "sdlx/c_map.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

int main() {
	sdlx::CollisionMap map1, map2, map3;
	map1.create(16, 16, 1);

	//LOG_DEBUG(("map dump: %s", map1.dump().c_str()));
	map2.create(7, 1, 1);
	int x, y = 15;
	for(x = -8; x <= 16; ++x) {
		sdlx::Rect pixel1(0, 0, 16, 16), pixel2(0, 0, 7, 1);
		bool b = map1.collides(pixel1, &map2, pixel2, x, y, false);
		LOG_DEBUG(("%d,%d: %s", x, y, b?"true":"false"));
	}
	LOG_DEBUG(("vertical"));
	map3.create(1, 3, 1);
	x = 0;
	for(y = -5; y <= 16; ++y) {
		sdlx::Rect pixel1(0, 0, 16, 16), pixel2(0, 0, 1, 3);
		bool b = map1.collides(pixel1, &map3, pixel2, x, y, false);
		LOG_DEBUG(("%d,%d: %s", x, y, b?"true":"false"));
	}
}

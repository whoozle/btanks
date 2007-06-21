#include "sdlx/c_map.h"
#include "mrt/logger.h"
#include "sdlx/rect.h"

int main() {
	sdlx::CollisionMap map1, map2;
	map1.create(15, 15, 1);
	LOG_DEBUG(("map dump: %s", map1.dump().c_str()));
	map2.create(15, 15, 1);
	sdlx::Rect pixel(7, 7, 1, 1);
	for(int y = 0; y < 15; ++y) 
		for(int x = 0; x < 15; ++x) {
			bool b = map1.collides(pixel, &map2, pixel, x, y, false);
			LOG_DEBUG(("%d,%d: %s", x, y, b?"true":"false"));
		}
	LOG_DEBUG(("worst test"));

	for(int y = 0; y < 15; ++y) {
		sdlx::Rect r(14, 14, 1, 1);
		bool b = map1.collides(r, &map2, r, 0, y, false);
		LOG_DEBUG(("x=0, 0,%d: %s", y, b?"true":"false"));
	}
	for(int x = 0; x < 15; ++x) {
		sdlx::Rect r(14, 14, 1, 1);
		bool b = map1.collides(r, &map2, r, x, 0, false);
		LOG_DEBUG(("y=0, %d,0: %s", x, b?"true":"false"));
	}
}

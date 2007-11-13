#include "rush.h"
#include "tmx/map.h"
#include "object.h"
#include "mrt/random.h"

using namespace ai;

void Rush::calculateW(Way &way, Object *object) {
	way.clear();
	
	const v2<int> tile_size = Map->getPathTileSize();
	const v2<int> map_size = Map->getSize();
	const Matrix<int> & water = Map->getAreaMatrix("water");
	v2<int> pos;
	object->getCenterPosition(pos);
	int im = water.get(pos.y / tile_size.y, pos.x / tile_size.x);
	if (im != 1) {
		LOG_WARN(("object %s:%d is now on non-hint area (value: %d)", object->animation.c_str(), object->getID(), im));
		object->emit("death", NULL); //bam! 
		return;
	}
	
	int dirs = object->getDirectionsNumber();
	int dir = mrt::random(dirs);
	v2<float> d; 
	d.fromDirection(dir, dirs);
	d.normalize((tile_size.x + tile_size.y) / 2);
	int len = 0;
	while(water.get(pos.y / tile_size.y, pos.x / tile_size.x) == 1) {
		++len;
		pos += d.convert<int>();
	}
	//LOG_DEBUG(("d: %g %g, len: %d", d.x, d.y, len));
	len -= (int)(object->size.x + object->size.y) / (tile_size.x + tile_size.y) / 2 + 1;
	if (len > 0) {
		len = 1 + len / 2 + (len % 2) + mrt::random(len / 2);
		object->getCenterPosition(pos);
		pos += (d * len).convert<int>();
		if (pos.x < object->size.x / 2) 
			pos.x = (int)object->size.x / 2;
		if (pos.y < object->size.y / 2) 
			pos.y = (int)object->size.y / 2;
		if (pos.x + object->size.x / 2 > map_size.x) 
			pos.x = map_size.x - (int)object->size.x / 2;
		if (pos.y + object->size.y / 2 > map_size.y) 
			pos.y = map_size.y - (int)object->size.y / 2;
		way.push_back(pos);
		return;
	}
}

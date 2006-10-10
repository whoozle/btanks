#include "c_map.h"
#include "surface.h"
#include "mrt/logger.h"
#include "math/minmax.h"

using namespace sdlx;

CollisionMap::CollisionMap() : _w(0), _h(0) {}

const bool CollisionMap::collides(const CollisionMap *other, const int bx, const int by) const {
	int ax1 = 0 + _w * 8 - 1;
	int ay1 = 0 + _h - 1;
	
	/*b - bottom right co-ordinates*/
	int bx1 = bx + other->_w * 8 - 1;
	int by1 = by + other->_h - 1;
	
	int inter_x0, inter_x1, inter_y0, inter_y1, x, y;

	/*check if bounding boxes intersect*/
	if((bx1 < 0) || (ax1 < bx))
		return 0;
	if((by1 < 0) || (ay1 < by))
		return 0;


	inter_x0 = math::max(0, bx);
	inter_x1 = math::min(ax1, bx1);

	inter_y0 = math::max(0, by);
	inter_y1 = math::min(ay1, by1);
	/* printf("%d %d :: %d %d\n", inter_x0, inter_y0, inter_x1, inter_y1); */

	unsigned char * ptr1 = (unsigned char *) _data.getPtr();
	unsigned char * ptr2 = (unsigned char *) other->_data.getPtr();

	int size1 = _data.getSize();
	int size2 = other->_data.getSize();

	for(y = inter_y0 ; y <= inter_y1 ; ++y) {
		for(x = inter_x0 ; x <= inter_x1 ; ++x)	{
			int pos1 = x / 8 + y * _w;
			int pos2 = (x - bx) / 8 + (y - bx) * _w;
			
			assert(pos1 >= 0 && pos1 < size1);
			assert(pos2 >= 0 && pos2 < size2);
			
			unsigned bit1 = 1<<(7 - (x & 7));
			unsigned bit2 = 1<<(7 - ((x - bx) & 7));
			
			if ( (ptr1[pos1]&bit1) != 0 && (ptr2[pos2]&bit2) != 0)
				return true;
		}
	}
	return false;
}


static const bool test_pixel(const sdlx::Surface * surface, const unsigned x, const unsigned y) {
	register Uint32 pixelcolor = surface->getPixel(x, y);
	
	if ((surface->getFlags() & SDL_SRCALPHA) == SDL_SRCALPHA) {
		Uint8 r, g, b, a;
		SDL_GetRGBA(pixelcolor, surface->getPixelFormat(), &r, &g, &b, &a); 
		return a == 255;
	}

	return (pixelcolor !=  surface->getPixelFormat()->colorkey);
}

void CollisionMap::init(const sdlx::Surface * surface) {
	assert(surface->getWidth() != 0 && surface->getHeight() != 0);
	_w = (surface->getWidth() - 1) / 8 + 1;
	_h = surface->getHeight();
	_data.setSize(_w * _h);
	LOG_DEBUG(("got surface %d %d -> %d %d, allocated: %u bytes", surface->getWidth(), surface->getHeight(), _w, _h, (unsigned)_data.getSize()));
	_data.fill(0);
	
	surface->lock();
	unsigned char * data = (unsigned char *)_data.getPtr();
	for(int y = 0; y < surface->getHeight(); ++y) {
		for(int x = 0; x < surface->getWidth(); ++x) {
			unsigned int b = 7-(x&7);
			unsigned int pos = y * _w + x / 8;
			assert(pos < _data.getSize());
			data[pos] |= (test_pixel(surface, x, y)?1:0) << b;
		}
	}
	surface->unlock();	
	//LOG_DEBUG(("built collision map (size: %u): %s", (unsigned)_data.getSize(), _data.dump().c_str()));
}




/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "c_map.h"
#include "surface.h"
#include "mrt/logger.h"
#include "math/minmax.h"
#include "mrt/file.h"

using namespace sdlx;

CollisionMap::CollisionMap() : _w(0), _h(0) {}

const bool CollisionMap::collides(const sdlx::Rect &src, const CollisionMap *other, const sdlx::Rect &other_src, const int bx, const int by) const {
	int aw = (src.w > 0)?src.w:(_w * 8); 
	int ah = (src.h > 0)?src.h:_h; 

	int bw = (other_src.w > 0)?other_src.w:(other->_w * 8); 
	int bh = (other_src.h > 0)?other_src.h:other->_h; 
	
	int ax1 = 0 + aw - 1;
	int ay1 = 0 + ah - 1;
	
	/*b - bottom right co-ordinates*/
	int bx1 = bx + bw - 1;
	int by1 = by + bh - 1;
	
	int inter_x0, inter_x1, inter_y0, inter_y1, x, y;

	/*check if bounding boxes intersect*/
	if((bx1 < 0) || (ax1 < bx) || (by1 < 0) || (ay1 < by))
		return false;

	inter_x0 = math::max(0, bx);
	inter_x1 = math::min(ax1, bx1);

	inter_y0 = math::max(0, by);
	inter_y1 = math::min(ay1, by1);
	/* printf("%d %d :: %d %d\n", inter_x0, inter_y0, inter_x1, inter_y1); */

	unsigned char * ptr1 = (unsigned char *) _data.getPtr();
	unsigned char * ptr2 = (unsigned char *) other->_data.getPtr();

	int size1 = _data.getSize();
	int size2 = other->_data.getSize();
	
	//int steps = 0;

#define INTERLACE_STEP 8

#if INTERLACE_STEP == 8
	int steps_pos[] = {0, 4, 2, 6, 3, 7, 1, 5};
#elif INTERLACE_STEP == 4
	int steps_pos[] = {0, 2, 1, 3};
#elif INTERLACE_STEP == 2
	int steps_pos[] = {0, 1};
#endif

	for(int sy = 0; sy < INTERLACE_STEP; ++sy) 
	for(int sx = 0; sx < INTERLACE_STEP; ++sx) 
	for(y = inter_y0 + steps_pos[sy]; y <= inter_y1 ; y += INTERLACE_STEP) {
		for(x = inter_x0 + steps_pos[sx]; x <= inter_x1 ; x += INTERLACE_STEP)	{
			int pos1 = (src.x + x) / 8 + (src.y + y) * _w;
			int pos2 = (other_src.x + x - bx) / 8 + (other_src.y + y - by) * other->_w;
			
			/*
			assert(pos1 >= 0 && pos1 < size1);
			assert(pos2 >= 0 && pos2 < size2);
			*/ //collision detection code works in 99% cases. 
			//this asserts above can be triggered by malicious objects (invalid rectangle returned by getRenderRect)
			//so skip it. :)
			if (pos1 < 0 || pos1 >= size1 || pos2 < 0 || pos2 >= size2)
				continue;
			
			unsigned bit1 = 1<<(7 - (x & 7));
			unsigned bit2 = 1<<(7 - ((x - bx) & 7));
			
			if ( (ptr1[pos1]&bit1) != 0 && (ptr2[pos2]&bit2) != 0) {
				//LOG_DEBUG(("collision detected: %d steps", steps));
				return true;
			}
			//++steps;
		}
	}
	//LOG_DEBUG(("no collision : %d steps", steps));
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
	//LOG_DEBUG(("got surface %d %d -> %d %d, allocated: %u bytes", surface->getWidth(), surface->getHeight(), _w, _h, (unsigned)_data.getSize()));
	_data.fill(0);
	
	surface->lock();
	unsigned char * data = (unsigned char *)_data.getPtr();
	for(int y = 0; y < surface->getHeight(); ++y) {
		for(int x = 0; x < surface->getWidth(); ++x) {
			unsigned int b = 7-(x&7);
			unsigned int pos = y * _w + x / 8;
			assert(pos < _data.getSize());
	
			if (test_pixel(surface, x, y))
				data[pos] |= 1 << b;
		}
	}
	surface->unlock();	
	//LOG_DEBUG(("built collision map (size: %u): %s", (unsigned)_data.getSize(), _data.dump().c_str()));
}

void CollisionMap::save(const std::string &fname) const {
	mrt::File f;
	f.open(fname + ".raw", "wb");
	f.writeAll(_data);
	f.close();
	
	sdlx::Surface s;
	s.createRGB(_w * 8, _h, 8, SDL_SWSURFACE);
	s.lock();
	unsigned char *ptr = (unsigned char *)_data.getPtr();
	unsigned int idx = 0;
	for(unsigned y = 0; y < _h; ++y) {
		for(unsigned x = 0; x < _w; ++x) {
			assert(idx < _data.getSize());
			unsigned int byte = ptr[idx++];
			for(int b = 0; b < 8; ++b) {
				bool c = (byte & (0x80 >> b)) != 0;
				if (c)
					s.putPixel(x*8 + b, y, 0xffffffff);
			}
		}
	}
	s.unlock();
	s.saveBMP(fname + ".bmp");
}


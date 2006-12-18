
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

CollisionMap::CollisionMap() : _empty(true), _w(0), _h(0), _data() {}

const bool CollisionMap::collides(const sdlx::Rect &src, const CollisionMap *other, const sdlx::Rect &other_src, const int bx, const int by, const bool hidden_by_other) const {
	if (_empty || other->_empty)
		return false;
	
	int aw = (src.w > 0)?src.w:(_w * 8); 
	int ah = (src.h > 0)?src.h:_h; 

	int bw = (other_src.w > 0)?other_src.w:(other->_w * 8); 
	int bh = (other_src.h > 0)?other_src.h:other->_h; 
	
	int ax1 = 0 + aw - 1;
	int ay1 = 0 + ah - 1;
	
	/*b - bottom right co-ordinates*/
	int bx1 = bx + bw - 1;
	int by1 = by + bh - 1;
	
	int inter_x0, inter_x1, inter_y0, inter_y1;

	/*check if bounding boxes intersect*/
	if((bx1 < 0) || (ax1 < bx) || (by1 < 0) || (ay1 < by))
		return false;

	inter_x0 = math::max(0, bx);
	inter_x1 = math::min(ax1, bx1);

	inter_y0 = math::max(0, by);
	inter_y1 = math::min(ay1, by1);
	/* printf("%d %d :: %d %d\n", inter_x0, inter_y0, inter_x1, inter_y1); */

	register unsigned char * ptr1 = (unsigned char *) _data.getPtr();
	register unsigned char * ptr2 = (unsigned char *) other->_data.getPtr();

	int size1 = _data.getSize();
	int size2 = other->_data.getSize();
	

//you can play with it, but 8 seems optimal for me.
#define INTERLACE_STEP 8 

#if INTERLACE_STEP == 16
	int steps_pos[] = {0, 8, 4, 12,  2, 10, 6, 14,  1, 9, 5, 13,  3, 11, 7, 15};	
#elif INTERLACE_STEP == 8
	int steps_pos[] = {0, 4, 2, 6, 3, 7, 1, 5};
#elif INTERLACE_STEP == 4
	int steps_pos[] = {0, 2, 1, 3};
#elif INTERLACE_STEP == 2
	int steps_pos[] = {0, 1};
#endif

/*	int steps = 0;
	int steps_total = (inter_y1 - inter_y0 + 1) * (inter_x1 - inter_x0 + 1);
*/
	for(int sy = 0; sy < INTERLACE_STEP; ++sy) 
	for(int sx = 0; sx < INTERLACE_STEP; ++sx) 
	for(int y = inter_y0 + steps_pos[sy]; y <= inter_y1 ; y += INTERLACE_STEP) {
		int ybase1 = (src.y + y) * _w;
		int ybase2 = (other_src.y + y - by) * other->_w;
		for(int x = inter_x0 + steps_pos[sx]; x <= inter_x1 ; x += INTERLACE_STEP)	{
			int pos1 = (src.x + x) / 8 + ybase1;
			int pos2 = (other_src.x + x - bx) / 8 + ybase2;
			
			/*
			assert(pos1 >= 0 && pos1 < size1);
			assert(pos2 >= 0 && pos2 < size2);
			*/ //collision detection code works in 99% cases. 
			//this asserts above can be triggered by malicious objects (invalid rectangle returned by getRenderRect)
			//so skip it. :)
			if (pos1 < 0 || pos2 < 0)
				continue;
			
			if (pos1 >= size1 || pos2 >= size2)
				break;
			
			unsigned bit1 = 1<<(7 - (x & 7));
			unsigned bit2 = 1<<(7 - ((x - bx) & 7));
			
			if ( (ptr1[pos1]&bit1) != 0 && (ptr2[pos2]&bit2) != 0) {
				//LOG_DEBUG(("collision detected: %d of %d : %g%", steps, steps_total, 100.0 * steps / steps_total));
				return true;
			}
			//++steps;
		}
	}
	//LOG_DEBUG(("no collision : %d steps", steps));
	return false;
}


static const bool test_pixel(const sdlx::Surface * surface, const unsigned x, const unsigned y, const CollisionMap::Type type) {
	register Uint32 pixelcolor = surface->getPixel(x, y);
	
	switch(type) {
	case CollisionMap::OnlyOpaque:
		if ((surface->getFlags() & SDL_SRCALPHA) == SDL_SRCALPHA) {
			Uint8 r, g, b, a;
			SDL_GetRGBA(pixelcolor, surface->getPixelFormat(), &r, &g, &b, &a); 
			return a == 255;
		}
		return (pixelcolor !=  surface->getPixelFormat()->colorkey);

	case CollisionMap::AnyVisible: 
		if ((surface->getFlags() & SDL_SRCALPHA) == SDL_SRCALPHA) {
			Uint8 r, g, b, a;
			SDL_GetRGBA(pixelcolor, surface->getPixelFormat(), &r, &g, &b, &a); 
			return a >= 250;
		}
		return (pixelcolor !=  surface->getPixelFormat()->colorkey);
	}
	
	return false;
}

void CollisionMap::init(const sdlx::Surface * surface, const Type type) {
	_empty = true;
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
	
			if (test_pixel(surface, x, y, type)) {
				data[pos] |= 1 << b;
				_empty = false;
			}
		}
	}
	surface->unlock();	
	//LOG_DEBUG(("built collision map (size: %u): %s", (unsigned)_data.getSize(), _data.dump().c_str()));
	//if (_empty)
	//	LOG_DEBUG(("this collision map is empty"));
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


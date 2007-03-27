/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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
#include "mrt/file.h"

#include "math/binary.h"
#include "math/matrix.h"

#if defined(__GNUC__)
#define restrict __restrict__
#elif !defined(restrict)
#	define restrict
#endif

using namespace sdlx;

CollisionMap::CollisionMap() : _empty(true), _full(false), _w(0), _h(0), _data() {}

/*
	[-----+++] (size - 2) * [++++++++] [+++++++-]
	before 5                            after 7
*/

static inline const bool bitline_collide(const unsigned char *ptr1, const unsigned char *ptr2, const int first_bits, const int last_bits, const int shift_1, const int size, const int line_size) {
	assert(shift_1 > 0 && shift_1 < 8 && last_bits >= 0 && last_bits < 8);	
	register unsigned int b1 = (ptr1[0]<<8) | ((size > 0)?ptr1[1] : 0);
	register unsigned int b2 = (ptr2[0]<<8) | ((size > 0)?ptr2[1] : 0);
	//LOG_DEBUG(("first_bits: %d, last_bits: %d, shift: %d, size: %d", first_bits, last_bits, shift_1, size));
	unsigned int mask1 = (1 << (16 - first_bits)) - 1;
	if (mask1 & (b1 << shift_1 ) & b2)
		return true;

	
	register int i;
	for(i = 1; i <= size - 2; ++i) {
		b1 = (ptr1[i]<<8) | ptr1[i + 1];
		b2 = (ptr2[i]<<8) | ptr2[i + 1];
		if ( (b1 << shift_1 ) & b2)
			return true;
	}

	if (last_bits == 0 || size < 2)
		return false;
	
	unsigned int mask2 = ~((1 << (8 - last_bits)) - 1);
	if ((ptr1[size - 1] << shift_1) & mask2 & ptr2[size - 1])
		return true;
	
	return false;
}

static inline const bool bitline_collide(
 const unsigned char *base1, const int size1, const int pos1, 
 const unsigned char *base2, const int size2, const int pos2, 
 int line_size) {
	if (size1 <= 0 || size2 <= 0) 
		return false;
	
	const int pos1_aligned_start = pos1 / 8;
	const int pos1_bits_before   = math::min(pos1 % 8, line_size);
	const int pos1_aligned_size  = (line_size - pos1_bits_before) / 8;
	int pos1_bits_after    = line_size - 8 * pos1_aligned_size  - pos1_bits_before;
	assert(pos1_bits_after >= 0 && pos1_bits_after < 8 && pos1_aligned_size >= 0);

	const int pos2_aligned_start = pos2 / 8;
	const int pos2_bits_before   = math::min(pos2 % 8, line_size);
	const int pos2_aligned_size  = (line_size - pos2_bits_before) / 8;
	int pos2_bits_after    = line_size - 8 * pos2_aligned_size - pos2_bits_before;
	assert(pos2_bits_after >= 0 && pos2_bits_after < 8 && pos2_aligned_size >= 0);
	
	//LOG_DEBUG(("1: start: %d, bits before: %d, aligned size: %d, bits after: %d", pos1_aligned_start, pos1_bits_before, pos1_aligned_size, pos1_bits_after));
	//LOG_DEBUG(("2: start: %d, bits before: %d, aligned size: %d, bits after: %d", pos2_aligned_start, pos2_bits_before, pos2_aligned_size, pos2_bits_after));

	int size = math::min(pos1_aligned_size, pos2_aligned_size); //first byte
	assert(size >= 0 /* && size <= size1 && size <= size2 */);
	if (size > math::min(size1, size2)) 
		size = math::min(size1, size2);
	
	int shift = pos2_bits_before - pos1_bits_before;
	//int before = math::min(pos2_bits_before, pos1_bits_before);
	//int after = math::max(pos2_bits_after, pos1_bits_after);
	
	if (shift < 0) {
		if (bitline_collide(base1 + pos1_aligned_start, base2 + pos2_aligned_start, pos2_bits_before, pos2_bits_after, -shift, size, line_size))
			return true; 
	} else if (shift > 0) {
		if (bitline_collide(base2 + pos2_aligned_start, base1 + pos1_aligned_start, pos1_bits_before, pos1_bits_after, shift, size, line_size))
			return true;
	} else {
		//LOG_DEBUG(("quick compare, aligned data. %d afterbits: %d %d", size, pos1_bits_after, pos2_bits_after));
		int ln = size / 4;
		unsigned long *p1 = (unsigned long *)(base1 + pos1_aligned_start);
		unsigned long *p2 = (unsigned long *)(base2 + pos2_aligned_start);
		while(ln--) {
			if ((*p1++) & (*p2++)) 
				return true;
		}
		int cn = size % 4;
		unsigned char *pc1 = (unsigned char *) p1;
		unsigned char *pc2 = (unsigned char *) p2;
		while(cn--) {		
			if ((*pc1++) & (*pc2++)) 
				return true;
		}
	}
	
	return false;
}

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

	if (_full && other->_full)
		return true;


	inter_x0 = math::max(0, bx);
	inter_x1 = math::min(ax1, bx1);

	inter_y0 = math::max(0, by);
	inter_y1 = math::min(ay1, by1);

	//LOG_DEBUG(("%p->collide(%p, src:(%d, %d, %d, %d), osrc:(%d, %d, %d, %d), [%d, %d, %d, %d])", this, other, src.x, src.y, aw, ah, other_src.x, other_src.y, bw, bh, inter_x0, inter_y0, inter_y0, inter_y1));

	unsigned char * restrict ptr1 = (unsigned char *) _data.getPtr();
	unsigned char * restrict ptr2 = (unsigned char *) other->_data.getPtr();

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
#elif INTERLACE_STEP == 1
	int steps_pos[] = {0};
#endif

/*	int steps = 0;
	int steps_total = (inter_y1 - inter_y0 + 1) * (inter_x1 - inter_x0 + 1);
*/
#define NEW_COLLIDES
#ifndef NEW_COLLIDES
	for(int sy = 0; sy < INTERLACE_STEP; ++sy) 
	for(int sx = 0; sx < INTERLACE_STEP; ++sx) 
	for(int y = inter_y0 + steps_pos[sy]; y <= inter_y1 ; y += INTERLACE_STEP) {
		const int ybase1 = (src.y + y) * _w;
		const int ybase2 = (other_src.y + y - by) * other->_w;
		for(int x = inter_x0 + steps_pos[sx]; x <= inter_x1 ; x += INTERLACE_STEP)	{
			const int pos1 = (src.x + x) / 8 + ybase1;
			const int pos2 = (other_src.x + x - bx) / 8 + ybase2;
			
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
			
			const unsigned char bit1 = 1<<(7 - ((x + src.x) & 7));
			const unsigned char bit2 = 1<<(7 - ((other_src.x + x - bx) & 7));
			
			if ( (ptr1[pos1]&bit1) != 0 && (ptr2[pos2]&bit2) != 0) {
				//LOG_DEBUG(("collision detected: %d of %d : %g%", steps, steps_total, 100.0 * steps / steps_total));
				return true;
			}
			//++steps;
		}
	}
#else
	for(int sy = 0; sy < INTERLACE_STEP; ++sy) 
		for(int y = inter_y0 + steps_pos[sy]; y <= inter_y1 ; y += INTERLACE_STEP) {
			const int ybase1 = (src.y + y) * _w;
			const int ybase2 = (other_src.y + y - by) * other->_w;
			if (::bitline_collide(
				ptr1 + ybase1, size1 - ybase1, src.x + inter_x0, 
				ptr2 + ybase2, size2 - ybase2, other_src.x + inter_x0 - bx , 
				inter_x1 - inter_x0 + 1
				))
				return true;
		}
#endif

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

void CollisionMap::create(const unsigned int w, const unsigned int h, const bool bit) {
	_empty = !bit;
	_full =   bit;
	
	_w = (w - 1) / 8 + 1; 
	_h = h;
	
	
	_data.setSize(_w * _h);
	_data.fill(bit?~0:0);
}


void CollisionMap::init(const sdlx::Surface * surface, const Type type) {
	_empty = true;
	_full = true;
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
			} else _full = false;
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


void CollisionMap::project(Matrix<bool> &result, const unsigned w, const unsigned h) const {
	unsigned xs = _w / w, ys = _h / h;
	if (xs * w != _w || ys * h != _h) 
		throw_ex(("cannot project collision map %dx%d (square size: %dx%d)", _w, _h, xs, ys));
	result.setSize(h, w, false);
	unsigned char *ptr = (unsigned char *)_data.getPtr();
	unsigned int size = _data.getSize();
	for(unsigned int y = 0; y < _h; ++y) 
		for(unsigned int x = 0; x < _w; ++x) {
			assert(x + _w * y < size);
			if (ptr[x + _w * y])
				result.set(y / ys, x / xs, true);
		}
}

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
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


//DO NOT USE THIS FUNCTION FOR SIGNED TYPES! :(
template <typename T> 
static inline const bool type_collide(T* &ptr1, const int shift1, T* &ptr2, const int shift2, const T mask = ~(T)0) {
	const T a = (shift1 != 0)?((*ptr1++ << shift1) | (*ptr1 >> (sizeof(T) * 8 - shift1))):*ptr1++;
	const T b = (shift2 != 0)?((*ptr2++ << shift2) | (*ptr2 >> (sizeof(T) * 8 - shift2))):*ptr2++;
	return (mask & a & b) != 0;
}

static inline const bool bitline_collide(
 const unsigned char *base1, const int size1, const int x1, 
 const unsigned char *base2, const int size2, const int x2, 
 int line_size) {
	if (size1 <= 0 || size2 <= 0 || line_size <= 0) 
		return false;
	
	const int shift1 = x1 % 8, pos1 = x1 / 8;
	const int shift2 = x2 % 8, pos2 = x2 / 8;
	
	assert((line_size - 1) / 8 + 1 <= size1);
	assert((line_size - 1) / 8 + 1 <= size2);

	unsigned int *iptr1 = (unsigned int *) (base1 + pos1);
	unsigned int *iptr2 = (unsigned int *) (base2 + pos2);

	for(; line_size >= 8 * (int)sizeof(int); line_size -= 8 * (int)sizeof(int)) {
		if (type_collide(iptr1, shift1, iptr2, shift2))
			return true;
	}

	Uint8 *ptr1 = (Uint8 *) iptr1;
	Uint8 *ptr2 = (Uint8 *) iptr2;

	for(; line_size >= 8 * (int)sizeof(Uint8); line_size -= 8 * (int)sizeof(Uint8)) {
		if (type_collide(ptr1, shift1, ptr2, shift2))
			return true;
	}

	if (line_size == 0)
		return false; //no collision, line_size aligned by 8 bits.

	const Uint8 mask = ~((1 << (8 - line_size)) - 1);
	//LOG_DEBUG(("a: 0x%x, b: 0x%x, line_size: %d, mask: 0x%x", a, b, line_size, mask));
	return type_collide(ptr1, shift1, ptr2, shift2, mask);
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

	unsigned char * restrict ptr1 = (unsigned char *) _data.get_ptr();
	unsigned char * restrict ptr2 = (unsigned char *) other->_data.get_ptr();

	int size1 = _data.get_size();
	int size2 = other->_data.get_size();
	

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
			//this asserts above can be triggered by malicious objects (invalid rectangle returned by get_render_rect)
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


static inline const bool test_pixel(const sdlx::Surface * surface, const unsigned x, const unsigned y, const CollisionMap::Type type) {
	Uint32 pixelcolor = surface->get_pixel(x, y);
	
	switch(type) {
	case CollisionMap::OnlyOpaque:
		if ((surface->getFlags() & SDL_SRCALPHA) == SDL_SRCALPHA) {
			Uint8 r, g, b, a;
			surface->get_rgba(pixelcolor, r, g, b, a);
			return a == 255;
		}
		return (pixelcolor !=  surface->get_pixel_format()->colorkey);

	case CollisionMap::AnyVisible: 
		if ((surface->getFlags() & SDL_SRCALPHA) == SDL_SRCALPHA) {
			Uint8 r, g, b, a;
			surface->get_rgba(pixelcolor, r, g, b, a);
			return a >= 250;
		}
		return (pixelcolor !=  surface->get_pixel_format()->colorkey);
	}
	
	return false;
}

void CollisionMap::create(const unsigned int w, const unsigned int h, const bool bit) {
	_empty = !bit;
	_full =   bit;
	
	_w = (w - 1) / 8 + 1; 
	_h = h;
	
	
	_data.set_size(_w * _h);
	_data.fill(bit?~0:0);
}

bool CollisionMap::load(unsigned int w, unsigned int h, const mrt::Chunk &data) {
	unsigned size = ((w - 1) / 8 + 1) * h;
	if (size != data.get_size()) {
		LOG_WARN(("collision data size mismatch. %ux%u = %u, got %u", w, h, size, (unsigned)data.get_size()));
		return false;
	}
	
	_data = data;
	return true;
}

void CollisionMap::init(const sdlx::Surface * surface, const Type type) {
	_empty = true;
	_full = true;
	assert(surface->get_width() != 0 && surface->get_height() != 0);
	_w = (surface->get_width() - 1) / 8 + 1;
	_h = surface->get_height();
	_data.set_size(_w * _h);
	//LOG_DEBUG(("got surface %d %d -> %d %d, allocated: %u bytes", surface->get_width(), surface->get_height(), _w, _h, (unsigned)_data.get_size()));
	_data.fill(0);
	
	surface->lock();
	unsigned char * data = (unsigned char *)_data.get_ptr();
	for(int y = 0; y < surface->get_height(); ++y) {
		for(int x = 0; x < surface->get_width(); ++x) {
			unsigned int b = 7-(x&7);
			unsigned int pos = y * _w + x / 8;
			assert(pos < _data.get_size());
	
			if (test_pixel(surface, x, y, type)) {
				data[pos] |= (1 << b);
				_empty = false;
			} else _full = false;
		}
	}
	surface->unlock();	
	//LOG_DEBUG(("built collision map (size: %u): %s", (unsigned)_data.get_size(), _data.dump().c_str()));
	//if (_empty)
	//	LOG_DEBUG(("this collision map is empty"));
}

void CollisionMap::save(const std::string &fname) const {
	mrt::File f;
	f.open(fname, "wb");
	f.write_all(_data);
	f.close();
}


void CollisionMap::project(Matrix<bool> &result, const unsigned w, const unsigned h) const {
	unsigned xs = _w / w, ys = _h / h;
	if (xs * w != _w || ys * h != _h) 
		throw_ex(("cannot project collision map %dx%d (square size: %dx%d)", _w, _h, xs, ys));
	result.set_size(h, w, false);
	unsigned char *ptr = (unsigned char *)_data.get_ptr();
	unsigned int size = _data.get_size();
	for(unsigned int y = 0; y < _h; ++y) 
		for(unsigned int x = 0; x < _w; ++x) {
			assert(x + _w * y < size);
			if (ptr[x + _w * y])
				result.set(y / ys, x / xs, true);
		}
}

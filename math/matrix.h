#ifndef __BTANKS_MATRIX_H__
#define __BTANKS_MATRIX_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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


#include "mrt/chunk.h"
#include "mrt/exception.h"

template <class T> class Matrix {
public:
	Matrix() : _data(), _w(0), _h(0), _use_default(false), _default() {}
	Matrix(const int h, const int w, const T v = 0): _use_default(false) {
		setSize(h, w, v);
	}
	
	void useDefault(const T d) { _default = d; _use_default = true; }
	
	void fill(const T v) {
		T *ptr = (T*) _data.getPtr();
		for(int i = 0; i < _w * _h; ++i) {
			*ptr++ = v;
		}	
	}
	
	void setSize(const int h, const int w, const T v = 0) {
		_w = w;
		_h = h;
		_data.setSize(w * h * sizeof(T));

		fill(v);
	}
	
	inline const T get(const int y, const int x) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) {
			if (_use_default) 
				return _default;
			
			throw_ex(("get(%d, %d) is out of bounds", y, x));
		}
		register int idx = y * _w + x;
		register const T *ptr = (const T*) _data.getPtr();
		return *(ptr + idx);
	}
	
	inline void set(const int y, const int x, const T v) {
		if (x < 0 || x >= _w || y < 0 || y >= _h) {
			if (_use_default)
				return;
			throw_ex(("set(%d, %d) is out of bounds", y, x));
		}
		
		register int idx = y * _w + x;
		register T *ptr = (T*) _data.getPtr();
		*(ptr + idx) = v;
	}
	
	inline const int getWidth() const { return _w; }
	inline const int getHeight() const { return _h; }
	
	const std::string dump() const {
	//fixme: add template functions for conversion int/float other types to string
		std::string result;
		result += "      ";
		for(int x = 0; x < _w; ++x) 
			result += mrt::formatString("%-2d ", x);
		result += "\n";
		
		for(int y = 0; y < _h; ++y) {
			result += mrt::formatString("%-2d ", y);
			result += "[ ";
			for(int x = 0; x < _w; ++x) {
				result += mrt::formatString("%-2d ", (int)get(y, x));
			}
			result += " ]";
			result += mrt::formatString("%-2d\n", y);
		}

		result += "      ";
		for(int x = 0; x < _w; ++x) 
			result += mrt::formatString("%-2d ", x);
		result += "\n";

		return result;
	}
	
protected: 
	mrt::Chunk _data;	
	int _w, _h;
	bool _use_default;
	T _default;
};


#endif

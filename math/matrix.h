#ifndef __BTANKS_MATRIX_H__
#define __BTANKS_MATRIX_H__

#include "mrt/chunk.h"
#include "mrt/exception.h"

template <class T> class Matrix {
public:
	Matrix() : _w(0), _h(0), _use_default(false) {}
	Matrix(const int h, const int w, const T v = 0): _use_default(false) {
		setSize(h, w, v);
	}
	
	void useDefault(const T d) { _default = d; _use_default = true; }
	
	void setSize(const int h, const int w, const T v = 0) {
		_w = w;
		_h = h;
		_data.setSize(w * h * sizeof(T));

		T *ptr = (T*) _data.getPtr();
		for(int i = 0; i < w*h; ++i) {
			*ptr++ = v;
		}
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
		for(int y = 0; y < _h; ++y) {
			result += "[ ";
			for(int x = 0; x < _w; ++x) {
				result += mrt::formatString("%3d ", get(y, x));
			}
			result += " ]\n";
		}
		return result;
	}
	
protected: 
	mrt::Chunk _data;	
	int _w, _h;
	bool _use_default;
	T _default;
};


#endif

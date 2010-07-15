#ifndef __BTANKS_V2_H__
#define __BTANKS_V2_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "mrt/serializator.h"
#include "mrt/serializable.h"

#include <string>
#include <stdexcept>
#include "mrt/exception.h"
#include <typeinfo>

template <typename T> class v3;
template <typename T> class v2 : public mrt::Serializable {
public:
	T x, y;
	inline v2<T>() : x(0), y(0) {}
	inline v2<T>(const T x, const T y) : x(x), y(y) {} 

	inline void clear() { x = y = 0; }
	inline const bool is0() const {
		return x== 0 && y == 0;
	}
	
	inline const T normalize() {
		const T len = length();
		if (len == (T)0 || len ==(T)1) 
			return len;
		
		x /= len;
		y /= len;
		return len;
	}

	inline const T normalize(const T nlen) {
		const T len = length();
		if (len == (T)0 || len == nlen) 
			return len;
		
		x *= nlen / len;
		y *= nlen / len;
		return len;
	}
	
	inline const T length() const {
#ifdef _WINDOWS
		return (T)_hypot(x, y);
#else
		return (T)hypot(x, y);
#endif
	}

	inline const T quick_length() const {
		return (T)(x * x + y * y);
	}

	template <typename T2> 
		inline v2<T2> convert() const { return v2<T2>((T2)x, (T2)y); }
	
	inline const T distance(const v2<T>& other) const {
		v2<T>d(*this);
		d-= other;
		return d.length();
	}
	
	
	inline const T quick_distance(const v2<T>& other) const {
		const T dx = x - other.x;
		const T dy = y - other.y;
		return (dx * dx + dy * dy);
	}

	inline const T scalar(const v2<T> &other) const {
		return (x * other.x + y * other.y);
	}

	inline const bool same_sign(const v2<T> &other) const {
		return !(is0() || other.is0() || 
			(x != 0 && other.x * x < 0) || 
			(y != 0 && other.y * y < 0) 
		);
	}
	
	//operators 
	inline const bool operator<(const v2<T> &other) const {
		if (y != other.y) {
			return y < other.y;
		}
		return x < other.x;
	}


	inline const v2<T> operator-() const {
		return v2<T>(-x, -y);
	}
	
	inline const bool operator==(const v2<T> &other) const {
		return x == other.x && y == other.y;
	}

	inline const bool operator!=(const v2<T> &other) const {
		return x != other.x || y != other.y;
	}
	

	inline const v2<T>& operator+=(const v2<T>& other) {
		x += other.x; y += other.y;
		return *this;
	}

	inline const v2<T>& operator-=(const v2<T>& other) {
		x -= other.x; y -= other.y;
		return *this;
	}

	inline const v2<T>& operator*=(const v2<T>& other) {
		x *= other.x; y *= other.y;
		return *this;
	}

	inline const v2<T>& operator/=(const v2<T>& other) {
		x /= other.x; y /= other.y;
		return *this;
	}

	inline const v2<T>& operator%=(const v2<T>& other) {
		x %= other.x; y %= other.y;
		return *this;
	}
	
	inline const v2<T> operator*(const v2<T>& other) const {
		return v2<T>(x * other.x, y * other.y);
	}
	inline const v2<T> operator+(const v2<T>& other) const {
		return v2<T>(x + other.x, y + other.y);
	}
	inline const v2<T> operator-(const v2<T>& other) const {
		return v2<T>(x - other.x, y - other.y);
	}
	inline const v2<T> operator/(const v2<T>& other) const {
		return v2<T>(x / other.x, y / other.y);
	}
	inline const v2<T> operator%(const v2<T>& other) const {
		return v2<T>(x % other.x, y % other.y);
	}

	inline const v2<T> operator*(const T& other) const {
		return v2<T>(x * other, y * other);
	}
	inline const v2<T> operator+(const T& other) const {
		return v2<T>(x + other, y + other);
	}
	inline const v2<T> operator-(const T& other) const {
		return v2<T>(x - other, y - other);
	}
	inline const v2<T> operator/(const T& other) const {
		return v2<T>(x / other, y / other);
	}
	inline const v2<T> operator%(const T& other) const {
		return v2<T>(x % other, y % other);
	}

	inline const v2<T>& operator/=(const T& other) {
		x /= other;
		y /= other;
		return *this;
	}

	inline const v2<T>& operator%=(const T& other) {
		x %= other;
		y %= other;
		return *this;
	}

	inline const v2<T>& operator*=(const T& other) {
		x *= other;
		y *= other;
		return *this;
	}

	inline const v2<T>& operator+=(const T& other) {
		x += other;
		y += other;
		return *this;
	}

	inline const v2<T>& operator-=(const T& other) {
		x -= other;
		y -= other;
		return *this;
	}

private:
	static inline int c2d8(const T c) {
		if (c > T(0.9238795325112867385)) //cos(22.5)
			return 0;
		else if (c > T(0.3826834323650898373)) //cos(67.5)
			return 1;
		else if (c > T(-0.3826834323650898373))
			return 2;
		else if (c > T(-0.9238795325112867385))
			return 3;
		return 4;
	}

	static inline int c2d16(const T c) {
		if (c > T(0.9807852804032304306)) //11.25
			return 0;
		else if (c > T(0.8314696123025452357)) //cos(33.75)
			return 1;
		else if (c > T(0.5526644777167217804)) //cos 56.45
			return 2;
		else if (c > T(0.1916655539320546719)) //cos 78.95
			return 3;
		else if (c > T(-0.1916655539320546719)) 
			return 4;
		else if (c > T(-0.5526644777167217804))
			return 5;
		else if (c > T(-0.8314696123025452357))
			return 6;
		else if (c > T(-0.9807852804032304306)) //11.25
			return 7;
		return 8;
	}

public:	
	inline int get_direction8() const {
		if (is0())
			return 0;

		int xx = c2d8(x) + 1;
		return (y <= 0 || xx == 1)? xx: (10 - xx);
	}

	inline int get_direction16() const {
		if (is0())
			return 0;

		int xx = c2d16(x) + 1;
		return (y <= 0 || xx == 1)? xx: (18 - xx);
	}
	
	inline int get_direction(int dirs) const {
		switch(dirs) {
			case 8: return get_direction8();
			case 16: return get_direction16();
			case 1: return 1;
		}
		return 0; //make msvc happy
	}

	static inline void quantize8(T &x) {
		if (x > T(0.3826834323650898373)) {
			x = 1;
		} else if (x < T(-0.3826834323650898373))
			x = -1;
		else x = 0;
	}

	
	inline void quantize8() {
		normalize();
		quantize8(x);
		quantize8(y);
		normalize();
	}

	inline void quantize16() {
		static T cos_t[] = { 1, T(0.9238795325112867385), T(0.7071067811865475727), T(0.3826834323650898373), 
						  0, T(-0.3826834323650898373), T(-0.7071067811865475727), T(-0.9238795325112867385), -1};
		static T sin_t[] = {0, T(0.3826834323650898373), T(0.7071067811865475727), T(0.9238795325112867385), 1,
						T(0.9238795325112867385), T(0.7071067811865475727), T(0.3826834323650898373), 0 };
		normalize();
		int xx = c2d16(x);
		x = cos_t[xx];
		if (y >= 0) 
			y = sin_t[xx];
		else 
			y = -sin_t[xx];
	}
	
	virtual void serialize(mrt::Serializator &s) const {
		s.add(x);
		s.add(y);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		s.get(x);
		s.get(y);
	}

	void fromString(const std::string &str) {
		clear();
		if (typeid(T) == typeid(int)) {
			if (sscanf(str.c_str(), "%d,%d", &x, &y) < 2)
				throw std::invalid_argument("cannot parse %d,%d from " + str);
		} else throw std::invalid_argument("invalid type T. only int allowed for fromString()");
	}
	
	void fromDirection(const int dir, const int total) {
		static const float cos_vt16[] = {1.0f,0.92388f,0.707107f,0.382683f,0.0f,-0.382683f,-0.707107f,-0.92388f,-1.0f,-0.92388f,-0.707107f,-0.382683f,0.0f,0.382683f,0.707107f,0.92388f,};
		static const float sin_vt16[] = {0.0f,0.382683f,0.707107f,0.92388f,1.0f,0.92388f,0.707107f,0.382683f,0.0f,-0.382683f,-0.707107f,-0.92388f,-1.0f,-0.92388f,-0.707107f,-0.382683f,};
		static const float cos_vt8[] = {1.0f,0.707107f,0.0f,-0.707107f,-1.0f,-0.707107f,0.0f,0.707107f,};
		static const float sin_vt8[] = {0.0f,0.707107f,1.0f,0.707107f,0.0f,-0.707107f,-1.0f,-0.707107f,};
		
		if (total != 4 && total != 8 && total != 16)
			throw std::invalid_argument("fromDirection() supports 4, 8 or 16 directions.");
		if (dir < 0 || dir >= total)
			throw std::invalid_argument("direction is greater than total direction count.");
		
		if (total != 16) {
			int idx = dir * (8 / total);
			x = cos_vt8[idx];
			y = -sin_vt8[idx];
		} else {
			x = cos_vt16[dir];
			y = -sin_vt16[dir];
		}
	}

	const v3<T> convert2v3(const T z) {
		return v3<T>(x, y, z);
	}
};

template <typename T>
	const v2<T> operator+(const T a, const v2<T> &v)  {
		return v2<T>(v.x + a, v.y + a);
	}

template <typename T>
	const v2<T> operator+(const v2<T> &v, const T a)  {
		return v2<T>(v.x + a, v.y + a);
	}


template <typename T>
	const v2<T> operator*(const T a, const v2<T> &v)  {
		return v2<T>(v.x *a, v.y * a);
	}

template <typename T>
	const v2<T> operator*(const v2<T> &v, const T a)  {
		return v2<T>(v.x *a, v.y * a);
	}

template <typename T>
	const v2<T> operator/(const T a, const v2<T> &v)  {
		return v2<T>(a / v.x, a / v.y);
	}

template <typename T>
	const v2<T> operator/(const v2<T> &v, const T a)  {
		return v2<T>(v.x / a, v.y  / a);
	}

template <typename T>
	const v2<T> operator%(const v2<T> &v, const T a)  {
		return v2<T>(v.x % a, v.y % a);
	}

#endif


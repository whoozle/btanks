#ifndef __BTANKS_V3_H__
#define __BTANKS_V3_H__

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


#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "mrt/serializator.h"
#include "mrt/serializable.h"

#include <string>
#include <stdexcept>
#include "mrt/exception.h"
#include <typeinfo>

template <typename T> class v3 : public mrt::Serializable {
public:
	T x, y, z;
	inline v3<T>() : x(0), y(0), z(0) {}
	inline v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 
	static const v3<T> empty;

	inline void clear() { x = y = z = 0; }
	inline const bool is0() const {
#ifndef V3_DISABLE_Z		
		return x== 0 && y == 0 && z == 0;
#else
		return x== 0 && y == 0;
#endif
	}
	
	inline const T normalize() {
		const T len = length();
		if (len == (T)0 || len ==(T)1) 
			return len;
		
		x /= len;
		y /= len;
		z /= len;
		return (T)1;
	}

	inline const T normalize(const T nlen) {
		const T len = length();
		if (len == (T)0 || len == nlen) 
			return len;
		
		x *= nlen / len;
		y *= nlen / len;
		z *= nlen / len;
		return nlen;
	}
	
	inline const T length() const {
#ifndef V3_DISABLE_Z		
		if (x == 0 && y == 0 && z == 0) 
			return 0;
		
		return (T)sqrt(x * x + y * y + z * z);
#else 
		if (x == 0 && y == 0) 
			return 0;
		
		return (T)sqrt(x * x + y * y);
#endif
	}

	inline const T quick_length() const {
#ifndef V3_DISABLE_Z		
		return (T)(x * x + y * y + z * z);
#else
		return (T)(x * x + y * y);
#endif
	}

	template <typename T2> 
		inline v3<T2> convert() const { return v3<T2>((T2)x, (T2)y, (T2)z); }
	
	inline const T distance(const v3<T>& other) const {
		v3<T>d(*this);
		d-= other;
		return d.length();
	}
	
	
	inline const T quick_distance(const v3<T>& other) const {
		const T dx = x - other.x;
		const T dy = y - other.y;
#ifndef V3_DISABLE_Z		
		const T dz = z - other.z;
		return (dx * dx + dy * dy + dz * dz);
#else
		return (dx * dx + dy * dy);
#endif
	}

	inline const T scalar(const v3<T> &other) const {
#ifndef V3_DISABLE_Z		
		return (x * other.x + y * other.y + z * other.z);
#else
		return (x * other.x + y * other.y);
#endif
	}

	inline const bool same_sign(const v3<T> &other) const {
		
		if (x == 0) {
			if (other.x != 0)
				return false;
		} else { //x != 0
			if (other.x * x < 0)
				return false;
		}

		if (y == 0) {
			if (other.y != 0)
				return false;
		} else { //y != 0
			if (other.y * y < 0)
				return false;
		}
#ifndef V3_DISABLE_Z		
		if (z == 0) {
			if (other.z != 0)
				return false;
		} else { //z != 0
			if (other.z * z < 0)
				return false;
		}
#endif		
		return true;
	}
	
	//operators 
	inline const bool operator<(const v3<T> &other) const {
		return x < other.x && y < other.y && z < other.z;
	}


	inline const v3<T> operator-() const {
		return v3<T>(-x, -y, -z);
	}
	
	inline const bool operator==(const v3<T> &other) {
		return x == other.x && y == other.y && z == other.z;
	}

	inline const bool operator!=(const v3<T> &other) {
		return x != other.x || y != other.y || z != other.z;
	}
	

	inline const v3<T>& operator+=(const v3<T>& other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	inline const v3<T>& operator-=(const v3<T>& other) {
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}

	inline const v3<T>& operator*=(const v3<T>& other) {
		x *= other.x; y *= other.y; z *= other.z;
		return *this;
	}

	inline const v3<T>& operator/=(const v3<T>& other) {
		x /= other.x; y /= other.y; z /= other.z;
		return *this;
	}
	
	inline const v3<T> operator*(const v3<T>& other) const {
		return v3<T>(x * other.x, y * other.y, z * other.z);
	}
	inline const v3<T> operator+(const v3<T>& other) const {
		return v3<T>(x + other.x, y + other.y, z + other.z);
	}
	inline const v3<T> operator-(const v3<T>& other) const {
		return v3<T>(x - other.x, y - other.y, z - other.z);
	}
	inline const v3<T> operator/(const v3<T>& other) const {
		return v3<T>(x / other.x, y / other.y, z / other.z);
	}

	inline const v3<T> operator*(const T& other) const {
		return v3<T>(x * other, y * other, z * other);
	}
	inline const v3<T> operator+(const T& other) const {
		return v3<T>(x + other, y + other, z + other);
	}
	inline const v3<T> operator-(const T& other) const {
		return v3<T>(x - other, y - other, z - other);
	}
	inline const v3<T> operator/(const T& other) const {
		return v3<T>(x / other, y / other, z / other);
	}

	inline const v3<T>& operator/=(const T& other) {
		x /= other;
		y /= other;
		z /= other;
		return *this;
	}

	inline const v3<T>& operator*=(const T& other) {
		x *= other;
		y *= other;
		z *= other;
		return *this;
	}

	inline const v3<T>& operator+=(const T& other) {
		x += other;
		y += other;
		z += other;
		return *this;
	}

	inline const v3<T>& operator-=(const T& other) {
		x -= other;
		y -= other;
		z -= other;
		return *this;
	}

private:
	static inline int c2d8(const T c) {
		if (c > 0.9238795325112867385) //cos(22.5)
			return 0;
		else if (c > 0.3826834323650898373) //cos(67.5)
			return 1;
		else if (c > -0.3826834323650898373)
			return 2;
		else if (c > -0.9238795325112867385)
			return 3;
		return 4;
	}

	static inline int c2d16(const T c) {
		if (c > 0.9807852804032304306) //11.25
			return 0;
		else if (c > 0.8314696123025452357) //cos(33.75)
			return 1;
		else if (c > 0.5526644777167217804) //cos 56.45
			return 2;
		else if (c > 0.1916655539320546719) //cos 78.95
			return 3;
		else if (c > -0.1916655539320546719) 
			return 4;
		else if (c > -0.5526644777167217804)
			return 5;
		else if (c > -0.8314696123025452357)
			return 6;
		else if (c > -0.9807852804032304306) //11.25
			return 7;
		return 8;
	}

public:	
	inline int getDirection8() const {
		if (is0())
			return 0;

		int xx = c2d8(x) + 1;
		return (y <= 0 || xx == 1)? xx: 10 - xx;
	}

	inline int getDirection16() const {
		if (is0())
			return 0;

		int xx = c2d16(x) + 1;
		return (y <= 0 || xx == 1)? xx: 18 - xx;
	}
	
	inline int getDirection(int dirs) {
		switch(dirs) {
			case 8: return getDirection8();
			case 16: return getDirection16();
			case 1: return 1;
		}
		return 0; //make msvc happy
	}

	static inline void quantize8(T &x) {
		if (x > 0.3826834323650898373) {
			x = 1;
		} else if (x < -0.3826834323650898373)
			x = -1;
		else x = 0;
	}

	
	inline void quantize8() {
		normalize();
		quantize8(x);
		quantize8(y);
		quantize8(z);
		normalize();
	}

	inline void quantize16() {
		static T cos_t[] = {1, 0.9238795325112867385, 0.7071067811865475727, 0.3826834323650898373, 
						  0, -0.3826834323650898373, -0.7071067811865475727, -0.9238795325112867385, -1};
		static T sin_t[] = {0, 0.3826834323650898373, 0.7071067811865475727, 0.9238795325112867385, 1,
						0.9238795325112867385, 0.7071067811865475727, 0.3826834323650898373, 0 };
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
		s.add(z);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		s.get(x);
		s.get(y);
		s.get(z);
	}

	void fromString(const std::string &str) {
		clear();
		if (typeid(T) == typeid(int)) {
			if (sscanf(str.c_str(), "%d,%d,%d", &x, &y, &z) < 2)
				throw std::invalid_argument("cannot parse %d,%d,%d from " + str);
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
		z = 0;
		
		if (total != 16) {
			int idx = dir * (8 / total);
			x = cos_vt8[idx];
			y = -sin_vt8[idx];
		} else {
			x = cos_vt16[dir];
			y = -sin_vt16[dir];
		}
	}
};

template <typename T> const v3<T> v3<T>::empty;


template <typename T>
	const v3<T> operator+(const T a, const v3<T> &v)  {
		return v3<T>(v.x + a, v.y + a, v.z + a);
	}

template <typename T>
	const v3<T> operator+(const v3<T> &v, const T a)  {
		return v3<T>(v.x + a, v.y + a, v.z + a);
	}


template <typename T>
	const v3<T> operator*(const T a, const v3<T> &v)  {
		return v3<T>(v.x *a, v.y * a, v.z * a);
	}

template <typename T>
	const v3<T> operator*(const v3<T> &v, const T a)  {
		return v3<T>(v.x *a, v.y * a, v.z * a);
	}

template <typename T>
	const v3<T> operator/(const T a, const v3<T> &v)  {
		return v3<T>(a / v.x, a / v.y, a / v.z);
	}

template <typename T>
	const v3<T> operator/(const v3<T> &v, const T a)  {
		return v3<T>(v.x / a, v.y  / a, v.z / a);
	}

#endif


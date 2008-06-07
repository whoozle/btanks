#ifndef CLUNK_V3_H__
#define CLUNK_V3_H__

/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2007-2008 Netive Media Group
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

#include <math.h>
namespace clunk {

template <typename T> class v3  {
public:
	T x, y, z;
	inline v3<T>() : x(0), y(0), z(0) {}
	inline v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 

	inline void clear() { x = y = z = 0; }
	inline const bool is0() const {
		return x== 0 && y == 0 && z == 0;
	}
	
	inline const T normalize() {
		const T len = length();
		if (len == (T)0 || len ==(T)1) 
			return len;
		
		x /= len;
		y /= len;
		z /= len;
		return len;
	}

	inline const T normalize(const T nlen) {
		const T len = length();
		if (len == (T)0 || len == nlen) 
			return len;
		
		x *= nlen / len;
		y *= nlen / len;
		z *= nlen / len;
		return len;
	}
	
	inline const T length() const {
		const T ql = x * x + y * y + z * z;
		if (ql == (T)0 || ql == (T)1) 
			return ql;
		
		return (T)sqrt(ql);
	}

	inline const T quick_length() const {
		return (T)(x * x + y * y + z * z);
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
		const T dz = z - other.z;
		return (dx * dx + dy * dy + dz * dz);
	}

	//operators 
	inline const bool operator<(const v3<T> &other) const {
		if (x != other.x) {
			return x < other.x;
		}
		if (y != other.y) {
			return y < other.y;
		} 
		return z < other.z;
	}


	inline const v3<T> operator-() const {
		return v3<T>(-x, -y, -z);
	}
	
	inline const bool operator==(const v3<T> &other) const {
		return x == other.x && y == other.y && z == other.z;
	}

	inline const bool operator!=(const v3<T> &other) const {
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
};
	
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
	
} //namespace clunk

#endif


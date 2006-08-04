#ifndef __BTANKS_V3_H__
#define __BTANKS_V3_H__

#include <stdio.h>
#include <math.h>

#include "mrt/serializator.h"
#include "mrt/serializable.h"

#include <string>
#include <stdexcept>
#include <typeinfo>

template <typename T> class v3 : public mrt::Serializable {
public:
	T x, y, z;
	v3<T>() : x(0), y(0), z(0) {}
	v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 

	void clear() { x = y = z = 0; }
	const bool is0() const {
		return x== 0 && y == 0 && z == 0;
	}
	
	const T normalize() {
		T len = lenght();
		if (len == 0 || len == 1) 
			return len;
		
		x /= len;
		y /= len;
		z /= len;
		return 1;
	}
	
	const T lenght() const {
		if (x == 0 && y == 0 && z == 0) 
			return 0;
		
		return (T)sqrt(x * x + y * y + z * z);
	}

	template <typename T2> 
		v3<T2> convert() const { return v3<T2>((T2)x, (T2)y, (T2)z); }
	
	const T distance(const v3<T>& other) const {
		v3<T>d(this);
		d-= other;
		return d.lenght();
	}
	
	const T quick_distance(const v3<T>& other) const {
		T dx = x - other.x;
		T dy = y - other.y;
		T dz = z - other.z;
		return (dx * dx + dy * dy + dz * dz);
	}

	
	//operators 
	const bool operator==(const v3<T> &other) {
		return x == other.x && y == other.y && z == other.z;
	}

	const bool operator!=(const v3<T> &other) {
		return x != other.x || y != other.y || z != other.z;
	}
	

	const v3<T>& operator+=(const v3<T>& other) {
		x += other.x; y += other.y; z += other.z;
		return *this;
	}

	const v3<T>& operator-=(const v3<T>& other) {
		x -= other.x; y -= other.y; z -= other.z;
		return *this;
	}
	
	const v3<T> operator*(const v3<T>& other) const {
		return v3<T>(x * other.x, y * other.y, z * other.z);
	}
	const v3<T> operator+(const v3<T>& other) const {
		return v3<T>(x + other.x, y + other.y, z + other.z);
	}
	const v3<T> operator-(const v3<T>& other) const {
		return v3<T>(x - other.x, y - other.y, z - other.z);
	}
	const v3<T> operator/(const v3<T>& other) const {
		return v3<T>(x / other.x, y / other.y, z / other.z);
	}

	const v3<T> operator*(const T& other) const {
		return v3<T>(x * other, y * other, z * other);
	}
	const v3<T> operator+(const T& other) const {
		return v3<T>(x + other, y + other, z + other);
	}
	const v3<T> operator-(const T& other) const {
		return v3<T>(x - other, y - other, z - other);
	}
	const v3<T> operator/(const T& other) const {
		return v3<T>(x / other, y / other, z / other);
	}

private:
	static inline int c2d(const T c) {
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

public:
	static int getDirection8(const v3<T> &v) {
		if (v.is0())
			return 0;

		int x = c2d(v.x) + 1;
		if (v.y <= 0) 
			return x;
		else {
			//LOG_DEBUG(("%d %d", x, 10 - x));
			return 10 - x;
		}
		return 0;
	}

	static void quantize(T &x) {
		if (x > 0.3826834323650898373) {
			x = 1;
		} else if (x < -0.3826834323650898373)
			x = -1;
		else x = 0;
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

#endif


#ifndef __BTANKS_V3_H__
#define __BTANKS_V3_H__

#include <math.h>

template <typename T> class v3 {
public:
	T x, y, z;
	v3<T>() : x(0), y(0), z(0) {}
	v3<T>(const T x, const T y, const T z) : x(x), y(y), z(z) {} 

	void clear() { x = y = z = 0; }
	const T normalize() {
		T len = (T)sqrt(x * x + y * y + z * z);
		if (len != 0) {
			x /= len;
			y /= len;
			z /= len;
			return 1;
		}
		return 0;
	}
	const T lenght() const {
		return (T)sqrt(x * x + y * y + z * z);
	}
	
	//operators 
	const v3<T>& operator+=(const v3<T>& other) {
		x += other.x;
		y += other.y;
		z += other.z;
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
	
	template <typename T2> 
		v3<T2> convert() const { return v3<T2>((T2)x, (T2)y, (T2)z); }
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


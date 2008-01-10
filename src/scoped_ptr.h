#ifndef BTANKS_SCOPED_PTR_H__
#define BTANKS_SCOPED_PTR_H__

template<typename T>
class scoped_ptr {
public: 
	explicit inline scoped_ptr(T *ptr): ptr(ptr) {}
	inline ~scoped_ptr() { delete ptr; }
	
	inline T* operator->() { return ptr; }
	inline T& operator*() { return *ptr; }
	inline T * release() { T * r = ptr; ptr = 0; return r; }

private:
	scoped_ptr(const scoped_ptr<T>& other);
	const scoped_ptr<T> &operator=(const scoped_ptr<T>& other);
	T * ptr;
};

#endif


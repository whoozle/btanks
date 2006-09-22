#ifndef __SDLX_MUTEX_H__
#define __SDLX_MUTEX_H__

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

namespace sdlx {

class Mutex {
public: 
	Mutex();
	~Mutex();
private: 
	void lock() const;
	void unlock() const;

	SDL_mutex *_mutex;
	
	friend class AutoMutex;
};

class AutoMutex {
public: 
	AutoMutex(const Mutex &m, const bool lock = true);
	void lock() const;
	void unlock() const;
	~AutoMutex();
private:
	const Mutex &_mutex;
	mutable bool _locked;
};

}

#endif


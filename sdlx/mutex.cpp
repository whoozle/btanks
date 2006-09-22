#include "mutex.h"
#include "sdl_ex.h"

using namespace sdlx;

Mutex::Mutex() : _mutex(SDL_CreateMutex()) {
	if (_mutex == NULL)
		throw_sdl(("SDL_CreateMutex"));
}

Mutex::~Mutex() {
	SDL_DestroyMutex(_mutex);
}

void Mutex::lock() const {
	if (SDL_mutexP(_mutex) != 0)
		throw_sdl(("SDL_LockMutex"));
}

void Mutex::unlock() const {
	if (SDL_mutexV(_mutex) != 0)
		throw_sdl(("SDL_UnlockMutex"));
}


AutoMutex::AutoMutex(const Mutex &m, const bool lock) : _mutex(m), _locked(lock) {
	if (lock)
		_mutex.lock();
}

void AutoMutex::lock() const {
	_mutex.lock();
	_locked = true;
}

void AutoMutex::unlock() const {
	_mutex.unlock();
	_locked = false;
}

AutoMutex::~AutoMutex() {
	if (_locked)
		_mutex.unlock();
}

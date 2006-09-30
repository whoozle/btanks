#include "semaphore.h"
#include "sdl_ex.h"

using namespace sdlx;

Semaphore::Semaphore(const Uint32 value) {
	if ((_sem = SDL_CreateSemaphore(value)) == NULL) 
		throw_sdl(("SDL_CreateSemaphore"));
}

void Semaphore::post() {
	if (SDL_SemPost(_sem) == -1)
		throw_sdl(("SDL_SemPost"));
}

void Semaphore::wait() {
	if (SDL_SemWait(_sem) == -1)
		throw_sdl(("SDL_SemWait"));
}

const bool Semaphore::wait(const Uint32 timeout) {
	int r = SDL_SemWaitTimeout(_sem, timeout);
	switch(r) {
		case 0: return true;
		case SDL_MUTEX_TIMEDOUT: return false;
	}	
	throw_sdl(("SDL_SemWaitTimeout"));
	return false;
}

const bool Semaphore::tryWait() {
	int r = SDL_SemTryWait(_sem);
	switch(r) {
		case 0: return true;
		case SDL_MUTEX_TIMEDOUT: return false;
	}	
	throw_sdl(("SDL_SemTryWait"));
	return false;
}

Semaphore::~Semaphore() {
	SDL_DestroySemaphore(_sem);
	_sem = NULL;
}

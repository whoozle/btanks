#ifndef __SDLX_SEMAPHORE_H__
#define __SDLX_SEMAPHORE_H__

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

namespace sdlx {
class Semaphore {
public:
	Semaphore(const Uint32 value = 0);
	~Semaphore();

	void post();
	void wait();

	const bool wait(const Uint32 timeout);
	const bool tryWait(); 

private: 
	SDL_sem *_sem;
};
}

#endif

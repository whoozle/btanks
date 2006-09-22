#ifndef __SDLX__THREAD_H__
#define __SDLX__THREAD_H__

#include <SDL/SDL.h>
#include <SDL/SDL_thread.h>

namespace sdlx {

class Thread {
public:
	Thread();
	virtual ~Thread();

	void start();

	virtual const int run() = 0;

	Uint32 getID() const;
	
	const int wait();
	void kill();
private: 
	SDL_Thread * _thread;
};

}

#endif

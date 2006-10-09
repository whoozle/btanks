#include "thread.h"
#include "sdl_ex.h"
#include "mrt/logger.h"

using namespace sdlx;

Thread::Thread() : _thread(NULL) {}

static int thread_starter(void *o) {
	TRY {
		Thread *t = reinterpret_cast<Thread *>(o);
		return t->run();
	} CATCH("thread::run", );
	return -1;
}

Thread::~Thread() {
	if (_thread != NULL) 
		LOG_WARN(("~Thread: thread %x was not stopped", getID()));
}

Uint32 Thread::getID() const {
	if (_thread == NULL)
		throw_sdl(("getID: thread was not started"));
	
	return SDL_GetThreadID(_thread);
}


void Thread::start() {
	_thread = SDL_CreateThread(thread_starter, reinterpret_cast<void *>(this));
}

const int Thread::wait() {
	if (_thread == NULL)
		throw_sdl(("wait: thread was not started"));
	int r;
	SDL_WaitThread(_thread, &r);
	_thread = NULL;
	return r;
}

void Thread::kill() {
	if (_thread == NULL)
		throw_sdl(("kill: thread was not started"));
	SDL_KillThread(_thread);
	_thread = NULL;
}

#ifndef CLUNK_LOCKER_H__
#define CLUNK_LOCKER_H__

#include <SDL_audio.h>
#include "export_clunk.h"

struct CLUNKAPI AudioLocker {
	AudioLocker () {
		SDL_LockAudio();
	}
	~AudioLocker() {
		SDL_UnlockAudio();
	}
};


#endif


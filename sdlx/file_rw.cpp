#include "file_rw.h"
#include "mrt/base_file.h"
#include <SDL_rwops.h>
#include "sdl_ex.h"
#include <assert.h>

static int mrt_seek(struct SDL_RWops *context, int offset, int whence) {
	assert(context->hidden.unknown.data1 != NULL);
	const mrt::BaseFile *file = (const mrt::BaseFile *)context->hidden.unknown.data1;
	file->seek(offset, whence);
	return file->tell();
}

static int mrt_read(struct SDL_RWops *context, void *ptr, int size, int maxnum) {
	assert(context->hidden.unknown.data1 != NULL);
	const mrt::BaseFile *file = (const mrt::BaseFile *)context->hidden.unknown.data1;
	int r = file->read(ptr, size * maxnum);
	return (r > 0) ? r / size : r;
}

SDL_RWops * sdlx::RWFromMRTFile(const mrt::BaseFile *file) {
	SDL_RWops * op = SDL_AllocRW();
	if (op == NULL)
		throw_sdl(("SDL_AllocRW()"));

	op->hidden.unknown.data1 = (void *)file; //sic! 
	
	op->seek = &mrt_seek;
	op->read = &mrt_read;
	op->write = NULL;
	op->close = NULL;
	return op;
}

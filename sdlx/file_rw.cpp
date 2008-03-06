#include "file_rw.h"
#include "mrt/base_file.h"
#include <SDL_rwops.h>
#include "sdl_ex.h"
#include <assert.h>

static int mrt_seek(struct SDL_RWops *context, int offset, int whence) {
	assert(context->hidden.unknown.data1 != NULL);
	const mrt::BaseFile *file = (const mrt::BaseFile *)context->hidden.unknown.data1;
	TRY {
		file->seek(offset, whence);
		return file->tell();
	} CATCH("mrt_seek", return -1);
}

static int mrt_read(struct SDL_RWops *context, void *ptr, int size, int maxnum) {
	assert(context->hidden.unknown.data1 != NULL);
	const mrt::BaseFile *file = (const mrt::BaseFile *)context->hidden.unknown.data1;
	TRY {
		int r = file->read(ptr, size * maxnum);
		return (r > 0) ? r / size : r;
	} CATCH("mrt_read", return -1);
}

static int mrt_close(struct SDL_RWops *context) {
	assert(context->hidden.unknown.data1 != NULL);
	mrt::BaseFile *file = (mrt::BaseFile *)context->hidden.unknown.data1;
	TRY {
		file->close();
		return 0;
	} CATCH("mrt_close", return -1;);
}

SDL_RWops * sdlx::RWFromMRTFile(mrt::BaseFile *file) {
	SDL_RWops * op = SDL_AllocRW();
	if (op == NULL)
		throw_sdl(("SDL_AllocRW()"));

	op->hidden.unknown.data1 = (void *)file; //sic! 
	
	op->seek = &mrt_seek;
	op->read = &mrt_read;
	op->write = NULL;
	op->close = &mrt_close;
	return op;
}

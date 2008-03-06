#include <SDL_rwops.h>
#include "sample.h"
#include "mrt/chunk.h"
#include "sdl_ex.h"

using namespace clunk;

Sample::Sample() {}

void Sample::init(const mrt::Chunk &data, int rate, const Uint16 format, const Uint8 channels) {
	SDL_RWops *op = SDL_RWFromConstMem(data.getPtr(), data.getSize());
	if (op == NULL)
		throw_sdl(("SDL_RWFromConstMem"));

	deinit();

	SDL_AudioSpec spec;
	memset(&spec, 0, sizeof(spec));

	spec.freq = rate;
	spec.format = format;
	spec.channels = channels;
	
	SDL_AudioSpec *r = SDL_LoadWAV_RW(op, 1, &spec, &data_ptr, &data_len);
	if (r == NULL)
		throw_sdl(("SDL_LoadWAV_RW"));
	this->spec = *r;
}

void Sample::deinit() {
	if (data_ptr != NULL) {
		SDL_FreeWAV(data_ptr);
		data_ptr = NULL;
		data_len = 0;
	}
}

Sample::~Sample() {
	deinit();
}

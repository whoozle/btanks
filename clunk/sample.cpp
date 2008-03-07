#include <SDL_rwops.h>
#include "sample.h"
#include "mrt/chunk.h"
#include "sdl_ex.h"
#include "context.h"
#include <math.h>

using namespace clunk;

Sample::Sample(Context *context) : context(context) {}

void Sample::generateSine(const int freq, const float len) {
	spec.freq = context->get_spec().freq;
	spec.channels = 1;
	spec.format = context->get_spec().format;

	mrt::Chunk data;
	unsigned size = ((int)(len * spec.freq)) * 2;
	data.setSize(size);

	static double a = 0;
	double da = freq * 2 * M_PI / spec.freq;
	//LOG_DEBUG(("da = %g", da));
	
	int n = size / 2;

	Sint16 * stream = (Sint16 *)data.getPtr();
	for(int i = 0; i < n; ++i) {
		*stream++ = (Sint16)(32767 * sin(a));
		//*stream++ = 0;
		a += da;
	}
	
	SDL_RWops *op = SDL_RWFromConstMem(data.getPtr(), data.getSize());
	if (op == NULL)
		throw_sdl(("SDL_RWFromConstMem"));
	SDL_AudioSpec *r = SDL_LoadWAV_RW(op, 1, &spec, &data_ptr, &data_len);
	if (r == NULL)
		throw_sdl(("SDL_LoadWAV_RW"));
	spec = *r;
}

void Sample::init(const mrt::Chunk &data, int rate, const Uint16 format, const Uint8 channels) {
	SDL_RWops *op = SDL_RWFromConstMem(data.getPtr(), data.getSize());
	if (op == NULL)
		throw_sdl(("SDL_RWFromConstMem"));

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

Sample::~Sample() {
	if (data_ptr != NULL) {
		SDL_FreeWAV(data_ptr);
		data_ptr = NULL;
		data_len = 0;
	}
}

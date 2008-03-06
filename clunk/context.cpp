#include <SDL.h>
#include <SDL_audio.h>
#include "context.h"
#include <string.h>
#include "sdl_ex.h"
#include "mrt/logger.h"
#include <assert.h>

using namespace clunk;

Context::Context() : period_size(0) {
}

void Context::callback(void *userdata, Uint8 *stream, int len) {
	Context *self = (Context *)userdata;
	assert(self != NULL);
	LOG_DEBUG(("requested %d bytes!", len));
}


void Context::init(const int sample_rate, const Uint8 channels, int period_size) {
	SDL_AudioSpec src;
	memset(&src, 0, sizeof(src));
	src.freq = sample_rate;
	src.channels = channels;
	src.format = AUDIO_S16LSB;
	src.samples = period_size;
	src.callback = &Context::callback;
	src.userdata = (void *) this;
	
	this->period_size = period_size;
	
	if ( SDL_OpenAudio(&src, &spec) < 0 )
		throw_sdl(("SDL_OpenAudio(%d, %u, %d)", sample_rate, channels, period_size));
	if (src.format != AUDIO_S16LSB)
		throw_ex(("SDL_OpenAudio(%d, %u, %d) returned format %d", sample_rate, channels, period_size, spec.format));
	LOG_DEBUG(("opened audio device, sample rate: %d, period: %d", spec.freq, spec.samples));
//	SDL_InitSubSystem(SDL_INIT_AUDIO);
	SDL_PauseAudio(0);
}


void Context::deinit() {
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
	
Context::~Context() {
	deinit();
}

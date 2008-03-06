#include <SDL.h>
#include <SDL_audio.h>
#include "context.h"
#include <string.h>
#include "sdl_ex.h"
#include "mrt/logger.h"
#include <assert.h>
#include <math.h>

using namespace clunk;

Context::Context() : period_size(0) {
}

void Context::callback(void *userdata, Uint8 *bstream, int len) {
	Context *self = (Context *)userdata;
	assert(self != NULL);
	//LOG_DEBUG(("requested %d bytes!", len));
	Sint16 *stream = (Sint16*)bstream;
	
	static double a = 0;
	double da = 440 * 2 * M_PI / self->spec.freq;
	//LOG_DEBUG(("da = %g", da));
	
	int n = len / 2 / self->spec.channels;
	for(int i = 0; i < n; ++i) {
		*stream++ = (Sint16)(32767 * sin(a));
		*stream++ = (Sint16)(32767 * sin(a + self->spec.freq / 10.0)); //sample delay
		//*stream++ = 0;
		a += da;
	}
}

Object *Context::createObject() {
	return new Object(this);
}

Sample *Context::createSample() {
	return new Sample(this);
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

void Context::deleteObject(Object *o) {
	//blah blah
}

void Context::deinit() {
	SDL_PauseAudio(1);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
	
Context::~Context() {
	deinit();
}

#include <SDL.h>
#include <SDL_audio.h>
#include "context.h"
#include <string.h>
#include "sdl_ex.h"
#include "mrt/logger.h"
#include "source.h"
#include <assert.h>
#include <math.h>
#include <map>

using namespace clunk;

struct AudioLocker {
	AudioLocker () {
		SDL_LockAudio();
	}
	~AudioLocker() {
		SDL_UnlockAudio();
	}
};

Context::Context() : period_size(0), max_sources(8) {
}

void Context::callback(void *userdata, Uint8 *bstream, int len) {
	Context *self = (Context *)userdata;
	assert(self != NULL);
	Sint16 *stream = (Sint16*)bstream;
	TRY {
		self->process(stream, len);
	} CATCH("callback", )
}

void Context::process(Sint16 *stream, int size) {
	typedef std::multimap<const float, Source *> sources_type;
	sources_type sources;
	
	for(objects_type::iterator i = objects.begin(); i != objects.end(); ++i) {
		Object *o = *i;
		v3<float> base = o->position;
		std::set<Source *> & sset = o->sources;
		for(std::set<Source *>::iterator j = sset.begin(); j != sset.end(); ++j) {
			Source *s = *j;
			v3<float> position = base + s->delta_position;
			float dist = position.distance(base);
			if (sources.size() < max_sources) {
				sources.insert(sources_type::value_type(dist, s));
			} else {
				if (sources.rbegin()->first <= dist) 
					continue;
				//sources.erase(sources.rbegin());
				sources.insert(sources_type::value_type(dist, s));
			}
		}
	}
	std::vector<Source *> lsources;
	sources_type::iterator j = sources.begin();
	for(unsigned i = 0; i < max_sources && j != sources.end(); ++i, ++j) {
		LOG_DEBUG(("%u: source in %g", i, j->first));
		lsources.push_back(j->second);
	}
	sources.clear();

	unsigned n = size / spec.channels / 2;
	LOG_DEBUG(("generating %u samples", n));
}


Object *Context::create_object() {
	AudioLocker l;
	Object *o = new Object(this);
	objects.insert(o);
	return o;
}

Sample *Context::create_sample() {
	AudioLocker l;
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

void Context::delete_object(Object *o) {
	AudioLocker l;
	objects.erase(o);
}

void Context::deinit() {
	SDL_PauseAudio(1);
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
	
Context::~Context() {
	deinit();
}

#define _USE_MATH_DEFINES
#include <math.h>
#include <SDL_rwops.h>
#include "sample.h"
#include "mrt/chunk.h"
#include "sdl_ex.h"
#include "context.h"
#include "locker.h"

using namespace clunk;

Sample::Sample(Context *context) : context(context) {}

void Sample::generateSine(const int freq, const float len) {
	AudioLocker l;
	
	spec.freq = context->get_spec().freq;
	spec.channels = 1;
	spec.format = context->get_spec().format;

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
	LOG_DEBUG(("generated %u bytes", (unsigned)data.getSize()));
}

void Sample::init(const mrt::Chunk &src_data, int rate, const Uint16 format, const Uint8 channels) {
	AudioLocker l;

	spec.freq = context->get_spec().freq;
	spec.channels = 1; //fixme: do not 
	spec.format = context->get_spec().format;
	//fixme: check format	
	unsigned len = ((format & 0xff) - 1) / 8 + 1;
	if (len != 2) 
		throw_ex(("unsupported data format (%u bps)", len));

	data.setSize(src_data.getSize() * spec.freq / rate / channels);
	
	Sint16 *dst = (Sint16 *)data.getPtr();
	Sint16 *src = (Sint16 *)src_data.getPtr();
	for(size_t i = 0; i < data.getSize() / 2; ++i) {
		int v = 0;
		int offset = i * rate * channels / spec.freq;
		for(int j = 0; j < channels; ++j) {
			v += src[offset + j];
		}
		v /= channels;
		*dst++ = v;
	}
}

Sample::~Sample() {
	
}

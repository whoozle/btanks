#include "source.h"
#include <SDL.h>
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "sample.h"

using namespace clunk;
Source::Source(const Sample * sample, const bool loop, const v3<float> &delta, float gain, float pitch) : 
	sample(sample), loop(loop), delta_position(delta), gain(gain), pitch(pow(2.0, pitch)), position(0) {}

float Source::process(mrt::Chunk &buffer, unsigned dst_ch, const v3<float> &delta_position) {
	LOG_DEBUG(("delta position: %g %g", delta_position.x, delta_position.y));
	float r2 = delta_position.quick_length();
	if (r2 < 1)
		r2 = 1;
	float v = gain / r2;
	if (v < 0)
		return 0;
	if (v > 1)
		v = 1;
	
	Sint16 * src = (Sint16*) sample->data_ptr;
	if (src == NULL)
		throw_ex(("uninitialized sample used (%p)", (void *)sample));

	unsigned src_ch = sample->spec.channels; 
	Sint16 * dst = (Sint16*) buffer.getPtr();
	
	unsigned dst_n = buffer.getSize() / dst_ch / 2;
	unsigned src_n = sample->data_len / src_ch / 2;
	
	for(unsigned i = 0; i < dst_n; ++i) {
		int p = (position + (int)(i * pitch)) % src_n;
		if (p < 0)
			p += src_n;
		
		Sint16 *src_v = src + p * src_ch;
		int v = 0;
		for(unsigned c = 0; c < src_ch; ++c) {
			v += src_v[c];
		}
		v /= src_ch; //mono sample.
		for(unsigned c = 0; c < dst_ch; ++c) {
			dst[i * dst_ch + c] = v;
			//LOG_DEBUG(("%u -> %d", i * dst_ch + c,  v));
		}
	}
	position += ((int)(dst_n * pitch)) % src_n;
	return v;
}

#ifndef CLUNK_SOURCE_H__
#define CLUNK_SOURCE_H__

#include "v3.h"
#include <SDL_audio.h>

namespace mrt {
	class Chunk;
}

namespace clunk {

class Sample;
class Source {
public:
	const Sample * const sample;

	bool loop;
	v3<float> delta_position; //0 - from the center of the object. 
	float gain;
	float pitch;
	
	Source(const Sample * sample, const bool loop = false, const v3<float> &delta = v3<float>(), float gain = 1, float pitch = 0);

	float process(mrt::Chunk &buffer, unsigned ch, const v3<float> &position);

private: 
	typedef const float (*kemar_ptr)[2][512];
	void get_kemar_data(kemar_ptr & kemar_data, int & samples, const v3<float> &delta_position);

	void idt(const v3<float> &delta, float &idt_offset, float &angle_gr);
	void hrtf(mrt::Chunk &result, int dst_n, const Sint16 *src, int src_ch, int src_n, const kemar_ptr& kemar_data, int kemar_idx);

	int position;
};
}

#endif

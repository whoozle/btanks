#ifndef CLUNK_SOURCE_H__
#define CLUNK_SOURCE_H__

#include "v3.h"

namespace clunk {

class Sample;
class Source {
public:
	const Sample * const sample;

	bool loop;
	v3<float> delta_position; //0 - from the center of the object. 
	float gain;
	float pitch;
	
	Source(const Sample * sample, const bool loop = false, const v3<float> &delta = v3<float>(), float gain = 1, float pitch = 1);

private: 
	int position;
};
}

#endif

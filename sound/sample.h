#ifndef __BTANKS_SAMPLE_H__
#define __BTANKS_SAMPLE_H__

#include <AL/al.h>
#include "mrt/chunk.h"

class Sample {
public:
	mrt::Chunk data;
	ALenum format;
	ALsizei rate;
	ALuint buffer;
	
	void init();
	~Sample();
};

#endif

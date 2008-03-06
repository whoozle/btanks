#ifndef CLUNK_SAMPLE_H__
#define CLUNK_SAMPLE_H__

#include <SDL_audio.h>
#include "export_clunk.h"

namespace mrt {
	class Chunk;
}

namespace clunk {
class Context;
class CLUNKAPI Sample {
public: 
	~Sample();
	void init(const mrt::Chunk &data, int rate, const Uint16 format, const Uint8 channels);
	void generateSine(const int freq);
	
private: 	
	friend class Context;
	Sample(Context *context);

	Sample(const Sample &);
	const Sample& operator=(const Sample &);

	Context *context;
	SDL_AudioSpec spec;
	Uint8 *data_ptr;
	Uint32 data_len;	
};
}

#endif


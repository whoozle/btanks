#ifndef CLUNK_STREAM_H__
#define CLUNK_STREAM_H__

#include "export_clunk.h"
#include <SDL_audio.h>

namespace mrt {
	class Chunk;
}

namespace clunk {
class Context;

class CLUNKAPI Stream {
public: 
	Stream();
	virtual void rewind() = 0;
	virtual bool read(mrt::Chunk &data, unsigned hint) = 0;
	virtual ~Stream();
protected: 
	int sample_rate;
	Uint16 format;
	Uint8 channels;

	friend class Context;
};
}

#endif


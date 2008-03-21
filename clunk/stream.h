#ifndef CLUNK_STREAM_H__
#define CLUNK_STREAM_H__

#include "export_clunk.h"

namespace mrt {
	class Chunk;
}

namespace clunk {
class CLUNKAPI Stream {
public: 
	virtual void rewind() = 0;
	virtual bool read(mrt::Chunk &data, unsigned hint) = 0;
	virtual ~Stream();
};
}

#endif


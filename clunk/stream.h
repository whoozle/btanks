#ifndef CLUNK_STREAM_H__
#define CLUNK_STREAM_H__

#include "export_clunk.h"

namespace mrt {
	class Chunk;
}

namespace clunk {
class CLUNKAPI Stream {
public: 
	virtual bool read(mrt::Chunk &data) = 0;
	virtual ~Stream();
};
}

#endif


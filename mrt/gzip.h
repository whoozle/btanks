#ifndef __MRT_GZIP_H__
#define __MRT_GZIP_H__

#include <zlib.h>

namespace mrt {

class Chunk;

class ZStream {
public:
	static void decompress(mrt::Chunk &dst, const mrt::Chunk &src);
private:
};

}


#endif


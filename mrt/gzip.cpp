#include "gzip.h"
#include "chunk.h"
#include "exception.h"
#include "logger.h"
#include <string.h>

using namespace mrt;

#define throw_z(method, ret) throw_ex(("zlib.%s failed: %s, code: %d", method, z.msg, ret));

#define BUF 0x10000

void ZStream::decompress(mrt::Chunk &dst, const mrt::Chunk &src) {
	z_stream z;
	memset(&z, 0, sizeof(z));
	try {
		int ret;
		z.avail_in = src.getSize();
		z.next_in = (Bytef*) src.getPtr();

		if ((ret = inflateInit2(&z, 0x1f)) != Z_OK)
			throw_z("inflateInit", ret);
		
		dst.setSize(BUF);
		size_t out_pos = 0;

		z.avail_out = dst.getSize();
		z.next_out = (Bytef*)dst.getPtr() + out_pos;
		
		do {
			//LOG_DEBUG(("inflate"));
			if (z.avail_in ==0)
				break; //fixme ?
			
			ret = inflate(&z, Z_NO_FLUSH);
			
			if (ret == Z_STREAM_END) 
				break;
			
			if (ret == Z_BUF_ERROR) {
				if (z.avail_out == 0) {
					LOG_DEBUG(("ran out of out buf"));
					dst.setSize(dst.getSize() + BUF);
					
					out_pos += BUF;
					z.next_out = (Bytef*)dst.getPtr() + out_pos;
					z.avail_out = BUF;
					continue;
				} else if (z.avail_in == 0) {
					throw_ex(("stream was truncated. unable to proceed."));
				}
			}

			if (ret != Z_OK)
				throw_z("inflate", ret);
			
		} while(1);
		if ((ret = inflateEnd(&z)) != Z_OK) 
			throw_z("inflateEnd", ret);
		
		dst.setSize(z.total_out);
	} CATCH("decompress", {inflateEnd(&z); throw;});
}

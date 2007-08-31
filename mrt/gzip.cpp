/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "gzip.h"
#include "chunk.h"
#include "exception.h"
#include "logger.h"
#include <string.h>

using namespace mrt;

#define throw_z(method, ret) throw_ex(("zlib.%s failed: %s, code: %d", method, z.msg, ret));

#define BUF 0x10000

void ZStream::decompress(mrt::Chunk &dst, const mrt::Chunk &src, const bool gzip_header) {
	z_stream z;
	memset(&z, 0, sizeof(z));
	try {
		int ret;
		z.avail_in = src.getSize();
		z.next_in = (Bytef*) src.getPtr();

		if ((ret = inflateInit2(&z, gzip_header?0x1f:0x0f)) != Z_OK)
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

void ZStream::compress(mrt::Chunk &dst, const mrt::Chunk &src, const bool gzip_header, const int level) {
	z_stream z;
	memset(&z, 0, sizeof(z));
	try {
		int ret;
		z.avail_in = src.getSize();
		z.next_in = (Bytef*) src.getPtr();

		if ((ret = deflateInit2(&z, level, Z_DEFLATED, gzip_header?0x1f:0x0f, 8, Z_DEFAULT_STRATEGY)) != Z_OK)
			throw_z("DeflateInit", ret);
		
		dst.setSize(BUF);
		size_t out_pos = 0;

		z.avail_out = dst.getSize();
		z.next_out = (Bytef*)dst.getPtr() + out_pos;
		
		do {
			//LOG_DEBUG(("deflate"));
			if (z.avail_in ==0)
				break; //fixme ?
			
			ret = deflate(&z, Z_FINISH);
			
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
				throw_z("deflate", ret);
			
		} while(1);
		if ((ret = deflateEnd(&z)) != Z_OK) 
			throw_z("deflateEnd", ret);
		
		dst.setSize(z.total_out);
	} CATCH("compress", {deflateEnd(&z); throw;});

}

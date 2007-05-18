#include "b64.h"
#include "chunk.h"
#include <assert.h>
#include "mrt/exception.h"

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

using namespace mrt;

void Base64::encode(std::string &dst, const mrt::Chunk &src, int linesize) {
	const unsigned char * p_src = (const unsigned char *)src.getPtr();
	size_t size = src.getSize();
	dst.clear();
	int lost = 0;
	while(size) {
		//process next 3 bytes.	
		static const char *zoo = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
		unsigned int src24 = 0;
		for(int i = 0; i < 3; ++i) {
			src24 <<= 8;
			if (size) {
				src24 |= *p_src;
				--size;
			} else ++lost;
		}
		assert(lost < 3);
		//LOG_DEBUG(("encode %08x [%d]", src24, lost));
		dst += 				   zoo[(src24 & 0xfc0000) >> 18];
		dst += 				   zoo[(src24 & 0x03f000) >> 12];
		dst += lost > 1 ? '=': zoo[(src24 & 0x000fc0) >> 6 ];
		dst += lost > 0 ? '=': zoo[(src24 & 0x00003f)];
	}
}

void Base64::decode(mrt::Chunk &dst, const std::string &src) {
	dst.setSize(3 * src.size() / 4);

	unsigned char * p_dst = (unsigned char *)dst.getPtr();
	unsigned int p_idx = 0;

	size_t size = src.size(), dst_size = dst.getSize();

	unsigned int dst24 = 0;
	int got = 0, padded = 0;
	for(size_t i = 0; i < size; ++i) {
		if (got < 4) {
			//decode next char
			char c = src[i];
			//ABCDE FGHIJ KLMNO PQRST UVWXY Zabcd efghi jklmn opqrs tuvwx yz012 34567 89+/
			if (c >= 'A' && c <= 'Z') {
				dst24 = (dst24 << 6) | (c - 'A');
				++got;
			} else if (c >= 'a' && c <= 'z') {
				dst24 = (dst24 << 6) | (c - 'a' + 26);
				++got;
			} else if (c >= '0' && c <= '9') {
				dst24 = (dst24 << 6) | (c - '0' + 52);
				++got;
			} else if (c == '+') {
				dst24 = (dst24 << 6) | 62;
				++got;
			} else if (c == '/') {
				dst24 = (dst24 << 6) | 63;
				++got;
			} else if (c == '=') {
				dst24 <<= 6;
				++got;
				++padded;
			}
			if (got < 4) 	
				continue;
		}
		//LOG_DEBUG(("storing %08x", dst24));
		if (padded > 2) 
			throw_ex(("invalid padding used (%d)", padded));
		
		assert(p_idx < dst_size);
		p_dst[p_idx++] = dst24 >> 16;
		
		if (padded < 2) {
			assert(p_idx < dst_size);
			p_dst[p_idx++] = (dst24 >> 8) & 0xff;
		}
		
		if (padded < 1) {
			assert(p_idx < dst_size);
			p_dst[p_idx++] = dst24 & 0xff;
		}
		dst24 = 0;
		got = 0;
		
		if (padded) 
			break;
	}
	dst.setSize(p_idx);
}

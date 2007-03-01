#include "b64.h"
#include "chunk.h"
#include <assert.h>

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

//code here derived from base64.sf.net
/*
** Translation Table as described in RFC1113
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Translation Table to decode (created by author) (base64.sf.net)
*/
static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

/*
** decodeblock
**
** decode 4 '6-bit' characters into 3 8-bit binary bytes
*/
static inline void decodeblock( unsigned char in[4], unsigned char out[3] )
{   
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

/*
** decode
**
** decode a base64 encoded stream discarding padding, line breaks and noise
*/
void Base64::decode(mrt::Chunk &dst, const std::string &src) {
    unsigned char in[4], out[3], v;
    int i, len;
	
	std::string::const_iterator fi = src.begin();
	dst.setSize(src.size());
	unsigned char *dst_p = static_cast<unsigned char *>(dst.getPtr());
	assert(dst_p != NULL);
	
	size_t dst_l = 0;

    while( fi != src.end() ) {
        for( len = 0, i = 0; i < 4 && fi != src.end(); i++ ) {
            v = 0;
            while( fi != src.end() && v == 0 ) {
                v = *fi++;
                v = (unsigned char) ((v < 43 || v > 122) ? 0 : cd64[ v - 43 ]);
                if( v ) {
                    v = (unsigned char) ((v == '$') ? 0 : v - 61);
                }
            }
            if( fi != src.end() ) {
                len++;
                if( v ) {
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            decodeblock( in, out );
            for( i = 0; i < len - 1; i++ ) {
				*dst_p++ = out[i];
				++dst_l;
            }
        }
    }
	dst.setSize(dst_l);
}


/*
** encodeblock
**
** encode 3 8-bit binary bytes as 4 '6-bit' characters
*/

static inline void encodeblock( unsigned char in[3], unsigned char out[4], int len )
{
    out[0] = cb64[ in[0] >> 2 ];
    out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
}

/*
** encode
**
** base64 encode a stream adding padding and line breaks as per spec.
*/

void Base64::encode(std::string &dst, const mrt::Chunk &src, int linesize ) {
    unsigned char in[3], out[4];
    int i, len, blocksout = 0;

	unsigned char *src_ptr = (unsigned char *)src.getPtr();
	size_t src_i = 0, src_size = src.getSize();
	
	dst.clear();
	
    while( src_i < src_size) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            in[i] = src_ptr[src_i++];
            if( src_i <= src_size ) {
                ++len;
            } else {
                in[i] = 0;
            }
        }
        if( len ) {
            encodeblock( in, out, len );
            dst += std::string((const char *)out, 4);
            blocksout++;
        }
        if(linesize != 0 && blocksout >= (linesize/4)) {
            if( blocksout ) {
                dst += "\r\n";
            }
            blocksout = 0;
        }
    }
}


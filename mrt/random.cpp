/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "random.h"
#include <time.h>
#include "serializator.h"

#ifdef _WINDOWS
#	ifndef uint64_t
#		define uint64_t unsigned __int64
#	endif
#	ifndef uint32_t
#		define uint32_t unsigned __int32
#	endif
#	ifndef uint16_t
#		define uint16_t unsigned __int16
#	endif
#else
#	include <stdint.h>
#endif

#include <assert.h>

static uint32_t mrt_rand_seed;

//LPRG
#if 0
static uint32_t rand_impl() {
	uint64_t x = mrt_rand_seed;
	x *= 279470273; 
	x %= 4294967291U;
	mrt_rand_seed = (uint32_t)x;
	return x;
}
#endif

#if 1
//LFSR
static uint32_t rand_impl() {
	return mrt_rand_seed = (mrt_rand_seed >> 1) ^ (-(mrt_rand_seed & 1u) & 0xd0000001u); /* taps 32 31 29 1 */
}
#endif

//#include "logger.h"

const int mrt::random(const unsigned max) {
	if (max < 2) 
		return 0;

	int bits;
	unsigned m = max;
	for(bits = 1; m >>= 1; ++bits);
	bits = (32 - bits) / 2; //take middle bits of the generated number
	
	unsigned x = rand_impl();
	/*
	unsigned len, n = max;
	for(len = 0; n != 0; n >>= 1, ++len);
	assert(len > 0 && len <= 32);
	len = (32 - len);
	//LOG_DEBUG(("random len: %u for maximum %u", len, max));
	//LOG_DEBUG(("random number: 0x%08x, shifted: %u, max: %u", x, x >> len, max));
	x >>= len;
	*/
	x = (x >> bits) % max;
	//LOG_DEBUG(("result: %u of %d, bits: %d", x, max, bits));
	return (int)x;
}

void mrt::init_seed() {
	mrt_rand_seed = time(NULL); //fixme!
}

void mrt::random_serialize(Serializator &s) {
	s.add((unsigned)mrt_rand_seed);
}

void mrt::random_deserialize(const Serializator &s) {
	unsigned x;
	s.get(x);
	mrt_rand_seed = x;
}

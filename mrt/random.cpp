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
#include "random.h"
#include <time.h>
#include "serializator.h"

#ifdef WIN32
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

//#include "logger.h"
#include <assert.h>

static uint32_t mrt_rand_seed;

static uint32_t rand_impl() {
	uint64_t x = mrt_rand_seed;
	x *= 279470273; 
	x %= 4294967291U;
	mrt_rand_seed = (uint32_t)x;
	return x;
}

const unsigned mrt::random(const unsigned max) {
	if (max < 2) 
		return 0;

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
	x %= max;
	//LOG_DEBUG(("result: %u of %d", x, max));
	return x;
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

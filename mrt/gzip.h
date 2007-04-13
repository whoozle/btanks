#ifndef __MRT_GZIP_H__
#define __MRT_GZIP_H__
/* M-Runtime for c++
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

#include <zlib.h>
#include "export.h"

namespace mrt {

class Chunk;

class MRTAPI ZStream {
public:
	static void decompress(mrt::Chunk &dst, const mrt::Chunk &src);
	static void compress(mrt::Chunk &dst, const mrt::Chunk &src, const int level = 3);
private:
};

}


#endif


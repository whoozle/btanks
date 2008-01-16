#ifndef __BTANKS_SERIALIZABLE_H__
#define __BTANKS_SERIALIZABLE_H__

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


#include "export_mrt.h"

namespace mrt {
class Serializator;
class Chunk;

class MRTAPI Serializable {
public:
	virtual void serialize(Serializator &s) const = 0;
	virtual void deserialize(const Serializator &s) = 0;

	void serialize2(mrt::Chunk &s) const;
	void deserialize2(const mrt::Chunk &s);

	virtual ~Serializable();
};
}

#endif


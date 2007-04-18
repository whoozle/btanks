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
#include "serializable.h"
#include "serializator.h"
#include "chunk.h"

using namespace mrt;

void Serializable::serialize2(mrt::Chunk &d) const {
	mrt::Serializator s;
	serialize(s);
	d = s.getData();
}

void Serializable::deserialize2(const mrt::Chunk &d) {
	mrt::Serializator s(&d);
	deserialize(s);
}

Serializable::~Serializable() {}

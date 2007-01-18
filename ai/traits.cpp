#include "traits.h"
#include "mrt/random.h"
#include "mrt/logger.h"
#include <assert.h>

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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


using namespace ai;

const float Traits::get(const std::string &value, const std::string & object, const float hint1, const float hint2) {
	assert(!object.empty());
	const std::string name = value + ":" + object;
	const_iterator i = find(name);
	if (i != end())
		return i->second;
	
	const float v = hint1 + (mrt::random(1000000) / 1000000.0) * hint2;
	LOG_DEBUG(("generate value for %s -> %g", name.c_str(), v));
	return (operator[](name) = v);
}


void Traits::serialize(mrt::Serializator &s) const {
	
}
void Traits::deserialize(const mrt::Serializator &s) {

}

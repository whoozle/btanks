/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "./base.h"
#include "object.h"
#include "alarm.h"

using namespace ai;

#define MAGIC_SIZE 5
#define MAGIC_ROWS 5

const int Base::magic[MAGIC_ROWS][MAGIC_SIZE] = {
	{3, 2, 4, 1, 5, }, 
	{3, 3, 3, 3, 3, }, 
	{4, 2, 3, 4, 2, }, 
	{1, 1, 5, 5, 3, }, 
	{4, 3, 2, 5, 1, }, 
};

Base::Base() : multiplier(1.0f), row(0), pos(0), attempt(0), waiting(false) {}

const bool Base::canFire() {
	if (attempt == 0) {
		pos = (pos + 1) % MAGIC_SIZE;
		attempt = (int)(magic[row][pos] * multiplier);
		waiting = !waiting;
	} else {
		--attempt;
	}
	return !waiting;
}

void Base::on_spawn(Object *src)  {
	int id = src->get_id();
	row = id % MAGIC_ROWS;
	pos = (id * 3 + 7)  % MAGIC_SIZE;
	//LOG_DEBUG(("spawning %s(%d) with %dx%d", src->animation.c_str(), id, row, pos));
	attempt = (int) (magic[row][pos] * multiplier);
	waiting = false;
}

void Base::serialize(mrt::Serializator &s) const {
	s.add(multiplier);
	s.add(row);
	s.add(pos);
	s.add(attempt);
	s.add(waiting);
}

void Base::deserialize(const mrt::Serializator &s) {
	s.get(multiplier);
	s.get(row);
	s.get(pos);
	s.get(attempt);
	s.get(waiting);
}

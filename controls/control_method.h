#ifndef __BTANKS_CONTROL_METHOD_H__
#define __BTANKS_CONTROL_METHOD_H__

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


class PlayerState;
class PlayerSlot;

class ControlMethod {
public:
	virtual void updateState(PlayerSlot &slot, PlayerState &state) = 0;
	virtual void probe() const = 0;
	virtual ~ControlMethod() {}
};

#endif


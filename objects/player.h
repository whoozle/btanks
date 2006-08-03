#ifndef __BT_PLAYER_H__
#define __BT_PLAYER_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include <SDL/SDL.h>
#include <string>
#include "animated_object.h"
#include "v3.h"
#include "player_state.h"
#include "alarm.h"

class Player : public AnimatedObject {
public:
	Player(const bool stateless);
	Player(const std::string &animation, const bool stateless);
	
	virtual Object * clone(const std::string &opt) const;

	virtual void emit(const std::string &event, const Object * emitter);
	virtual void tick(const float dt);

protected:
	void setup(const std::string &animation);

	PlayerState _state;
	bool _stale;
private:
	bool _stateless;
	Alarm _fire;
	std::string _animation;
};

#endif


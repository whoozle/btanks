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

#include "object.h"
#include "resource_manager.h"
#include "math/matrix.h"
#include "tmx/map.h"
#include "mrt/random.h"
#include "config.h"
#include "world.h"

class SandWorm : public Object {
public: 
	SandWorm(): Object("monster"), _reaction_time(true) {
		setDirectionsNumber(1);
	}
	virtual void onSpawn() {
		disown();
		play("main", true);
		GET_CONFIG_VALUE("objects.sandworm.reaction-time", float, rt, 0.1);
		mrt::randomize(rt, rt/10);
		_reaction_time.set(rt);
		
		GET_CONFIG_VALUE("objects.sandworm.initial-length", int, il, 3);
		int i;
		for(i = 0; i < il; ++i) {
			if (_variants.has(mrt::formatString("%d", i))) {
				//LOG_DEBUG(("this is tail #%d", i));
				break;
			}
		}
		//LOG_DEBUG(("spawning tail #%d", i - 1));
		if (i > 0)
			spawn(mrt::formatString("sandworm(%d)", i - 1), "sandworm");
	}
	
	virtual void calculate(const float dt) {
		int sid = getSummoner();
		if (isDriven() || !_reaction_time.tick(dt)) {
			if (sid <= 0) {
				calculateWayVelocity();
				updateStateFromVelocity();
			} //save velocity of tails
			return;
		}

		if (sid > 0) {
			//tail following its predecessor.
			Object *summoner = World->getObjectByID(sid);
			if (summoner == NULL) {
				emit("death", NULL);
				return;
			}
			_velocity = summoner->getRelativePosition(this);
			float l = _velocity.normalize();
			if (l < 64)
				_velocity.clear();
			return;
		}
		
		LOG_DEBUG(("searching random hint area..."));
		const Matrix<int> &hint = Map->getAreaMatrix("sandworm"); //check
		const v2<int> tile_size = Map->getPathTileSize();

		int w = hint.getWidth(), h = hint.getHeight();
		std::set<std::pair<int, int> > coords;
		for(int y = 0; y < h; ++y) 
			for(int x = 0; x < w; ++x) {
				if (hint.get(y, x))
					coords.insert(std::pair<int, int>(x, y));
		}
		
		if (coords.empty()) {
			LOG_ERROR(("no hint area defined... committing suicide"));
			emit("death", NULL);
			return;
		}
		
		int n = mrt::random(coords.size());
		std::set<std::pair<int, int> >::const_iterator i = coords.begin();
		while(n--) ++i;
		
		Way way;
		way.push_back(WayPoint(i->first * tile_size.x + tile_size.x / 2, i->second * tile_size.y + tile_size.y / 2));
		setWay(way);
	}
	
	Object* clone() const  {
		return new SandWorm(*this);
	}
	
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_reaction_time);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_reaction_time);
	}
	
private:
	Alarm _reaction_time; 
};

REGISTER_OBJECT("sandworm", SandWorm, ());

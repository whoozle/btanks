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

#include "alarm.h"
#include "resource_manager.h"
#include "config.h"
#include "object.h"
#include "game.h"
#include "item.h"

#include <set>
#include <map>
#include <queue>
#include "tmx/map.h"
#include "math/unary.h"


static inline const int h(const int src, const int dst, const int pitch) {
	int y1 = src/pitch, y2 = dst/pitch;
	int x1 = src%pitch, x2 = dst%pitch;
	return 100 * (math::abs(x1 - x2) + math::abs<int>(y1 - y2));
}


class Car : public Object {
//** PATHFINDING STUFF
	typedef std::set<int> CloseList;

	struct Point {
		Point() : id(-1), parent(-1), dir(-1) {}
		int id, g, h, parent, dir;

		const bool operator<(const Point &other) const {
			return (g + h) > (other.g + other.h);
			//return g > other.g;
		}
	};

	typedef std::priority_queue<Point> OpenList;
	typedef std::map<const int, Point> PointMap;


public: 
	Car();

	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual Object * clone() const;
	
	void findPath(const v3<int> target, const int step);
	const bool findPathDone(Way &way);

	void emit(const std::string &event, BaseObject * emitter);
	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		_refresh_waypoints.serialize(s);
		_waypoint.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		_refresh_waypoints.deserialize(s);
		_waypoint.deserialize(s);
	}	
private: 
	Alarm _refresh_waypoints;
	v3<int> _waypoint;

	OpenList _open_list;
	PointMap _points;
	CloseList _close_list;
	int _pitch, _end_id, _begin_id, _step;
};


#include "world.h"

void Car::findPath(const v3<int> target, const int step) {
	_step = step;
	v3<int> begin, end = target;
	const v3<int> map_size = Map->getSize();
	_pitch = 1 + (map_size.x - 1) / step;
	
	getPosition(begin);
	begin /= step;
	end /= step;
	
	_end_id = end.x + end.y * _pitch;
	_begin_id = begin.x + begin.y * _pitch;
	
	while(!_open_list.empty())
		_open_list.pop();
	
	_close_list.clear();
	_points.clear();
	
	Point p;
	p.id = _begin_id;
	p.g = 0;
	p.h = h(p.id, _end_id, _pitch);
	p.dir = getDirection();

	_open_list.push(p);
	_points[p.id] = p;

}

const bool Car::findPathDone(Way &way) {
	const v3<int> map_size = Map->getSize();
	int dir_save = getDirection();
	GET_CONFIG_VALUE("engine.pathfinding-slice", int, ps, 3);
	
	while(!_open_list.empty() && ps--) {
		const Point current = _open_list.top();
		_open_list.pop();
		
		if (_close_list.find(current.id) != _close_list.end())
			continue;
/*
		LOG_DEBUG(("%d: popping vertex. id=%d, x=%d, y=%d, g=%d, h=%d, f=%d", getID(), 
			current.id, current.id % _pitch, current.id / _pitch, current.g, current.h, current.g + current.h));
*/		
		_close_list.insert(current.id);
		const int x = (current.id % _pitch) * _step;
		const int y = (current.id / _pitch) * _step;
		
		//searching surrounds 
		const int dirs = getDirectionsNumber();
		if (dirs < 4 || dirs > 8)
			throw_ex(("pathfinding cannot handle directions number: %d", dirs));
		
		for(int i = 0; i < dirs; ++i) {
			v3<float> d;
			d.fromDirection(i, dirs);
			d.x = math::sign(d.x) * _step;
			d.y = math::sign(d.y) * _step;
			
			d.x += x;
			d.y += y;
			
			if (d.x < 0 || d.x > map_size.x || d.y < 0 || d.y > map_size.y)
				continue;
			
			v3<int> pos((int)(d.x / _step), (int)(d.y / _step), 0);
			
			const int id = pos.x + pos.y * _pitch;
			assert( id != current.id );
			//LOG_DEBUG(("testing id %d, x=%d, y=%d", id, pos.x, pos.y));
			
			if (_close_list.find(id) != _close_list.end())
				continue;

			setDirection(i);
			v3<int> world_pos(pos.x * _step, pos.y * _step, 0);
			int map_im = Map->getImpassability(this, world_pos);
			if (map_im >= 100) {
				_close_list.insert(id);
				continue;			
			}
			float im = World->getImpassability(this, world_pos, NULL, true);
			if (im >= 1.0 || im < 0) {
				_close_list.insert(id);
				continue;
			}
			
			Point p;
			p.id = id;
			p.dir = i;
			p.parent = current.id;
			p.g = current.g + (d.x != 0 && d.y != 0)?144:100 + (int)(im * 100) + map_im;
			
			//add penalty for turning
			assert(current.dir != -1);
			int dd = math::abs(i - current.dir);
			if (dd > dirs/2) 
				dd = dirs - dd;
			p.g += 50 * dd;
			
			p.h = h(id, _end_id, _pitch);
			
			_points[p.id] = p;

			if (p.h < 100) {
				_end_id = p.id;
				goto found;
			}

			_open_list.push(p);
		}
	}
	
	setDirection(dir_save);

	if (ps < 0) {
		return false;
	}

	way.clear();
	return true;

found:
	way.clear();
	while(!_open_list.empty())
		_open_list.pop();
	
	_close_list.clear();
	
	setDirection(dir_save);

	for(int id = _end_id; id != _begin_id; ) {
		Point &p = _points[id];
		way.push_front(v3<int>((p.id % _pitch) * _step, (p.id / _pitch) * _step, 0));
		id = p.parent;
		assert(id != -1);
	}
	_points.clear();
	way.erase(way.begin());
	return true;

}


void Car::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		spawn("corpse", "dead-" + animation, v3<float>::empty, v3<float>::empty);
	} else if (event == "collision") {
		if (emitter != NULL) {
			Item * item = dynamic_cast<Item *>(emitter);
			//no items.
			if (item == NULL) {
				GET_CONFIG_VALUE("objects.car.damage", int, d, 5);
				emitter->addDamage(this, d);
				emit("death", emitter);
			}
		}
	}
	Object::emit(event, emitter);
}


void Car::onSpawn() {
	GET_CONFIG_VALUE("objects.car.refreshing-path-interval", float, rpi, 1);
	_refresh_waypoints.set(rpi);
	play("hold", true);
}

Car::Car() : Object("car"), _refresh_waypoints(false) {}

void Car::calculate(const float dt) {	
	if (!isDriven() && _open_list.empty()) {
		LOG_DEBUG(("looking for waypoints..."));
		v3<int> waypoint;
		Game->getRandomWaypoint(_waypoint, "cars");
		LOG_DEBUG(("next waypoint : %d %d", _waypoint.x, _waypoint.y));
		findPath(_waypoint, 16);
		return;
	}

	if (!_open_list.empty()) {
		Way way;
		if (findPathDone(way)) {
			if (!way.empty()) 
				setWay(way);
			else 
				LOG_WARN(("findPath failed. retry later."));
		}
	}
	if (isDriven() && _refresh_waypoints.tick(dt)) {
		_refresh_waypoints.reset();
		findPath(_waypoint, 16);
	}
	
	calculateWayVelocity();
	GET_CONFIG_VALUE("objects.car.rotation-time", float, rt, 0.05);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();
}

void Car::tick(const float dt) {
	Object::tick(dt);
	if (_velocity.is0() && getState() != "hold") {
		cancelAll();
		play("hold", true);
	} else if (!_velocity.is0() && getState() != "move") {
		cancelAll();
		play("move", true);
	}
}


Object * Car::clone() const {
	return new Car(*this);
}


REGISTER_OBJECT("car", Car, ());

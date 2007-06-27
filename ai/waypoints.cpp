#include "waypoints.h"
#include "mrt/serializator.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "game_monitor.h"
#include "player_manager.h"

using namespace ai;

Waypoints::Waypoints() : _avoid_obstacles(false), _stop_on_obstacle(true), _reaction_time(true), _stop(false), _obstacle(0) {}

const bool Waypoints::active() const {
	return !PlayerManager->isClient();
}

void Waypoints::calculate(Object *object, const float dt) {
	if (!active()) {
		if (object->isDriven()) 
			object->calculateWayVelocity();
		object->updateStateFromVelocity();
		return;
	}

	if (_avoid_obstacles && _reaction_time.tick(dt)) {
		std::set<const Object *> objs;
		object->enumerateObjects(objs, (object->size.x + object->size.y) * 2 / 3, NULL /* &obstacle_filter */);
		std::set<const Object *>::const_iterator i;
		for(i = objs.begin(); i != objs.end(); ++i) {
			if ((*i)->speed == 0)
				continue;
			
			v2<float> dpos = object->getRelativePosition(*i);
			dpos.normalize();
			int odir = dpos.getDirection(object->getDirectionsNumber()) - 1;
			//LOG_DEBUG(("%s: (%g %g)dir = %d, my_dir = %d", animation.c_str(), dpos.x, dpos.y, odir, getDirection()));
			if (odir == object->getDirection()) {
				_obstacle = *i;
				object->_velocity.clear();
				break;
			}
		}
		if (i == objs.end())
			_obstacle = NULL;
		
		if (_obstacle) {
			onObstacle(_obstacle);
		}
	}
	
	if (_obstacle && _stop_on_obstacle) {
		object->_velocity.clear();
		return;
	}
	
	if (!object->calculatingPath() && !object->isDriven()) {
		v2<float> waypoint;
		object->_velocity.clear();
		if (_waypoint_name.empty()) {
			_waypoint_name = object->getNearestWaypoint(object->registered_name);
			assert(!_waypoint_name.empty());
			GameMonitor->getWaypoint(waypoint, object->registered_name, _waypoint_name);
			if (waypoint.quick_length() < object->size.x * object->size.y) {
				//LOG_DEBUG(("waypoint is too close..."));
				goto random_wp; //REWRITE THIS UGLY CODE
			}
			//LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", object->animation.c_str(), object->getID(), waypoint.x, waypoint.y));
		} else {
		random_wp:
			//LOG_DEBUG(("%s[%d] reached waypoint '%s'", object->animation.c_str(), object->getID(), _waypoint_name.c_str()));
			_waypoint_name = GameMonitor->getRandomWaypoint(object->registered_name, _waypoint_name);
			GameMonitor->getWaypoint(waypoint, object->registered_name, _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", object->animation.c_str(), object->getID(), _waypoint_name.c_str(), waypoint.x, waypoint.y));
		}
		int pfs;
		Config->get("objects." + object->registered_name + ".pathfinding-step", pfs, 16);
		object->findPath(waypoint.convert<int>(), pfs);
	}
	Way way;
	if (object->calculatingPath() && object->findPathDone(way)) {
		if (way.size() == 1) { 
			object->_velocity.clear();
			return;
		} else if (way.empty()) {
			_waypoint_name.clear();
			LOG_DEBUG(("%s:%s[%d] no path[%u]. ", 
				object->registered_name.c_str(), object->animation.c_str(), object->getID(), (unsigned)way.size()));
			//emit("death", NULL);
		}
		object->setWay(way);
	} else object->_velocity.clear();

	object->calculateWayVelocity();	
}


void Waypoints::onSpawn(const Object *object) {
	float rt;
	Config->get("objects." + object->registered_name + ".reaction-time", rt, 0.1f);
	
	_reaction_time.set(rt);
	mrt::randomize(rt, rt / 10);

	_stop = false;
	_obstacle = 0;
}


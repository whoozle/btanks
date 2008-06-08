#include "waypoints.h"
#include "mrt/serializator.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "game_monitor.h"
#include "player_manager.h"

using namespace ai;

void Waypoints::serialize(mrt::Serializator &s) const {
	ai::OldSchool::serialize(s);
	s.add(_no_waypoints);
	s.add(_avoid_obstacles);
	s.add(_stop_on_obstacle);
	s.add(_reaction_time);
	s.add(_stop);
	s.add(_waypoint_name);
}
void Waypoints::deserialize(const mrt::Serializator &s) {
	ai::OldSchool::deserialize(s);
	s.get(_no_waypoints);
	s.get(_avoid_obstacles);
	s.get(_stop_on_obstacle);
	s.get(_reaction_time);
	s.get(_stop);
	s.get(_waypoint_name);
}

Waypoints::Waypoints() : _avoid_obstacles(false), _stop_on_obstacle(true), _reaction_time(true), _stop(false)  {}

void Waypoints::calculate(Object *object, const float dt) {
	if (_no_waypoints) {
		//LOG_DEBUG(("no waypoints stub!"));
		if (_reaction_time.tick(dt))
			ai::OldSchool::calculateV(object->_velocity, object);
		object->calculate_way_velocity();
		return;
	}
	
	if (_avoid_obstacles && _reaction_time.tick(dt)) {
		const Object * obstacle = NULL;
		
		std::set<const Object *> objs;
		object->enumerate_objects(objs, (object->size.x + object->size.y) * 2 / 3, NULL /* &obstacle_filter */);
		std::set<const Object *>::const_iterator i;
		for(i = objs.begin(); i != objs.end(); ++i) {
			if ((*i)->speed == 0 || (*i)->impassability == 0)
				continue;
			
			v2<float> dpos = object->get_relative_position(*i);
			dpos.normalize();
			int odir = dpos.get_direction(object->get_directions_number()) - 1;
			//LOG_DEBUG(("%s: (%g %g)dir = %d, my_dir = %d", animation.c_str(), dpos.x, dpos.y, odir, get_direction()));
			if (odir == object->get_direction()) {
				obstacle = *i;
				object->_velocity.clear();
				break;
			}
		}
		
		if (obstacle) {
			onObstacle(obstacle);
			_stop = true;
		} else 
			_stop = false;
	}
	
	if (_stop && _stop_on_obstacle) {
		object->_velocity.clear();
		return;
	}

	if (!object->calculating_path() && !object->is_driven()) {
		v2<float> waypoint;
		object->_velocity.clear();
		if (_waypoint_name.empty()) {
			_waypoint_name = object->get_nearest_waypoint(object->registered_name);
			assert(!_waypoint_name.empty());
			GameMonitor->get_waypoint(waypoint, object->registered_name, _waypoint_name);
			if (waypoint.quick_length() < object->size.x * object->size.y) {
				//LOG_DEBUG(("waypoint is too close..."));
				goto random_wp; //REWRITE THIS UGLY CODE
			}
			//LOG_DEBUG(("%s[%d] moving to nearest waypoint at %g %g", object->animation.c_str(), object->get_id(), waypoint.x, waypoint.y));
		} else {
		random_wp:
			//LOG_DEBUG(("%s[%d] reached waypoint '%s'", object->animation.c_str(), object->get_id(), _waypoint_name.c_str()));
			_waypoint_name = GameMonitor->getRandomWaypoint(object->registered_name, _waypoint_name);
			GameMonitor->get_waypoint(waypoint, object->registered_name, _waypoint_name);
			//LOG_DEBUG(("%s[%d] moving to next waypoint '%s' at %g %g", object->animation.c_str(), object->get_id(), _waypoint_name.c_str(), waypoint.x, waypoint.y));
		}
		int pfs;
		Config->get("objects." + object->registered_name + ".pathfinding-step", pfs, 16);
		object->find_path(waypoint.convert<int>(), pfs);
	}
	Way way;
	if (object->calculating_path() && object->find_path_done(way)) {
		if (way.size() == 1) { 
			object->_velocity.clear();
			return;
		} else if (way.empty()) {
			_waypoint_name.clear();
			LOG_DEBUG(("%s:%s[%d] no path[%u]. ", 
				object->registered_name.c_str(), object->animation.c_str(), object->get_id(), (unsigned)way.size()));
			//emit("death", NULL);
		}
		object->set_way(way);
	} else object->_velocity.clear();

	object->calculate_way_velocity();	
}


void Waypoints::on_spawn(const Object *object) {
	float rt;
	Config->get("objects." + object->registered_name + ".reaction-time", rt, 0.3f);
	if (rt <= 0.3f) {
		rt = 0.3f;
		Config->set("objects." + object->registered_name + ".reaction-time", rt);
	}
	
	mrt::randomize(rt, rt / 10);
	_reaction_time.set(rt);

	_stop = false;
	_no_waypoints = !GameMonitor->hasWaypoints(object->registered_name);
	if (_no_waypoints) 
		ai::OldSchool::on_spawn(object);
}

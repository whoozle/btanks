#ifndef BTANKS_AI_WAYPOINTS_H__
#define BTANKS_AI_WAYPOINTS_H__

#include <string>
//#include <set>
#include "alarm.h"
#include "mrt/serializable.h"
#include "export_btanks.h"

class Object;

namespace ai {

class BTANKSAPI Waypoints {
public: 
	Waypoints();
	virtual void onSpawn(const Object *object);
	void calculate(Object *object, const float dt);

	virtual void onObstacle(const Object *o) = 0;

	virtual ~Waypoints() {}
	
	const bool active() const;
protected:
	//std::set<std::string> obstacle_filter;
	bool _avoid_obstacles;
	bool _stop_on_obstacle;
	
private: 
	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
	const Object * _obstacle;
};

}

#endif


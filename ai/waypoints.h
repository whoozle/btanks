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

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	virtual void onObstacle(const int idx) {}

	virtual ~Waypoints() {}
	
	const bool active() const;
protected:
	//std::set<std::string> obstacle_filter;
	bool _avoid_obstacles;
	
private: 
	Alarm _reaction_time;
	bool _stop;
	std::string _waypoint_name;
	int _obstacle;	
};

}

#endif


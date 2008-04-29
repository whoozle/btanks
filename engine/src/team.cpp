#include "team.h"
#include "object.h"
#include "special_owners.h"

int Team::get_owner(const Team::ID team) {
	switch(team) {
		case Red:
			return OWNER_TEAM_RED;
		case Green: 
			return OWNER_TEAM_GREEN;
		case Blue: 
			return OWNER_TEAM_BLUE;
		case Yellow: 
			return OWNER_TEAM_YELLOW;
		default: 
			throw_ex(("no owner for team %d", (int)team));
	}
}


const char * Team::get_color(ID t) {
	switch(t) {
		case Red:
			return "red";
		case Green: 
			return "green";
		case Blue: 
			return "blue";
		case Yellow: 
			return "yellow";
		default: 
			return "unknown";
	}
}

Team::ID Team::get_team(const Object *o) {
	size_t l = o->animation.size();
	if (o->animation.compare(l - 4, 4, "-red") == 0 || o->animation.compare(0, 4, "red-") == 0) {
		return Red;
	} else if (o->animation.compare(l - 6, 6, "-green") == 0 || o->animation.compare(0, 6, "green-") == 0) {
		return Green;
	} else if (o->animation.compare(l - 5, 5, "-blue") == 0 || o->animation.compare(0, 5, "blue-") == 0) {
		return Blue;
	} else if (o->animation.compare(l - 7, 7, "-yellow") == 0 || o->animation.compare(0, 7, "yellow-") == 0) {
		return Yellow;
	} 
	return None;
}

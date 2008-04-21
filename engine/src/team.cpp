#include "team.h"
#include "object.h"

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
	if (o->animation.compare(0, 5, "flag-") != 0 && o->animation.compare(0, 8, "ctf-base") != 0)
		return None;

	size_t l = o->animation.size();
	if (o->animation.compare(l - 4, 4, "-red") == 0) {
		return Red;
	} else if (o->animation.compare(l - 6, 6, "-green") == 0) {
		return Green;
	} else if (o->animation.compare(l - 5, 5, "-blue") == 0) {
		return Blue;
	} else if (o->animation.compare(l - 7, 7, "-yellow") == 0) {
		return Yellow;
	} 
	return None;
}

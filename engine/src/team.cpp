#include "team.h"
#include "object.h"

const char * Team::get_color(ID t) {
	switch(t) {
		case TeamRed:
			return "red";
		case TeamGreen: 
			return "green";
		case TeamBlue: 
			return "blue";
		case TeamYellow: 
			return "yellow";
		default: 
			return "unknown";
	}
}

Team::ID Team::get_team(const Object *o) {
	if (o->animation.compare(0, 5, "flag-") != 0 && o->animation.compare(0, 8, "ctf-base") != 0)
		return TeamNone;

	size_t l = o->animation.size();
	if (o->animation.compare(l - 4, 4, "-red") == 0) {
		return TeamRed;
	} else if (o->animation.compare(l - 6, 6, "-green") == 0) {
		return TeamGreen;
	} else if (o->animation.compare(l - 5, 5, "-blue") == 0) {
		return TeamBlue;
	} else if (o->animation.compare(l - 7, 7, "-yellow") == 0) {
		return TeamYellow;
	} 
	return TeamNone;
}

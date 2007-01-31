#include "herd.h"
#include <set>
#include "world.h"
#include "object.h"
#include "config.h"

void ai::Herd::calculateV(v3<float> &velocity, Object *sheep, const int leader, const float distance) {
	if (leader == 0) 
		throw_ex(("cannot operate on objects without summoner."));
	std::set<const Object *> o_set;
	World->enumerateObjects(o_set, sheep, distance);
	int n = 0;
	for(std::set<const Object *>::iterator i = o_set.begin(); i != o_set.end(); ++i) {
		const Object *o = *i;
		if (o->getSummoner() != leader) 
			continue;
		int cd = getComfortDistance(o);
		if (cd == -1)
			continue;
			
		v3<float> pos = sheep->getRelativePosition(o);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos;
		
		++n;
	}
	const Object * o = World->getObjectByID(leader);
	if (o != NULL) {
		v3<float> pos = sheep->getRelativePosition(o);
		int cd = getComfortDistance(NULL);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos;
	}
	if (velocity.quick_length() < 100)
		velocity.clear();
}


#include "herd.h"
#include <set>
#include "world.h"
#include "object.h"
#include "config.h"

void ai::Herd::calculateV(v2<float> &velocity, Object *sheep, const int leader, const float distance) {
	velocity.clear();
	
	std::set<const Object *> o_set;
	World->enumerateObjects(o_set, sheep, distance, NULL);
	int n = 0;
	for(std::set<const Object *>::iterator i = o_set.begin(); i != o_set.end(); ++i) {
		const Object *o = *i;
		if (leader && o->getSummoner() != leader) 
			continue;
		int cd = getComfortDistance(o);
		if (cd == -1)
			continue;
			
		v2<float> pos = sheep->getRelativePosition(o);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos;
		
		++n;
	}
		
	const Object * o = leader?World->getObjectByID(leader): NULL;
	if (o != NULL) {
		//LOG_DEBUG(("leader: %p", o));
		v2<float> pos = sheep->getRelativePosition(o);
		int cd = getComfortDistance(NULL);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos * n;
	}
	if (velocity.normalize() < 100)
		velocity.clear();
}


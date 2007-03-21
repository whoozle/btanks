#include "herd.h"
#include <set>
#include "world.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "zbox.h"

void ai::Herd::calculateV(v2<float> &velocity, Object *sheep, const int leader, const float distance) {
	bool was_stopped = velocity.is0();
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
		float r = pos.length();
		if (r < 0.001) r = 0.001;
		if (pos.quick_length() < cd * cd)
			velocity -= pos / r;
		else 
			velocity += pos / r;
		
		++n;
	}
		
	const Object * o = leader?World->getObjectByID(leader): NULL;
	if (o != NULL && !ZBox::sameBox(o->getZ(), sheep->getZ())) 
		o = NULL;
	
	if (o != NULL) {
		//LOG_DEBUG(("leader: %p", o));
		v2<float> pos = sheep->getRelativePosition(o);
		int cd = getComfortDistance(NULL);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos * n;
	}
	float v = velocity.normalize();

	if (v < (was_stopped?0.5:0.0001)) 
		velocity.clear();
}


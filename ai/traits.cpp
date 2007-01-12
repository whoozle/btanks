#include "traits.h"
#include "mrt/random.h"
#include "mrt/logger.h"

using namespace ai;

const float Traits::get(const std::string &value, const char object, const float hint1, const float hint2) {
	const std::string name = value + "." + object;
	const_iterator i = find(name);
	if (i != end())
		return i->second;
	
	const float v = hint1 + mrt::random((int)hint2);
	LOG_DEBUG(("generate value for %s -> %g", name.c_str(), v));
	return (operator[](name) = v);
}


void Traits::serialize(mrt::Serializator &s) const {
	
}
void Traits::deserialize(const mrt::Serializator &s) {

}

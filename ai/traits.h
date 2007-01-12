#ifndef AI_TRAITS_H__
#define AI_TRAITS_H__

#include "mrt/serializable.h"

#include <map>
#include <string>

namespace ai {
class Traits : public mrt::Serializable, public std::map<const std::string, float> {
public: 
	const float get(const std::string &value, const char object, const float hint1, const float hint2);
	
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
};
}

#endif


#ifndef __BTANKS_VAR_H__
#define __BTANKS_VAR_H__

#include "mrt/serializable.h"
#include <string>

class Var : public mrt::Serializable {
public: 
	std::string type;
	Var() {}
	Var(const std::string & type): type(type) {}
	
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	void check(const std::string &t) const;
	
	const std::string toString() const;
	void fromString(const std::string &str);
	
	int i;
	bool b;
	float f;
	std::string s;
};

#endif

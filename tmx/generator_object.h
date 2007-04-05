#ifndef BTANKS_GENERATOR_OBJECT_H__
#define BTANKS_GENERATOR_OBJECT_H__

#include <string>
#include <map>

class GeneratorObject {
public: 
	int w, h;

	virtual void init(const std::map<const std::string, std::string>& attrs, const std::string &data);
	virtual ~GeneratorObject() {}
	
	static GeneratorObject *create(const std::string &name, const std::map<const std::string, std::string>& attrs, const std::string &data);
protected: 
	static std::string get(const std::map<const std::string, std::string>& attrs, const std::string &name);
private: 
	static GeneratorObject *create(const std::string &name);
};

#endif

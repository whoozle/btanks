#ifndef BTANKS_GENERATOR_OBJECT_H__
#define BTANKS_GENERATOR_OBJECT_H__

#include <string>
#include <map>

class Layer;

class GeneratorObject {
public: 
	int w, h;

	virtual void init(const std::map<const std::string, std::string>& attrs, const std::string &data);
	virtual void render(Layer *layer, const int first_gid, const int x, const int y) const = 0;
	virtual ~GeneratorObject() {}
	
	static GeneratorObject *create(const std::string &name, const std::map<const std::string, std::string>& attrs, const std::string &data);
protected: 
	static std::string get(const std::map<const std::string, std::string>& attrs, const std::string &name);
private: 
	static GeneratorObject *create(const std::string &name);
};

#endif

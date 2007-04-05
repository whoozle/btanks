#include "generator_object.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include <stdlib.h>
#include <vector>


class Background : public GeneratorObject {
public: 
	void init(const std::map<const std::string, std::string>& attrs, const std::string &data) {
		GeneratorObject::init(attrs, data);
		tiles.clear();
		
		std::vector<std::string> ts;
		mrt::split(ts, data, ",");
		for(size_t i = 0; i < ts.size(); ++i) {
			mrt::trim(ts[i]);
			tiles.push_back(atoi(ts[i].c_str()));
		}
	}
	std::vector<int> tiles;
};

void GeneratorObject::init(const std::map<const std::string, std::string>& attrs, const std::string &data)  {
	int size = atoi(get(attrs, "size").c_str());
	if (size > 0) 
		w = h = size;
	
}

GeneratorObject *GeneratorObject::create(const std::string &name, const std::map<const std::string, std::string>& attrs, const std::string &data) {
	GeneratorObject *o = create(name);
	o->init(attrs, data);
	return o;
}


GeneratorObject *GeneratorObject::create(const std::string &name) {
	if (name == "background") {
		//create background
		return new Background;
	} else throw_ex(("cannot handle '%s' object", name.c_str()));
}

std::string GeneratorObject::get(const std::map<const std::string, std::string>& attrs, const std::string &name)  {
	static std::string empty;
	const std::map<const std::string, std::string>::const_iterator i = attrs.find(name);
	if (i == attrs.end())
		return empty;
	return i->second;
}

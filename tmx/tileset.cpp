#include "tileset.h"
#include "generator_object.h"
#include "mrt/random.h"

void Tileset::start(const std::string &name, Attrs &attr) {
	if (name == "tileset")
		return;
	
	_cdata.clear();
	_attr = attr;
	if (attr["id"].empty())
		throw_ex(("empty id for element %s", name.c_str()));
}

void Tileset::charData(const std::string &data) {
	_cdata += data;
}

void Tileset::end(const std::string &name) {
	if (name == "tileset")
		return;

	GeneratorObject *o = GeneratorObject::create(name, _attr, _cdata);
	_objects.insert(Objects::value_type(_attr["id"], o));
}

const GeneratorObject *Tileset::getObject(const std::string &name) const {
	if (name == "?") {
		if (_objects.empty())
			return NULL;
		
		int n = mrt::random(_objects.size());
		Objects::const_iterator i = _objects.begin();
		while(n--) {
			++i;
		}
		return i->second;
	}
	
	Objects::const_iterator i = _objects.find(name);
	if (i == _objects.end());
		return NULL;
	
	return i->second;
}

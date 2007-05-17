#include <assert.h>
#include <algorithm>
#include "tileset.h"
#include "generator_object.h"
#include "mrt/random.h"
#include "utils.h"

void Tileset::start(const std::string &name, Attrs &attr) {
	if (name == "tileset")
		return;
	
	_cdata.clear();
	_attr = attr;
	if (attr["id"].empty()) {
		if (name == "background")
			throw_ex(("empty id for element %s", name.c_str()));
	}
}

void Tileset::charData(const std::string &data) {
	_cdata += data;
}

void Tileset::end(const std::string &name) {
	if (name == "tileset")
		return;
		
	if (_objects.find(name) != _objects.end())
		throw_ex(("duplicate id %s", name.c_str()));

	std::string id =  _attr["id"];
	GeneratorObject *o = GeneratorObject::create(name, _attr, _cdata);
	LOG_DEBUG(("adding '%s' object with id '%s' (%p)", name.c_str(), id.c_str(), (void *)o));
	_objects.insert(Objects::value_type(id, o));
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
	if (i == _objects.end())
		return NULL;

	assert(i->second != NULL);
	
	return i->second;
}

Tileset::~Tileset() {
	std::for_each(_objects.begin(), _objects.end(), delete_ptr2<Objects::value_type>());
}

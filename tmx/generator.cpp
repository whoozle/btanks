#include <algorithm>
#include <assert.h>

#include "generator.h"
#include "layer.h"
#include "tileset.h"
#include "generator_object.h"

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/fs_node.h"
#include "mrt/xml.h"

MapGenerator::MapGenerator() {}

void MapGenerator::fill(Layer *layer, const std::vector<std::string> &args) {
	if (args.size() < 2) 
		throw_ex(("fill command takes 2 arguments."));
	//LOG_DEBUG(("type: %s, name: %s",args[0].c_str(), args[1].c_str()));
	const GeneratorObject *obj = getObject(args[0], args[1]);
	int w = layer->getWidth(), h = layer->getHeight();
	for(int y = 0; y < h; y += obj->h) 
		for(int x = 0; x < w; x += obj->w) {
			obj->render(layer, first_gid[args[0]], x, y);	
	}
}

const GeneratorObject* MapGenerator::getObject(const std::string &tileset, const std::string &name) const {
	Tilesets::const_iterator i = _tilesets.find(tileset);
	if (i == _tilesets.end())
		throw_ex(("no tileset %s found", tileset.c_str()));
	assert(i->second != NULL);
	const GeneratorObject *o = i->second->getObject(name);
	if (o == NULL)
		throw_ex(("no object '%s' found in tileset '%s'", name.c_str(), tileset.c_str()));
	return o;
}

void MapGenerator::tileset(const std::string &fname, const int gid) {
	std::string name = getName(fname);
	std::string xml_name = getDescName(fname);
	LOG_DEBUG(("tileset: %s, gid: %d, description file: %s", name.c_str(), gid, xml_name.c_str()));
	first_gid[name] = gid;
	
	if (_tilesets.find(name) != _tilesets.end())
		return;
	
	if (!mrt::FSNode::exists(xml_name))
		return;
	
	Tileset *t = NULL;
	TRY {
		t = new Tileset;
		t->parseFile(xml_name);
		_tilesets.insert(Tilesets::value_type(name, t));
		t = NULL;
	} CATCH("parsing tileset descriptor", {delete t; throw;} );
}


void MapGenerator::exec(Layer *layer, const std::string &command, const std::string &value) {
	LOG_DEBUG(("executing command '%s'...", command.c_str()));
	std::vector<std::string> args;
	mrt::split(args, value, ":");
	
	if (command == "fill") 
		fill(layer, args);
	else throw_ex(("unknown command '%s'", command.c_str()));
}

void MapGenerator::clear() {
	first_gid.clear();
	//std::for_each(_objects.begin(), _objects.end(), delete_ptr2<ObjectMap::value_type>());
}

const std::string MapGenerator::getName(const std::string &fname) {
	size_t end = fname.rfind(".");
	if (end == fname.npos) 
		end = fname.size();
	
	size_t start = fname.rfind("/");
	start = (start == fname.npos) ? 0: start + 1;
	return fname.substr(start, end - start);
}

const std::string MapGenerator::getDescName(const std::string &fname) {
	size_t end = fname.rfind(".");
	if (end == fname.npos) 
		throw_ex(("invalid filename '%s' for tileset", fname.c_str()));
	
	return fname.substr(0, end) + ".xml";
}

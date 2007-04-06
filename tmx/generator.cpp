#include <algorithm>
#include <assert.h>

#include "generator.h"
#include "layer.h"
#include "tileset.h"
#include "generator_object.h"
#include "utils.h"

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/fs_node.h"
#include "mrt/xml.h"

void MapGenerator::exec(Layer *layer, const std::string &command, const std::string &value) {
	assert(layer != NULL);
	_layer = layer;
	LOG_DEBUG(("executing command '%s'...", command.c_str()));
	std::vector<std::string> args;
	mrt::split(args, value, ":");
	
	if (command == "fill") 
		fill(layer, args);
	else if (command == "fill-pattern") 
		fillPattern(layer, args);
	else if (command == "push-matrix") 
		pushMatrix(layer, args);
	else if (command == "pop-matrix") 
		popMatrix(layer, args);
	else if (command == "exclude") 
		exclude(layer, args);
	else throw_ex(("unknown command '%s'", command.c_str()));
	_layer = NULL;
}



MapGenerator::MapGenerator() : _layer(NULL) {}

void MapGenerator::fill(Layer *layer, const std::vector<std::string> &args) {
	if (args.size() < 2) 
		throw_ex(("fill command takes 2 arguments."));
	//LOG_DEBUG(("type: %s, name: %s",args[0].c_str(), args[1].c_str()));
	const GeneratorObject *obj = getObject(args[0], args[1]);
	const int gid = first_gid[args[0]];
	if (gid == 0) 
		throw_ex(("unknown layer %s", args[0].c_str()));
	int w = layer->getWidth(), h = layer->getHeight();
	for(int y = 0; y < h; y += obj->h) 
		for(int x = 0; x < w; x += obj->w) {
			obj->render(this, gid, x, y);	
	}
}

void MapGenerator::fillPattern(Layer *layer, const std::vector<std::string> &args) {
	if (args.size() < 4) 
		throw_ex(("fill-pattern command takes 4 arguments."));

	const int gid = first_gid[args[0]];
	if (gid == 0) 
		throw_ex(("unknown layer %s", args[0].c_str()));

	std::vector<std::string> sizes;
	mrt::split(sizes, args[2], "x");
	if (sizes.size() < 2)
		throw_ex(("size string must have form XxY, e.g. '2x3'"));

	int px = atoi(sizes[0].c_str());
	int py = atoi(sizes[1].c_str());
	if (px <= 0 || py <= 0) 
		throw_ex(("invalid size: %dx%d", px, py));

	const std::string &pattern = args[3];
	if ((int)pattern.size() != px * py) 
		throw_ex(("pattern size must be exact %d chars", px * py));

	const GeneratorObject *obj = getObject(args[0], args[1]);
	int w = layer->getWidth(), h = layer->getHeight();

	for(int y = 0; y < h; y += obj->h) 
		for(int x = 0; x < w; x += obj->w) {
			int pid = (x / obj->w) % px + px * ((y / obj->h) % py);
			if (pattern[pid] != '0' && pattern[pid] != ' ')
				obj->render(this, gid, x, y);
	}
}

void MapGenerator::pushMatrix(Layer *layer, const std::vector<std::string> &args) {
	IntMatrix m;
	m.setSize(layer->getHeight(), layer->getWidth(), 0);
	m.useDefault(0);
	_matrix_stack.push(m);
}

void MapGenerator::popMatrix(Layer *layer, const std::vector<std::string> &args) {
	_matrix_stack.pop();
}

void MapGenerator::exclude(Layer *layer, const std::vector<std::string> &args) {
	if (args.size() < 1) 
		throw_ex(("exclude command takes 1 arguments."));
	
	if (_matrix_stack.empty())
		throw_ex(("exclude cannot operate on empty matrix stack"));
	
	v2<int> pos;
	pos.fromString(args[0]);
	if (pos.x < 0) 
		pos.x += layer->getWidth();

	if (pos.y < 0) 
		pos.y += layer->getHeight();
	
	_matrix_stack.top().set(pos.y, pos.x, 1);
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


const Uint32 MapGenerator::get(const int x, const int y) const {
	if (_layer == NULL)
		throw_ex(("no layer to operate. (malicious external code?)"));
	Uint32 t = _layer->get(x, y);
	if (t != 0)
		return t;
	if (_matrix_stack.empty())
		return 0;
	
	return _matrix_stack.top().get(y, x);
}

void MapGenerator::set(const int x, const int y, const Uint32 tid) {
	if (_layer == NULL)
		throw_ex(("no layer to operate. (malicious external code?)"));
	_layer->set(x, y, tid);
	if (tid != 0 && !_matrix_stack.empty()) 
		_matrix_stack.top().set(y, x, tid);
}


void MapGenerator::clear() {
	first_gid.clear();
}

MapGenerator::~MapGenerator() {
	std::for_each(_tilesets.begin(), _tilesets.end(), delete_ptr2<Tilesets::value_type>());
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

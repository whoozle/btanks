#include <stdlib.h>
#include <vector>

#include "generator_object.h"
#include "mrt/logger.h"
#include "mrt/exception.h"

#include "layer.h"

GeneratorObject::GeneratorObject() : w(0), h(0) {}

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
			//LOG_DEBUG(("tiles[%u] = %d", (unsigned)i, tiles[i]));
		}
		if (tiles.size() != (unsigned)w * h)
			throw_ex(("you must provide exact %d tile ids (%u provided)", w * h, (unsigned)tiles.size()))
	}
	std::vector<int> tiles;

	void render(Layer *layer, const int first_gid, const int x, const int y) const {
		//LOG_DEBUG(("render(%d, %d, %d)", first_gid, x, y));
		for(int dy = 0; dy < h; ++dy) 
			for(int dx = 0; dx < w; ++dx) {
				if (layer->get(x + dx, y + dy) == 0)
					layer->set(x + dx, y + dy, first_gid + tiles[dy * w + dx]);
			}
	}
};

void GeneratorObject::init(const std::map<const std::string, std::string>& attrs, const std::string &data)  {
	int size = atoi(get(attrs, "size").c_str());
	if (size > 0) {
		w = h = size;
		return;
	}

	int w = atoi(get(attrs, "width").c_str());
	if (w > 0) 
		this->w = w;
	int h = atoi(get(attrs, "height").c_str());
	if (h > 0) 
		this->h = h;
	if (w == 0 || h == 0) 
		throw_ex(("you must specify size or width+height of every object"));
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

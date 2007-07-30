#include <stdlib.h>
#include <vector>

#include "generator_object.h"
#include "mrt/logger.h"
#include "mrt/exception.h"

#include "generator.h"

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

	void render(MapGenerator *gen, const int first_gid, const int x, const int y, const bool full) const {
		//LOG_DEBUG(("render(%d, %d, %d)", first_gid, x, y));
		if (full) {
			for(int dy = 0; dy < h; ++dy) 
				for(int dx = 0; dx < w; ++dx) {
					const int tid = tiles[dy * w + dx];
					if (tid != 0 && gen->get(x + dx, y + dy) == 0)
						gen->set(x + dx, y + dy, first_gid + tid);
				}
		} else {
			int px = x % w, py = y % h;
			int tid = tiles[py * w + px];
			if (tid != 0 && gen->get(x, y) == 0)
				gen->set(x, y, first_gid + tid);
		}
	}

};

class TileBox : public GeneratorObject {
	int split_w[3];
	int split_h[3];
	void init(const std::map<const std::string, std::string>& _attrs, const std::string &data) {
		std::map<const std::string, std::string> attrs = _attrs;
		memset(split_w, 0, sizeof(split_w));
		memset(split_h, 0, sizeof(split_h));
		if (sscanf(attrs["width"].c_str(), "%d,%d,%d", split_w, split_w + 1, split_w + 2) != 3)
			throw_ex(("invalid box(in: %s, out: %s) description: width is missing or invalid", attrs["in"].c_str(), attrs["out"].c_str()));
		if (sscanf(attrs["height"].c_str(), "%d,%d,%d", split_h, split_h + 1, split_h + 2) != 3)
			throw_ex(("invalid box(in: %s, out: %s) description: height is missing or invalid", attrs["in"].c_str(), attrs["out"].c_str()));
		std::vector<std::string> numbers; 
		mrt::split(numbers, data, ",");
		
		this->w = 1;
		this->h = 1;
		
		int w = split_w[0] + split_w[1] + split_w[2];
		int h = split_h[0] + split_h[1] + split_h[2];
		
		if (numbers.size() != (unsigned)w * h)
			throw_ex(("invalid box(in: %s, out: %s) description: got %u numbers, need: %d", attrs["in"].c_str(), attrs["out"].c_str(), (unsigned) numbers.size(), w * h));
		
		LOG_DEBUG(("box(%dx%d)[%d,%d,%d:%d,%d,%d]", w, h, split_w[0], split_w[1], split_w[2], split_h[0], split_h[1], split_h[1]));
	}
	void render(MapGenerator *gen, const int first_gid, const int x, const int y, const bool full) const {
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
	} else if (name == "box") {
		return new TileBox;
	} else throw_ex(("cannot handle '%s' object", name.c_str()));
}

std::string GeneratorObject::get(const std::map<const std::string, std::string>& attrs, const std::string &name)  {
	static std::string empty;
	const std::map<const std::string, std::string>::const_iterator i = attrs.find(name);
	if (i == attrs.end())
		return empty;
	return i->second;
}

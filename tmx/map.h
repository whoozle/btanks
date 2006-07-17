#ifndef __BT_MAP_H__
#define __BT_MAP_H__

#include <map>
#include <string>
#include <stack>
#include "mrt/xml.h"
#include "mrt/chunk.h"

namespace sdlx {
class Surface;
class Rect;
}

class TMXEntity;
class Layer;

class Map : protected mrt::XMLParser {
public:
	~Map();
	void clear();
	void load(const std::string &name);
	const bool loaded() const;
	
	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);
	
	void render(sdlx::Surface &window, const sdlx::Rect &dst);

private:
	long _w, _h, _tw, _th, _firstgid, _lastz;
	mrt::Chunk _data;
	sdlx::Surface *_image;
	bool _image_is_tileset;

	typedef std::map<const std::string, std::string> PropertyMap;
	PropertyMap _properties;
	
	typedef std::map<const long, Layer *> LayerMap;
	LayerMap _layers;

	typedef std::map<const long, sdlx::Surface *> TileMap;
	TileMap _tiles;

	struct Entity {
		mrt::XMLParser::Attrs attrs;
		std::string data;
		Entity(const mrt::XMLParser::Attrs & attrs) : attrs(attrs) {}
	};
	
	typedef std::stack<Entity> EntityStack;
	EntityStack _stack;
};

#endif


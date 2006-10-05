#include "map.h"
#include "layer.h"
#include "base_object.h"

#include "mrt/file.h"
#include "mrt/b64.h"
#include "mrt/gzip.h"
#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/surface.h"
#include "object.h"

#include <assert.h>
#include <limits>

#include "SDL_collide/SDL_collide.h"

IMPLEMENT_SINGLETON(Map, IMap)

const int IMap::pathfinding_step = 64;

const bool IMap::collides(const sdlx::Surface &surf, const int dx, const int dy, const sdlx::Surface *tile) const {
	if (tile == NULL)
		return false;
	
	//LOG_DEBUG(("dx: %d, dy: %d, w: %d, h: %d, tid: %d", dx, dy, surf.getWidth(), surf.getHeight(), tid));
	int r = SDL_CollidePixel(surf.getSDLSurface(), dx, dy, tile->getSDLSurface(), 0, 0);
	//LOG_DEBUG(("r = %d", r));
	return r != 0;
}


const int IMap::getImpassability(const Object *obj, const sdlx::Surface &s, const v3<int>&pos, v3<int> *tile_pos) const {
	if (obj->impassability <= 0) {
		return 0;
	}
	
	int w = s.getWidth(), h = s.getHeight();
	int x, x1;
	int y, y1;
	x = x1 = pos.x;
	y = y1 = pos.y;
	
	int x2 = x1 + w; int y2 = y1 + h;
	
	int xt1 = x1 / _tw; int xt2 = x2 / _tw;
	int yt1 = y1 / _th; int yt2 = y2 / _th; 
	int dx1 = x - xt1 * _tw; int dx2 = x - xt2 * _tw;
	int dy1 = y - yt1 * _th; int dy2 = y - yt2 * _th;

	int im = 101;
	//LOG_DEBUG(("%d:%d:%d:%d --> %d:%d %d:%d", x1, y1, w, h, xt1, yt1, xt2, yt2));
	for(LayerMap::const_reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
		const Layer *layer = l->second;
		if (layer->pierceable && obj->piercing) 
			continue;
		
		int layer_im = layer->impassability;
		if (layer_im == -1) 
			continue;
		//LOG_DEBUG(("im: %d, tile: %d", layer_im, layer->get(xt1, yt1)));
		if (collides(s, dx1, dy1, layer->getSurface(xt1, yt1)) && im > layer_im) {
			if (tile_pos) {
				tile_pos->x = xt1;
				tile_pos->y = yt1;
			}
			im = layer_im;
		}

		if (yt2 != yt1 && collides(s, dx1, dy2, layer->getSurface(xt1, yt2)) && im > layer_im) {
			if (tile_pos) {
				tile_pos->x = xt1;
				tile_pos->y = yt2;
			}
			im = layer_im;
		}
		if (xt2 != xt1) {
			if (collides(s, dx2, dy1, layer->getSurface(xt2, yt1)) && im > layer_im) {
				im = layer_im;
				if (tile_pos) {
					tile_pos->x = xt2;
					tile_pos->y = yt1;
				}
			}
			if (yt2 != yt1 && collides(s, dx2, dy2, layer->getSurface(xt2, yt2)) && im > layer_im) {
				if (tile_pos) {
					tile_pos->x = xt2;
					tile_pos->y = yt2;
				}
				im = layer_im;
			}
		}
		if (im < 101) {
			if (tile_pos) {
				tile_pos->x *= _tw;
				tile_pos->y *= _th;
				tile_pos->x += _tw / 2;
				tile_pos->y += _th / 2;
			}
			//LOG_DEBUG(("im = %d", im));	
			return im;
		}
	}
	if (im == 101) 
		im = 0;

	if (tile_pos) {
		tile_pos->x *= _tw;
		tile_pos->y *= _th;
		tile_pos->x += _tw / 2;
		tile_pos->y += _th / 2;
	}

	//LOG_DEBUG(("im = %d", im));
	return im;
}


void IMap::load(const std::string &name) {
	clear();
	
	LOG_DEBUG(("loading map '%s'", name.c_str()));
	const std::string file = "data/maps/" + name + ".tmx";
	parseFile(file);

	_name = name;
	
	LOG_DEBUG(("optimizing layers..."));
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		l->second->optimize(_tiles);
	}

#ifdef PRERENDER_LAYERS
	LOG_DEBUG(("rendering layers..."));
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		l->second->surface.createRGB(_w * _tw, _h * _th, 24);
		//l->second->surface.convertAlpha();
		//l->second->surface.convertToHardware();
		
		for(int ty = 0; ty < _h; ++ty) {
			for(int tx = 0; tx < _w; ++tx) {
				const sdlx::Surface * s = l->second->getSurface(tx, ty);
				if (s == NULL) 
					continue;
				l->second->surface.copyFrom(*s, tx * _tw, ty * _th);
			}
		}
		//static int i;
		//l->second->surface.saveBMP(mrt::formatString("layer%d.bmp", i++));
	}
#endif
	
	_imp_map.setSize(_h * _th / pathfinding_step, _w * _tw / pathfinding_step);
	LOG_DEBUG(("building map matrix[%d:%d]...", _imp_map.getHeight(), _imp_map.getWidth()));
	_imp_map.useDefault(-1);
	
	for(int y = 0; y < _h; ++y) {
		for(int x = 0; x < _w; ++x) {
			int im = 0;
			for(LayerMap::reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
				int tid = l->second->get(x, y);
				if (tid == 0)
					continue;
				int l_im = l->second->impassability;
				if (l_im == -1)
					continue;
				
				im = l_im;
				break;
			}
			if (im == 100) 
				im = -1; //inf :)
			//_imp_map.set(y, x, im);
			
			for(int y1 = y * _th / pathfinding_step; y1 < (y + 1) * _th / pathfinding_step; ++y1) 
				for(int x1 = x * _tw / pathfinding_step; x1 < (x + 1) * _tw / pathfinding_step; ++x1) 
					_imp_map.set(y1, x1, im);
			
		}
	}
	LOG_DEBUG(("\n%s", _imp_map.dump().c_str()));
	
	LOG_DEBUG(("loading completed"));
}

void IMap::start(const std::string &name, Attrs &attrs) {
	//LOG_DEBUG(("started %s", name.c_str()));
	Entity e(attrs);
	
	if (name == "map") {
		LOG_DEBUG(("map file version %s", e.attrs["version"].c_str()));
		_w = atol(e.attrs["width"].c_str());
		_h = atol(e.attrs["height"].c_str());
		_tw = atol(e.attrs["tilewidth"].c_str());
		_th = atol(e.attrs["tileheight"].c_str());
		
		if (_tw < 1 || _th < 1 || _w < 1 || _h < 1)
			throw_ex(("invalid map parameters. %dx%d tile: %dx%d", _w, _h, _tw, _th));
		
		LOG_DEBUG(("initializing map. size: %dx%d, tilesize: %dx%d", _w, _h, _tw, _th));
	} else if (name == "tileset") {
		_firstgid = atol(e.attrs["firstgid"].c_str());
		if (_firstgid < 1) 
			throw_ex(("tileset.firstgid must be > 0"));
		LOG_DEBUG(("tileset: '%s'. firstgid = %d", e.attrs["name"].c_str(), _firstgid));
	} else if (name == "layer") {
		_properties.clear();
		layer = true;
	}
	
	_stack.push(e);
}

void IMap::end(const std::string &name) {
	assert(!_stack.empty());
	Entity &e = _stack.top();
	
	if (name == "tile") {
		if (e.attrs.find("id") == e.attrs.end())
			throw_ex(("tile.id was not found")); 
			
		if (_image == NULL) 
			throw_ex(("tile must contain <image> inside it."));
		
		unsigned int id = atol(e.attrs["id"].c_str());
		id += _firstgid;
		LOG_DEBUG(("tile gid = %d, image: %p", id, (void *)_image));

		//TileManager->set(id, _image);
		//_tiles.reserve(id + 2);
		if (id >= _tiles.size())
			_tiles.resize(id + 20);
		
		sdlx::Surface * &tile = _tiles[id];	
		assert (tile == NULL);
		tile = _image;
		
		_image = NULL;

	} else if (name == "data") {
		std::string enc = e.attrs["encoding"];
		if (enc.size() == 0) enc = "none";
		std::string comp = e.attrs["compression"];
		if (comp.size() == 0) comp = "none";

		LOG_DEBUG(("data found. encoding: %s, compression: %s", enc.c_str(), comp.c_str()));
		
		mrt::Chunk data;
		if (enc == "base64") {
			mrt::Base64::decode(data, e.data);
		} else if (enc == "none") {
			data.setData(e.data.c_str(), e.data.size());
		} else throw_ex(("unknown encoding %s used", enc.c_str()));
		
		//LOG_DEBUG(("decoded size: %d", data.getSize()));

		if (comp == "gzip") {
			mrt::ZStream::decompress(_data, data);
		} else if (comp == "none") {
			_data = data;
		} else throw_ex(("unknown compression method ('%s') used. ", comp.c_str()));
		//LOG_DEBUG(("%s", _data.dump().c_str()));
	} else if (name == "image") {
		delete _image;
		_image = NULL;
		
		_image = new sdlx::Surface;
		std::string source = e.attrs["source"];
		if (source.size()) {
			source = "data/tiles/" + source;
			LOG_DEBUG(("loading tileset from single file ('%s')", source.c_str()));
			_image->loadImage(source);
			_image_is_tileset = true;
		} else {
			_image->loadImage(_data);
			_image_is_tileset = false;
		}
		//_image->convert(SDL_ASYNCBLIT | SDL_HWSURFACE);
		_image->convertAlpha();
		_image->convertToHardware();
		
		LOG_DEBUG(("image loaded. (%dx%d) format: %s", _image->getWidth(), _image->getHeight(), e.attrs["format"].c_str()));
	} else if (name == "layer") {
		int w = atol(e.attrs["width"].c_str());
		int h = atol(e.attrs["height"].c_str());
		int z = (_properties.find("z") == _properties.end())?++_lastz:atol(_properties["z"].c_str());
		_lastz = z;
		int impassability = (_properties.find("impassability") != _properties.end())?atoi(_properties["impassability"].c_str()):-1;
		const char pc = (_properties.find("pierceable") != _properties.end() && _properties["pierceable"].empty())?'t':_properties["pierceable"][0];
		const bool pierceable = pc == 't' || pc == 'T' || pc == '1';

		LOG_DEBUG(("layer '%s'. %dx%d. z: %d, size: %d, impassability: %d", e.attrs["name"].c_str(), w, h, z, _data.getSize(), impassability));
		if (_layers.find(z) != _layers.end())
			throw_ex(("layer with z %d already exists", z));
		_layers[z] = new Layer(w, h, _data, impassability, pierceable);
		//LOG_DEBUG(("(1,1) = %d", _layers[z]->get(1,1)));
		layer = false;
	} else if (name == "property") {
		if (layer)
			_properties[e.attrs["name"]] = e.attrs["value"];
		else 
			properties[e.attrs["name"]] = e.attrs["value"];
	} else if (name == "tileset" && _image != NULL && _image_is_tileset) {
		//fixme: do not actualy chop image in many tiles at once, use `tile' wrapper
		_image->setAlpha(0, 0);
		int w = _image->getWidth(), h = _image->getHeight();
		int id = 0;
		for(int y = 0; y < h; y += _th) {
			for(int x = 0; x < w; x += _tw) {
				sdlx::Surface *s = new sdlx::Surface;
				s->createRGB(_tw, _th, 24);
				s->convertAlpha();
				s->convertToHardware();

				sdlx::Rect from(x, y, _tw, _th);
				s->copyFrom(*_image, from);
				//s->saveBMP(mrt::formatString("tile-%d.bmp", id));

				//LOG_DEBUG(("cut tile %d from tileset [%d:%d, %d:%d]", _firstgid + id, x, y, _tw, _th));
				if ((size_t)(_firstgid + id) >= _tiles.size())
					_tiles.resize(_firstgid + id + 20);
				
				delete _tiles[_firstgid + id];
				_tiles[_firstgid + id++] = s;
				s = NULL;
			}
		}

		delete _image;
		_image = NULL;
	}
	
	_stack.pop();
}

void IMap::charData(const std::string &d) {
	assert(!_stack.empty());
	
	std::string data(d);
	mrt::trim(data);
	if (data.size() == 0)
		return;
	
	//LOG_DEBUG(("char %s", data.c_str()));
	_stack.top().data = d;
}

void IMap::render(sdlx::Surface &window, const sdlx::Rect &dst, const int z1, const int z2) const {
	if (_w == 0 || z1 >= z2)  //not loaded
		return;

#ifdef PRERENDER_LAYERS
	for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) 	
		if (l->first >= z1) {
			if (l->first >= z2) 
				break;
			window.copyFrom(l->second->surface, dst);
		}
#else
	int txp = dst.x / _tw, typ = dst.y / _th;
	int xp = - (dst.x % _tw), yp = -(dst.y % _th);
	
	int txn = (dst.w - 1) / _tw + 2;
	int tyn = (dst.h - 1) / _th + 2;
	
	LayerMap::const_iterator lz1 = _layers.lower_bound(z1);
	for(LayerMap::const_iterator l = lz1; l != _layers.end(); ++l) {
		if (l->first >= z2) //much quicker than lower_bound(z2)
			break;
		//LOG_DEBUG(("z: %d << %d, layer: %d", z1, z2, l->first));
		
		for(int ty = 0; ty < tyn; ++ty) {
			for(int tx = 0; tx < txn; ++tx) {
				const sdlx::Surface * s = l->second->getSurface(txp + tx, typ + ty);
				if (s != NULL) 
					window.copyFrom(*s, xp + tx * _tw, yp + ty * _th);
			}
		}
	}
#endif
}


void IMap::clear() {
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		delete i->second;
	}
	_layers.clear();
	
	for(TileMap::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		delete *i;
	}
	_tiles.clear();
	
	properties.clear();
	_properties.clear();

	_image = NULL;
	_lastz = -100;
	_w = _h = _tw = _th = _firstgid = 0;
}

IMap::~IMap() {
	clear();
}

const bool IMap::loaded() const {
	return _w != 0;
}

const v3<int> IMap::getSize() const {
	return v3<int>(_tw * _w,_th * _h, 0);
}

const v3<int> IMap::getTileSize() const {
	return v3<int>(_tw, _th, 0);
}

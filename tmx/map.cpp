
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "map.h"
#include "layer.h"
#include "base_object.h"

#include "mrt/file.h"
#include "mrt/b64.h"
#include "mrt/gzip.h"
#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/surface.h"
#include "sdlx/c_map.h"
#include "object.h"

#include <assert.h>
#include <limits>

#include "config.h"

IMPLEMENT_SINGLETON(Map, IMap)

const int IMap::pathfinding_step = 64;

IMap::IMap() : _w(0), _h(0), _tw(0), _th(0), _firstgid(0) {
	_lastz = -1000;
	_image = NULL;
}


const bool IMap::collides(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL)
		return false;
	return obj->collides(tile, -dx, -dy);
}

const bool IMap::hiddenBy(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL)
		return false;
	return obj->collides(tile, -dx, -dy, true);
}


const int IMap::getImpassability(const Object *obj, const v3<int>&pos, v3<int> *tile_pos, bool *hidden) const {
	if (obj->impassability <= 0) {
		return 0;
	}
	GET_CONFIG_VALUE("engine.disable-outlines", bool, disable_outlines, false);
	if (disable_outlines && hidden != NULL) {
		*hidden = false;
		hidden = NULL;
	}
	
	if (hidden)
		*hidden = false;
	
	const float obj_z = obj->getPosition().z;
	int w = (int)obj->size.x, h = (int)obj->size.y;
	int x, x1;
	int y, y1;
	x = x1 = pos.x;
	y = y1 = pos.y;
	
	int x2 = x1 + w; int y2 = y1 + h;
	
	int xt1 = x1 / _tw; int xt2 = x2 / _tw;
	int yt1 = y1 / _th; int yt2 = y2 / _th; 
	int dx1 = x - xt1 * _tw; int dx2 = x - xt2 * _tw;
	int dy1 = y - yt1 * _th; int dy2 = y - yt2 * _th;
	
	int hidden_mask = 0;

	int im = 101;
	int result_im = 101;
	//LOG_DEBUG(("%d:%d:%d:%d --> %d:%d %d:%d", x1, y1, w, h, xt1, yt1, xt2, yt2));
	for(LayerMap::const_reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
		const Layer *layer = l->second;
		int layer_im = layer->impassability;

		if (hidden && l->first > obj_z) {
			if (hiddenBy(obj, dx1, dy1, layer->getCollisionMap(xt1, yt1)))
				hidden_mask |= 1;
			if (yt1 != yt2 && hiddenBy(obj, dx1, dy2, layer->getCollisionMap(xt1, yt2)))
				hidden_mask |= 2;
			if (xt1 != xt2) {
				if (hiddenBy(obj, dx2, dy1, layer->getCollisionMap(xt2, yt1)))
					hidden_mask |= 4;
				if (yt1 != yt2 && hiddenBy(obj, dx2, dy2, layer->getCollisionMap(xt2, yt2)))
					hidden_mask |= 8;
			}
		}

		if (layer_im == -1 || (layer->pierceable && obj->piercing)) {
			continue;
		}
		
		if (result_im != 101)
			continue;
		
		//LOG_DEBUG(("im: %d, tile: %d", layer_im, layer->get(xt1, yt1)));
		if (collides(obj, dx1, dy1, layer->getCollisionMap(xt1, yt1)) && im > layer_im) {
			if (tile_pos) {
				tile_pos->x = xt1;
				tile_pos->y = yt1;
			}
			im = layer_im;
		};

		if (yt2 != yt1 && collides(obj, dx1, dy2, layer->getCollisionMap(xt1, yt2)) && im > layer_im) {
			if (tile_pos) {
				tile_pos->x = xt1;
				tile_pos->y = yt2;
			}
			im = layer_im;
		}
		
		if (xt2 != xt1) {
			if (collides(obj, dx2, dy1, layer->getCollisionMap(xt2, yt1)) && im > layer_im) {
				im = layer_im;
				if (tile_pos) {
					tile_pos->x = xt2;
					tile_pos->y = yt1;
				}
			}
			if (yt2 != yt1 && collides(obj, dx2, dy2, layer->getCollisionMap(xt2, yt2)) && im > layer_im) {
				if (tile_pos) {
					tile_pos->x = xt2;
					tile_pos->y = yt2;
				}
				im = layer_im;
			};
		}

		if (result_im == 101 && im < 101) 
			result_im = im;
	}
	
	if (xt1 == xt2) {
		hidden_mask |= 0x0c;
	}
	if (yt1 == yt2) {
		hidden_mask |= 0x0a;
	}
	
	if (hidden_mask & 0x0f && hidden)
		*hidden = true;

	if (tile_pos) {
		tile_pos->x *= _tw;
		tile_pos->y *= _th;
		tile_pos->x += _tw / 2;
		tile_pos->y += _th / 2;
	}

	if (result_im >= 101) 
		result_im = 0;

	assert(result_im >= 0);
	//LOG_DEBUG(("im = %d", im));
	return result_im;
}


void IMap::load(const std::string &name) {
	clear();
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	
	LOG_DEBUG(("loading map '%s'", name.c_str()));
	const std::string file = data_dir + "/maps/" + name + ".tmx";
	parseFile(file);

	_name = name;
	
	LOG_DEBUG(("optimizing layers..."));
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		l->second->optimize(_tiles);
	}
	
	for(std::map<const std::string, std::string>::const_iterator i = _damage4.begin(); i != _damage4.end(); ++i) {
		Layer *dl = NULL, *l = NULL;
		dl = _layers[_layer_z[i->first]];
		if (dl == NULL)
			throw_ex(("layer %s doesnt exits", i->first.c_str()));
		l = _layers[_layer_z[i->second]];
		if (l == NULL)
			throw_ex(("layer %s doesnt exits", i->second.c_str()));
		LOG_DEBUG(("mapping damage layers: %s -> %s", i->first.c_str(), i->second.c_str()));
		ChainedDestructableLayer *cl = dynamic_cast<ChainedDestructableLayer *>(dl);
		if (cl == NULL) 
			throw_ex(("layer %s is not destructable", i->first.c_str()));
		cl->setSlave(l);
	}

#ifdef PRERENDER_LAYERS
	LOG_DEBUG(("rendering layers..."));
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		if (!l->visible)
			continue;
		
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
			
			for(int y1 = y * _th / pathfinding_step; y1 <= ((y + 1) * _th - 1 ) / pathfinding_step; ++y1) 
				for(int x1 = x * _tw / pathfinding_step; x1 <= ((x + 1) * _tw - 1) / pathfinding_step; ++x1) 
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
		_layer = true;
		_layer_name = e.attrs["name"];
		if (_layer_name.empty())
			throw_ex(("layer name cannot be empty!"));
	}
	
	_stack.push(e);
	NotifyingXMLParser::start(name, attrs);
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
		
		TileMap::value_type &tile = _tiles[id];	
		if (tile.first != NULL)
			throw_ex(("duplicate tile %d found", id));
		tile.second = new sdlx::CollisionMap;
		tile.second->init(_image);
		tile.first = _image;
		
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
		//LOG_DEBUG(("decoded data: %s -> %s", e.data.c_str(), data.dump().c_str()));

		if (comp == "gzip") {
			mrt::ZStream::decompress(_data, data);
		} else if (comp == "none") {
			_data = data;
		} else throw_ex(("unknown compression method ('%s') used. ", comp.c_str()));
		data.free();
		//LOG_DEBUG(("%s", _data.dump().c_str()));
	} else if (name == "image") {
		delete _image;
		_image = NULL;
		
		_image = new sdlx::Surface;
		std::string source = e.attrs["source"];
		GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");

		if (source.size()) {
			source = data_dir + "/tiles/" + source;
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
		
		bool pierceable = false;
		
		int hp = atoi(_properties["hp"].c_str());
		
		PropertyMap::const_iterator pi = _properties.find("pierceable");
		if (pi != _properties.end()) {
			pierceable = true;
			if (!pi->second.empty()) {
				unsigned char pc = pi->second[0];
				pierceable = pc == 't' || pc == 'T' || pc == '1';
			}
		}
		Layer *layer = NULL;
		if (!_properties["visible-if-damaged"].empty()) {
			layer = new DestructableLayer(true);
		}
		if (!_properties["invisible-if-damaged"].empty()) {
			if (layer != NULL) 
				throw_ex(("visible/invisible options is mutually exclusive"));
			layer = new DestructableLayer(false);
		}
		const std::string damage = _properties["damage-for"];
		if (!damage.empty()) {
			if (layer != NULL)
				throw_ex(("damage-for cannot be combined with (in)visible-if-damaged"));
			layer = new ChainedDestructableLayer();
			_damage4[_layer_name] = damage;
		}
		LOG_DEBUG(("layer '%s'. %dx%d. z: %d, size: %d, impassability: %d", e.attrs["name"].c_str(), w, h, z, _data.getSize(), impassability));
		if (_layers.find(z) != _layers.end())
			throw_ex(("layer with z %d already exists", z));
		if(layer == NULL)
			layer = new Layer;
		
		layer->impassability = impassability;
		layer->pierceable = pierceable;
		layer->hp = hp;

		layer->init(w, h, _data); //fixme: fix possible memory leak here, if exception occurs
		
		_layers[z] = layer;
		_layer_z[_layer_name] = z;
		//LOG_DEBUG(("(1,1) = %d", _layers[z]->get(1,1)));
		_layer = false;
	} else if (name == "property") {
		if (_layer)
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
				
				delete _tiles[_firstgid + id].first;
				_tiles[_firstgid + id].first = NULL;
				delete _tiles[_firstgid + id].second;
				_tiles[_firstgid + id].second = NULL;
				
				_tiles[_firstgid + id].second = new sdlx::CollisionMap;
				_tiles[_firstgid + id].second->init(s);
				_tiles[_firstgid + id].first = s;
				++id;
				s = NULL;
			}
		}

		delete _image;
		_image = NULL;
	}
	
	_stack.pop();
	NotifyingXMLParser::end(name);
}

void IMap::charData(const std::string &d) {
	assert(!_stack.empty());
	//LOG_DEBUG(("char1 %s", d.c_str()));
	std::string data(d);
	mrt::trim(data);
	if (data.size() == 0)
		return;
	
	//LOG_DEBUG(("char2 %s", data.c_str()));
	_stack.top().data += d;
}

void IMap::render(sdlx::Surface &window, const sdlx::Rect &src, const sdlx::Rect &dst, const int z1, const int z2) const {
	if (_w == 0 || z1 >= z2)  //not loaded
		return;
#ifdef PRERENDER_LAYERS
	for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) 	
		
		if (l->first >= z1) {
			if (l->first >= z2) 
				break;
			window.copyFrom(l->second->surface, src);
		}
#else
	int txp = src.x / _tw, typ = src.y / _th;
	int xp = - (src.x % _tw), yp = -(src.y % _th);
	
	int txn = (src.w - 1) / _tw + 2;
	int tyn = (src.h - 1) / _th + 2;
	
	for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) 
	if (l->first >= z1) {
		if (l->first >= z2) 
			break;

		//LOG_DEBUG(("z: %d << %d, layer: %d", z1, z2, l->first));
		
		for(int ty = 0; ty < tyn; ++ty) {
			for(int tx = 0; tx < txn; ++tx) {
				const sdlx::Surface * s = l->second->getSurface(txp + tx, typ + ty);
				if (s != NULL) 
					window.copyFrom(*s, dst.x + xp + tx * _tw, dst.y + yp + ty * _th);
			}
		}
	}
	//LOG_DEBUG(("====================================="));
#endif
}


void IMap::clear() {
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		delete i->second;
	}
	_layers.clear();
	
	for(TileMap::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		delete i->first;
		delete i->second;
	}
	_tiles.clear();
	
	properties.clear();
	_properties.clear();

	_image = NULL;
	_lastz = -100;
	_w = _h = _tw = _th = _firstgid = 0;

	_damage4.clear();
	_layer_z.clear();
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

void IMap::damage(const v3<float> &position, const int hp) {
	v3<int> pos = position.convert<int>();
	pos.x /= _tw;
	pos.y /= _th;
	//LOG_DEBUG(("map damage: %g:%g -> %d:%d for %d hp", position.x, position.y, pos.x, pos.y, hp));
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		i->second->damage(pos.x, pos.y, hp);
	}
}

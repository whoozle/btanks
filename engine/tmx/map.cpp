/* Battle Tanks_imp_map.get(yp, xp) Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "map.h"
#include "layer.h"
#include "base_object.h"
#include "rt_config.h"

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

#include "math/binary.h"

#include "config.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "finder.h"
#include "zbox.h"

#include "generator.h"
#include "mrt/scoped_ptr.h"

#include "math/range_list.h"

static range_list<Uint32> tile_stats;

IMPLEMENT_SINGLETON(Map, IMap);

IMap::IMap() : _w(0), _h(0), _tw(0), _th(0), _ptw(0), _pth(0), _firstgid(0), _split(0), 
	_generator(new MapGenerator), _torus(false) 
{
	_lastz = -1001;
	_image = NULL;
}

Matrix<int> &IMap::getMatrix(int z, const bool only_pierceable) {
	const int box = ZBox::getBox(z);
	MatrixMap::iterator i = _imp_map.find(MatrixMap::key_type(box, only_pierceable));
	if (i != _imp_map.end())
		return i->second;

	Matrix<int> map;
	GET_CONFIG_VALUE("map.default-impassability", int, def_im, 0);
	map.set_size(_h * _split, _w * _split, 0);
	map.useDefault(-1);
	std::pair<MatrixMap::iterator, bool> r = _imp_map.insert(MatrixMap::value_type(MatrixMap::key_type(box, only_pierceable), map));
	return r.first->second;
}

Matrix<int> &IMap::getMatrix(const std::string &name) {
	ObjectAreaMap::iterator i = _area_map.find(name);
	if (i != _area_map.end())
		return i->second;

	Matrix<int> map;
	map.set_size(_h * _split, _w * _split, 0);
	map.useDefault(0);
	std::pair<ObjectAreaMap::iterator, bool> r =_area_map.insert(ObjectAreaMap::value_type(name, map));
	return r.first->second;
}


const Matrix<int>& IMap::getAreaMatrix(const std::string &name) {
	return getMatrix(name);
}


const Matrix<int>& IMap::get_impassability_matrix(const int z, const bool only_pierceable) {
	return getMatrix(z, only_pierceable);
}

inline const bool IMap::collides(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL) {
		return false;
	}
	return obj->collides(tile, -dx, -dy);
}

inline const bool IMap::hiddenBy(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const {
	if (tile == NULL)
		return false;
	return obj->collides(tile, -dx, -dy, true);
}

/*
static const int im2(const int im1, const int im2) {
	assert(im1 < 101 && im2 < 101);
	if (im1 == -1)
		return im2;
	if (im2 == -1) 
		return im1;
	return math::max(im1, im2);
}
*/

const int IMap::getImpassability(const Object *obj, const v2<int>&pos, TilePosition *tile_pos, bool *hidden) const {
TRY {
	assert(obj != NULL);
	
	if (obj->impassability >= 0 && obj->impassability < 1.0f) {
		return 0;
	}
	//LOG_DEBUG((">>IMap::getImpassability"));
	if (hidden)
		*hidden = false;

	GET_CONFIG_VALUE("engine.disable-outlines", bool, disable_outlines, false);

	if (disable_outlines) {
		hidden = NULL;
	}
	
	v2<float> position, velocity;
	obj->get_position(position);
	obj->get_velocity(velocity);
	
	GET_CONFIG_VALUE("engine.debug-map-collision-code", bool, debug, false);
	
	const int obj_z = obj->get_z();
	int w = (int)obj->size.x, h = (int)obj->size.y;
	int dx1, dx2, dy1, dy2;
	int xt1, xt2, yt1, yt2;
	{
		//hide x1 and other common ids into {} block :)
		int x, x1;
		int y, y1;
		x = x1 = pos.x;
		y = y1 = pos.y;
	
		int x2 = x1 + w - 1; int y2 = y1 + h - 1;
	
		xt1 = x1 / _tw; xt2 = x2 / _tw;
		yt1 = y1 / _th; yt2 = y2 / _th; 
		dx1 = x - xt1 * _tw; dx2 = x - xt2 * _tw;
		dy1 = y - yt1 * _th; dy2 = y - yt2 * _th;
		if (debug)
			LOG_DEBUG(("%d:%d:%d:%d (%+d:%+d:%+d:%+d)--> %d:%d %d:%d", x1, y1, w, h, dx1, dy1, dx2, dy2, xt1, yt1, xt2, yt2));
	}
	int hidden_mask = 0, prev_im = 0;

	int empty_mask = 0x0f;
	int im[4] = {101, 101, 101, 101};
	
	if (collides(obj, dx1, dy1, &_full_tile))
		empty_mask &= ~0x01;
	if (dy1 != dy2 && collides(obj, dx1, dy2, &_full_tile))
		empty_mask &= ~0x02;
	if (dx1 != dx2) {
		if (collides(obj, dx2, dy1, &_full_tile))
			empty_mask &= ~0x04;
		if (dy1 != dy2 && collides(obj, dx2, dy2, &_full_tile))
			empty_mask &= ~0x08;
	}
	
	for(LayerMap::const_reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
		
		const Layer *layer = l->second;
		int layer_im = layer->impassability;

		if (hidden && l->second->visible && l->first > obj_z) {
			if (!(hidden_mask & 1)) {
				if ((empty_mask & 1) || hiddenBy(obj, dx1, dy1, getVisibilityMap(layer, xt1, yt1)))
					hidden_mask |= 1;
			}
			
			if (!(hidden_mask & 2)) {
				if ((empty_mask & 2) || hiddenBy(obj, dx1, dy2, getVisibilityMap(layer, xt1, yt2)))
					hidden_mask |= 2;
			}
			
			if (!(hidden_mask & 4)) {
				if ((empty_mask & 4) || hiddenBy(obj, dx2, dy1, getVisibilityMap(layer, xt2, yt1)))
					hidden_mask |= 4;
			}
			
			if (!(hidden_mask & 8)) {
				if ((empty_mask & 8) || hiddenBy(obj, dx2, dy2, getVisibilityMap(layer, xt2, yt2)))
					hidden_mask |= 8;
			}
		}

		if (layer_im == -1 || 
			(layer->pierceable && obj->piercing) || 
			!ZBox::sameBox(l->first, obj->get_z()))
			continue;
		
		if (!(empty_mask & 1) && im[0] == 101) {
			if (collides(obj, dx1, dy1, getCollisionMap(layer, xt1, yt1))) {
				im[0] = layer_im;
				if (layer_im < 100 && layer_im > prev_im) 
					prev_im = layer_im;
				if (debug)
					LOG_DEBUG(("%d: im[0] = %d", l->first, layer_im));
			}
		}

		if (!(empty_mask & 2) && im[1] == 101) {
			if (collides(obj, dx1, dy2, getCollisionMap(layer, xt1, yt2))) {
				im[1] = layer_im;
				if (layer_im < 100 && layer_im > prev_im) 
					prev_im = layer_im;
				if (debug)
					LOG_DEBUG(("%d: im[1] = %d", l->first, layer_im));
			}
		}
		
		if (!(empty_mask & 4) && im[2] == 101) {
			if (collides(obj, dx2, dy1, getCollisionMap(layer, xt2, yt1))) {
				im[2] = layer_im;
				if (layer_im < 100 && layer_im > prev_im) 
					prev_im = layer_im;
				if (debug)
					LOG_DEBUG(("%d: im[2] = %d", l->first, layer_im));
			}
		}
		
		if (!(empty_mask & 8) && im[3] == 101) {
			if (collides(obj, dx2, dy2, getCollisionMap(layer, xt2, yt2))) { 
				im[3] = layer_im;
				if (layer_im < 100 && layer_im > prev_im) 
					prev_im = layer_im;
				if (debug)
					LOG_DEBUG(("%d: im[3] = %d", l->first, layer_im));
			}
		}
	}

	int result_im = 0;

	if (empty_mask & 1) 
		im[0] = -1;
	if (empty_mask & 2) 
		im[1] = -1;
	if (empty_mask & 4) 
		im[2] = -1;
	if (empty_mask & 8) 
		im[3] = -1;

	GET_CONFIG_VALUE("map.default-impassability", int, def_im, 0);

	if (debug) {
		LOG_DEBUG(("im : %d %d", im[0], im[2])); 
		LOG_DEBUG(("im : %d %d", im[1], im[3]));
		LOG_DEBUG(("empty_mask: 0x%02x, default im: %d", empty_mask, def_im));
	}
	
	if (obj->piercing) 
		def_im = 0;
	
	for(int i = 0; i < 4; ++i) 
		if (im[i] == 101) 
			im[i] = def_im; //default im value for a layer.
	
	if (tile_pos) {
		tile_pos->prev_im = prev_im;
		tile_pos->merged_x = tile_pos->merged_y = false;
		
		bool v1 = im[0] == 100 || im[1] == 100;
		bool v2 = im[2] == 100 || im[3] == 100;
		
		if (v1 && !v2) {
			tile_pos->position.x = _tw/2 + _tw * xt1;
		} else if (v2 && !v1) {
			tile_pos->position.x = _tw/2 + _tw * xt2;			
		} else {
			tile_pos->position.x = _tw * xt2;
			tile_pos->merged_x = true;
		}
		
		bool h1 = im[0] == 100 || im[2] == 100;
		bool h2 = im[1] == 100 || im[3] == 100;
		if (h1 && !h2) {
			tile_pos->position.y = _th/2 + _th * yt1;
		} else if (h2 && !h1) {
			tile_pos->position.y = _th/2 + _th * yt2;
		} else {
			tile_pos->merged_y = true;
			tile_pos->position.y = _th * yt2;
		}
	}
	
	/*
	const int im_l = im2(im[0], im[1]);
	const int im_r = im2(im[2], im[3]);
	const int im_u = im2(im[0], im[2]);
	const int im_d = im2(im[1], im[3]);
	
	if (velocity.y < 0 && im_u < 101 ) {
		result_im = math::max(result_im, im_u);
		//LOG_DEBUG(("y<0 : %d", t));
	}
	if (velocity.y > 0 && im_d < 101 ) {
		result_im = math::max(result_im, im_d);
		//LOG_DEBUG(("y>0 : %d", t));
	}
	if (velocity.x < 0 && im_l < 101 ) {
		result_im = math::max(result_im, im_l);
		//LOG_DEBUG(("x<0 : %d", t));
	}
	if (velocity.x > 0 && im_r < 101 ) {
		result_im = math::max(result_im, im_r);
		//LOG_DEBUG(("x>0 : %d", t));
	}
	*/
	for(int i = 0; i < 4; ++i) {
		if (im[i] > result_im) 
			result_im = im[i];
	}
	
	if (xt1 == xt2) {
		hidden_mask |= 0x0c;
	}
	if (yt1 == yt2) {
		hidden_mask |= 0x0a;
	}
	
	if (hidden_mask == 0x0f && hidden)
		*hidden = true;

	assert(result_im >= 0 && result_im < 101);

	if (debug)
		LOG_DEBUG(("*** im = %d", result_im));
	if (result_im == 100)
		return 100;
	
	//LOG_DEBUG(("<<IMap::getImpassability"));
	return (int)(100 * obj->get_effective_impassability(result_im / 100.0f));
} CATCH(mrt::format_string("Map::getImpassability(%p, (%d:%d), %p, %p)", 
	(void *)obj, pos.x, pos.y, (void *)tile_pos, (void *)hidden ).c_str(), throw;);
	return 0;
}

void IMap::updateMatrix(const int x, const int y) {
	if (x < 0 || x >= _w || y < 0 || y >= _h)
		return;
	//LOG_DEBUG(("updating matrix at [%d,%d]", y, x));
	
	for(LayerMap::reverse_iterator l = _layers.rbegin(); l != _layers.rend(); ++l) {
				int im = l->second->impassability;
				if (im == -1)
					continue;
				
				int tid = l->second->get(x, y);
				if (tid == 0)
					continue;
				const sdlx::CollisionMap *cmap = getCollisionMap(l->second, x, y);
				if (cmap == NULL || cmap->is_empty())
					continue;

				Matrix<int> &imp_map = getMatrix(l->first, false);
				Matrix<int> *pmap = (l->second->pierceable) ? &getMatrix(l->first, true): NULL;
				
				//break;
				//if (im == 100) 
				//	im = -1; //inf :)
				//_imp_map.set(y, x, im);
			
			
				Matrix<bool> proj;
				cmap->project(proj, _split, _split);
				//LOG_DEBUG(("projection: %s", proj.dump().c_str()));
				//_imp_map.set(y, x, im);
				const bool destructable = dynamic_cast<const DestructableLayer *>(l->second) != NULL;
				if (destructable)
					im = -100;
				
				for(int yy = 0; yy < _split; ++yy)
					for(int xx = 0; xx < _split; ++xx) {
						int yp = y * _split + yy, xp = x * _split + xx;
						if (proj.get(yy, xx) && imp_map.get(yp, xp) == -2) {
							imp_map.set(yp, xp, im);
							if (pmap)
								pmap->set(yp, xp, im);
						}
					}
	}

	GET_CONFIG_VALUE("map.default-impassability", int, def_im, 0);

	for(MatrixMap::iterator i = _imp_map.begin(); i != _imp_map.end(); ++i) {
		Matrix<int>& imp_map = i->second;
		for(int yy = 0; yy < _split; ++yy)
			for(int xx = 0; xx < _split; ++xx) {
				int yp = y * _split + yy, xp = x * _split + xx;
			
				if (imp_map.get(yp, xp) == -2)
					imp_map.set(yp, xp, def_im);
	
				if (imp_map.get(yp, xp) >= 100)
					imp_map.set(yp, xp, -1);
				}
	}
}

void IMap::updateMatrix(Matrix<int> &imp_map, const Layer *layer) {
	for(int y = 0; y < layer->get_height(); ++y) 
		for(int x = 0; x < layer->get_width(); ++x) {
			int tid = layer->get(x, y);
			if (tid == 0)
				continue;

			const sdlx::CollisionMap *cmap = getCollisionMap(layer, x, y);
			if (cmap == NULL || cmap->is_empty())
				continue;

			Matrix<bool> proj;
			cmap->project(proj, _split, _split);

			for(int yy = 0; yy < _split; ++yy)
				for(int xx = 0; xx < _split; ++xx) {
					int yp = y * _split + yy, xp = x * _split + xx;
					if (proj.get(yy, xx)) 
						imp_map.set(yp, xp, 1);
				}
		}	
}

void IMap::correctGids() {
	//int delta = 0;
	unsigned max = 0x7fffffff;
	for(CorrectionMap::reverse_iterator i = _corrections.rbegin(); i != _corrections.rend(); ++i) {
		const int d = i->second - i->first;
		LOG_DEBUG(("correcting: gid: %d-%u, delta: %d", i->first, max, d));
		for(LayerMap::iterator j = _layers.begin(); j != _layers.end(); ++j) {
			j->second->correct(i->first, max, d);
		}
		max = i->first;
		//delta += d;
	}
}

void IMap::load(const std::string &name) {
	clear();

	LOG_DEBUG(("loading map '%s'", name.c_str()));
	std::string file;
	{
		IFinder::FindResult fr;
		Finder->findAll(fr, "maps/" + name + ".tmx");
		if (fr.empty())
			throw_ex(("could not find map '%s'", name.c_str()));
		_path = fr[0].first;
		file = fr[0].second;
	}
	
	parse_file(file);
	delete _image;
	_image = NULL;
	
	correctGids();

	_full_tile.create(_tw, _th, true);
	
	LOG_DEBUG(("optimizing layers..."));
	
	
	for(std::map<const std::string, std::string>::const_iterator i = _damage4.begin(); i != _damage4.end(); ++i) {
		Layer *dl = NULL, *l = NULL;
		dl = _layers[_layer_z[i->first]];
		if (dl == NULL)
			throw_ex(("layer %s doesnt exits", i->first.c_str()));
		int slave_z = _layer_z[i->second];
		l = _layers[slave_z];
		if (l == NULL)
			throw_ex(("layer %s doesnt exits", i->second.c_str()));
		LOG_DEBUG(("mapping damage layers: %s -> %s", i->first.c_str(), i->second.c_str()));
		ChainedDestructableLayer *cl = dynamic_cast<ChainedDestructableLayer *>(dl);
		if (cl == NULL) 
			throw_ex(("layer %s is not destructable", i->first.c_str()));
		cl->setSlave(slave_z, l);
	}

	_name = name;
	LOG_DEBUG(("loading completed"));
	for(range_list<Uint32>::const_iterator i = tile_stats.begin(); i != tile_stats.end(); ++i) {
		LOG_DEBUG(("%u-%u", i->first, i->second));
	}
	
	{
		PropertyMap::const_iterator p = properties.find("config:map.torus");
		if (p != properties.end()) {
			if (p->second.find("true") != std::string::npos) {
				_torus = true;
				LOG_DEBUG(("torus mode switched on..."));
			}
		}
	}
	
	load_map_signal.emit();
}

void IMap::generateMatrixes() {
	_cover_map.set_size(_h, _w, -10000);
	_cover_map.useDefault(-10000);
	
	if (!RTConfig->editor_mode) {
	unsigned int ot = 0;
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		if (!l->second->velocity.is0() || !l->second->visible)
			continue;
		
		for(int ty = 0; ty < _h; ++ty) {
			for(int tx = 0; tx < _w; ++tx) {
				const sdlx::CollisionMap * vmap = getVisibilityMap(l->second, tx, ty);
				if (vmap == NULL)
					continue;
				if (vmap->is_full()) {
					_cover_map.set(ty, tx, l->first);
					++ot;
				}
			}
		}
	}

	LOG_DEBUG(("created render optimization map. opaque tiles found: %u, dump: \n%s", ot, _cover_map.dump().c_str()));
	}

	_imp_map.clear();
	for(LayerMap::const_iterator i = _layers.begin(); i != _layers.end(); ++i) {
		const Layer *layer = i->second;
		
		getMatrix(i->first, false).fill(-2);
		if (layer->pierceable)
			getMatrix(i->first, true).fill(-2);
				
	}

	for(int y = 0; y < _h; ++y) {
		for(int x = 0; x < _w; ++x) {
			updateMatrix(x, y);
		}
	}
	for(MatrixMap::const_iterator i = _imp_map.begin(); i != _imp_map.end(); ++i) {
		LOG_DEBUG(("z: %d(pierceable: %s)\n%s", i->first.first, i->first.second?"yes":"no", i->second.dump().c_str()));
	}

	for(LayerMap::const_iterator i = _layers.begin(); i != _layers.end(); ++i) {
		const Layer *layer = i->second;
		//LOG_DEBUG(("size(%s.properties) == %u", layer->name.c_str(), (unsigned)layer->properties.size()));
		for(PropertyMap::const_iterator p = layer->properties.begin(); p != layer->properties.end(); ++p) {
			//LOG_DEBUG(("%s.%s=%s", layer->name.c_str(), p->first.c_str(), p->second.c_str()));
			if (p->first.compare(0, 8, "ai-hint:") == 0) {
				LOG_DEBUG(("layer %d %s provide hint for %s", i->first, layer->name.c_str(), p->second.c_str()));
				updateMatrix(getMatrix(p->second), layer);
			}
		}
	}
	
	for(ObjectAreaMap::const_iterator i = _area_map.begin(); i != _area_map.end(); ++i) {
		LOG_DEBUG(("hint for '%s'\n%s", i->first.c_str(), i->second.dump().c_str()));
	}
	
	load_map_final_signal.emit();
}

void IMap::get_zBoxes(std::set<int> &layers) {
	layers.clear();
	for(MatrixMap::const_iterator i = _imp_map.begin(); i != _imp_map.end(); ++i) {
		layers.insert(i->first.first);
	}
}

void IMap::getSurroundings(Matrix<int> &matrix, const Object *obj, const int filler) const {
	if (matrix.get_width() % 2 == 0 || matrix.get_height() % 2 == 0)
		throw_ex(("use only odd values for surrond matrix. (used: %d, %d)", matrix.get_height(), matrix.get_width()));

	const int box = ZBox::getBox(obj->get_z());
	MatrixMap::const_iterator map = _imp_map.find(MatrixMap::key_type(box, false));
	if (map == _imp_map.end()) {
		matrix.fill(filler);
		return;
	}
	
	MatrixMap::const_iterator pmap = (obj->piercing)?_imp_map.find(MatrixMap::key_type(box, true)):_imp_map.end();
	
	int dx = (matrix.get_width() - 1) / 2;
	int dy = (matrix.get_height() - 1) / 2;
	
	v2<int> p;
	obj->get_center_position(p);
	p.x /= _tw;
	p.y /= _th;
	
	int y0 = p.y - dy, x0 = p.x - dx;
	for(int y = y0; y <= p.y + dy; ++y) 
		for(int x = x0; x <= p.x + dx; ++x) {
			int i = map->second.get(y, x);
			if (filler != -1 && i < 0)
				i = filler;

			if (obj->piercing && pmap != _imp_map.end()) {
				if (pmap->second.get(y, x))
					i = 0;
			}
			matrix.set(y - y0, x - x0, i);
		}
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
		
		GET_CONFIG_VALUE("map.pathfinding-step", int, ps, 32);
	
		_split = 2 * ((_tw - 1) / 2 + 1) / ps;
		LOG_DEBUG(("split mode: %dx", _split));
	
		_pth = _tw / _split;
		_ptw = _th / _split;

		if (_tw < 1 || _th < 1 || _w < 1 || _h < 1)
			throw_ex(("invalid map parameters. %dx%d tile: %dx%d", _w, _h, _tw, _th));
		
		LOG_DEBUG(("initializing map. size: %dx%d, tilesize: %dx%d", _w, _h, _tw, _th));
	} else if (name == "tileset") {
		status = "tileset";
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
	} else if (name == "properties") {
		if (!_layer)
			status = "properties";
	}
	
	_stack.push(e);
	NotifyingXMLParser::start(name, attrs);
}

void IMap::end(const std::string &name) {
	assert(!_stack.empty());
	Entity &e = _stack.top();
	
	if (name == "tile") {
		status = "tileset";
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
		if (tile.surface != NULL)
			throw_ex(("duplicate tile %d found", id));

		tile.cmap = new sdlx::CollisionMap;
		tile.cmap->init(_image, sdlx::CollisionMap::OnlyOpaque);
		tile.vmap = new sdlx::CollisionMap;
		tile.vmap->init(_image, sdlx::CollisionMap::AnyVisible);
		tile.surface = _image;
		
		_image = NULL;

	} else if (name == "data") {
		std::string enc = e.attrs["encoding"];
		if (enc.empty()) enc = "none";
		std::string comp = e.attrs["compression"];
		if (comp.empty()) comp = "none";

		LOG_DEBUG(("data found. encoding: %s, compression: %s", enc.c_str(), comp.c_str()));
		
		mrt::Chunk data;
		if (enc == "base64") {
			mrt::Base64::decode(data, e.data);
		} else if (enc == "none") {
			data.set_data(e.data.c_str(), e.data.size());
		} else throw_ex(("unknown encoding %s used", enc.c_str()));
		
		//LOG_DEBUG(("decoded size: %d", data.get_size()));
		//LOG_DEBUG(("decoded data: %s -> %s", e.data.c_str(), data.dump().c_str()));

		if (comp == "gzip") {
			mrt::ZStream::decompress(_data, data, true);
		} else if (comp == "none") {
			_data = data;
		} else throw_ex(("unknown compression method ('%s') used. ", comp.c_str()));
		data.free();
		//LOG_DEBUG(("%s", _data.dump().c_str()));
	} else if (name == "image") {
		status = "tileset";
		delete _image;
		_image = NULL;
		
		_image = new sdlx::Surface;
		std::string source = e.attrs["source"];
		if (source.size()) {
			LOG_DEBUG(("loading tileset from single file ('%s')", source.c_str()));
			_image_source = source;
			_image_name = Finder->find("maps/" + source, false);
			if (_image_name.empty()) {
				//last resort, try match filename with any tilesets folder.
				_image_name = Finder->find("tilesets/" + mrt::FSNode::get_filename(source));
			}
			source = _image_name;
			
			scoped_ptr<mrt::BaseFile> file(Finder->get_file(source, "rb"));

			mrt::Chunk data;
			file->read_all(data);
			file->close();
			
			_image->load_image(data);
			_image_is_tileset = true;
		} else {
			_image->load_image(_data);
			_image_is_tileset = false;
		}
		_image->display_format_alpha();
		
		LOG_DEBUG(("image loaded. (%dx%d)", _image->get_width(), _image->get_height()));
	} else if (name == "layer") {
		status = "layer";
		int w = atol(e.attrs["width"].c_str());
		int h = atol(e.attrs["height"].c_str());
		int z = (_properties.find("z") == _properties.end())?++_lastz:atol(_properties["z"].c_str());
		_lastz = z;
		int impassability = (_properties.find("impassability") != _properties.end())?atoi(_properties["impassability"].c_str()):-1;
		
		bool pierceable = false;
		
		int hp = (_properties.find("hp") != _properties.end())?atoi(_properties["hp"].c_str()):0;
		
		PropertyMap::const_iterator pi = _properties.find("pierceable");
		if (pi != _properties.end()) {
			pierceable = true;
			if (!pi->second.empty()) {
				unsigned char pc = pi->second[0];
				pierceable = pc == 't' || pc == 'T' || pc == '1';
			}
		}
		Layer *layer = NULL;
		if (!RTConfig->editor_mode) {
		
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

		} //editor
		
		LOG_DEBUG(("layer '%s'. %dx%d. z: %d, size: %u, impassability: %d", e.attrs["name"].c_str(), w, h, z, (unsigned)_data.get_size(), impassability));
		if (_layers.find(z) != _layers.end())
			throw_ex(("layer with z %d already exists", z));
		if(layer == NULL)
			layer = new Layer;
			
		if (RTConfig->editor_mode) {
			int visible = (!e.attrs["visible"].empty())?atoi(e.attrs["visible"].c_str()):-1;
			LOG_DEBUG(("visible = %d", visible));
			if (visible == 0)
				layer->visible = false;
		} else {
			//hide layers with 'hidden' attribute set
			PropertyMap::const_iterator i = _properties.find("hidden");
			if (i != _properties.end() && !i->second.empty() && (i->second[0] == 't' || i->second[0] == 'T' || i->second[0] == '1')) {
				layer->visible = false;
			}
		}
		layer->properties = _properties;
		layer->name = e.attrs["name"];

		const std::string a_frame_size = _properties["animation-frame-size"];
		const std::string a_frames = _properties["animation-frames"];
		const std::string a_speed = _properties["animation-speed"];
		if (!a_frame_size.empty() && !a_frames.empty()) {
			int fs = atoi(a_frame_size.c_str());
			int fn = atoi(a_frames.c_str());
			float speed = (a_speed.empty())?1.0f:(float)atof(a_speed.c_str());
			if (a_speed.empty())
				LOG_WARN(("layer '%s': default speed of 1 used.", e.attrs["name"].c_str()));
			LOG_DEBUG(("layer '%s': animation-frame-size: %d, animation-speed: %g", e.attrs["name"].c_str(), fs, speed));
			layer->setAnimation(fs, fn, speed);
		}
		
		const std::string a_velocity = _properties["shifting-velocity"];
		const std::string a_size = _properties["shifting-size"];
		if (!a_velocity.empty() && !a_size.empty()) {
			v2<int> vel, size;
			vel.fromString(a_velocity);
			size.fromString(a_size);
			if (size.x <= 0 || size.y <= 0)	
				throw_ex(("shift size must not be negative or zero"));
			layer->velocity = vel.convert<float>();
			layer->size = size * v2<int>(_tw, _th);
			LOG_DEBUG(("shifting rendering: velocity: (%g,%g) wrapping: %dx%d", layer->velocity.x, layer->velocity.y, layer->size.x, layer->size.y));
		}
		
		layer->impassability = impassability;
		layer->pierceable = pierceable;
		layer->hp = hp;

		TRY { 
			GET_CONFIG_VALUE("map.log-tile-stats", bool, lts, false);
			if (lts) {
				Uint32 n = w * h;
				Uint32 *p = (Uint32 *)_data.get_ptr();
				for(size_t i = 0; i < n; ++i) {
					Uint32 t = SDL_SwapLE32(*p++);
					if (t > 0)
						tile_stats.insert(t);
				}
			}

			layer->init(w, h, _data); 
		} CATCH(mrt::format_string("layer '%s'", _layer_name.c_str()).c_str(), 
			{delete layer; layer = NULL; throw; }
		);
		
		if (!RTConfig->editor_mode) 
		for(PropertyMap::iterator i = _properties.begin(); i != _properties.end(); ++i) {
			if (i->first.compare(0, 10, "generator:") == 0) {		
				TRY {
					_generator->exec(layer, i->first.substr(i->first.find(":", 11) + 1), i->second);
				} CATCH("executing generator's commands", {})
			}
		}
		
		_layers[z] = layer;
		_layer_z[_layer_name] = z;
		//LOG_DEBUG(("(1,1) = %d", _layers[z]->get(1,1)));
		_layer = false;
	} else if (name == "property") {
		std::string name = e.attrs["name"];
		mrt::trim(name);
		if (_layer) {
			if (_properties.find(name) != _properties.end())
				throw_ex(("duplicate property name '%s' found in layer %s", name.c_str(), _layer_name.c_str()));
			_properties[name] = e.attrs["value"];
		} else {
			if (properties.find(name) != properties.end())
				throw_ex(("duplicate property name '%s' found", name.c_str()));
			properties[name] = e.attrs["value"];
		}
	} else if (name == "tileset" && _image != NULL && _image_is_tileset) {
		status = "tileset";

		int n = ((_image->get_width() - 1) / _tw + 1) * ((_image->get_height() - 1) / _th + 1);
		LOG_DEBUG(("tileset: %s, first_gid: %d, estimated tiles: %d", _image_source.c_str(), _firstgid, n));
		
		int gid = _tilesets.add(_image_source, _firstgid, n);
		if (gid != _firstgid) 
			_corrections.insert(CorrectionMap::value_type(_firstgid, gid));
		_firstgid = gid;
		_generator->tileset(_image_name, _firstgid);

		addTiles(_image, _firstgid);

		delete _image;
		_image = NULL;
	}
	
	_stack.pop();
	NotifyingXMLParser::end(name);
}

void IMap::addTileset(const std::string &tileset) {
	if (!loaded())
		throw_ex(("addTileset(%s) on uninitialized map", tileset.c_str()));
	const sdlx::Surface *image = ResourceManager->load_surface("../maps/" + tileset);
	std::string fname = Finder->find("tiles/" + tileset);
	int gid = _tilesets.last() + 1;
	int n = addTiles(image, gid);
	_generator->tileset(fname, gid);
	_tilesets.add(tileset, gid, n);
}


void IMap::cdata(const std::string &d) {
	assert(!_stack.empty());
	//LOG_DEBUG(("char1 %s", d.c_str()));
	std::string data(d);
	mrt::trim(data);
	if (data.empty())
		return;
	
	//LOG_DEBUG(("char2 %s", data.c_str()));
	_stack.top().data += d;
}

const bool IMap::hasSoloLayers() const {
	bool solo_layer = false;
	if (RTConfig->editor_mode) {
		for(LayerMap::const_iterator l = _layers.begin(); l != _layers.end(); ++l) 
			if (l->second->solo) {
				solo_layer = true;
				break;
			}
	}
	return solo_layer;
}

void IMap::render(sdlx::Surface &window, const sdlx::Rect &src, const sdlx::Rect &dst, const int z1, const int z2) const {
	if (_w == 0 || z1 >= z2)  //not loaded
		return;

	int txn = (dst.w - 1) / _tw + 2;
	int tyn = (dst.h - 1) / _th + 2;
	
	//unsigned int skipped = 0;
	const bool _solo_layer = hasSoloLayers();
	const v2<int> tile_size(_tw, _th);
	GET_CONFIG_VALUE("engine.strip-alpha-from-map-tiles", bool, strip_alpha, false);
	
	for(LayerMap::const_iterator l = _layers.lower_bound(z1); l != _layers.end(); ++l) {
		const int z = l->first;
		if (_solo_layer && !l->second->solo)
			continue;
		
		if (z < z1) 
			continue;
		
		if (z >= z2) 
			break;

		if (!l->second->visible && (!_solo_layer || !l->second->solo)) 
			continue;

		//if (strip_alpha && l->second->impassability == -1) 
		//	continue;
		
		const bool shifting = !l->second->velocity.is0();
		//LOG_DEBUG(("z: %d << %d, layer: %d", z1, z2, l->first));

		v2<int> pos = v2<int>(src.x, src.y) - l->second->position.convert<int>();
		pos.x %= _tw * _w; pos.y %= _th * _h;
		if (pos.x < 0) pos.x += _tw * _w;
		if (pos.y < 0) pos.y += _th * _h;
			
		v2<int> tile_pos = pos / tile_size;
		v2<int> shift_pos = -(pos % tile_size);
		
		for(int ty = -1; ty < tyn; ++ty) {
			for(int tx = -1; tx < txn; ++tx) {
				int sx = (tile_pos.x + tx) % _w, sy = (tile_pos.y + ty) % _h;

				if (sx < 0)
					sx += _w;
				if (sy < 0)
					sy += _h;

				if (!strip_alpha && !shifting && z < _cover_map.get(sy, sx)) {//this tile covered by another tile
					//++skipped;
					continue;
				}
				
				const sdlx::Surface * s = get_surface(l->second, sx, sy);
				if (s == NULL)
					continue;
				
				const int dx = dst.x + tx * _tw + shift_pos.x, dy = dst.y + ty * _th + shift_pos.y;
				window.blit(*s, dx, dy);
			}
		}
	}
	//LOG_DEBUG(("blits skipped: %u", skipped));
	//LOG_DEBUG(("====================================="));
}


void IMap::clear() {
	LOG_DEBUG(("cleaning up..."));

	tile_stats.clear();
	//LOG_DEBUG(("clearing layers..."));
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		delete i->second;
	}
	_layers.clear();
	
	//LOG_DEBUG(("clearing surfaces and collision maps..."));
	for(TileMap::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		delete i->surface;
		delete i->cmap;
		delete i->vmap;
	}
	_tiles.clear();
	
	//LOG_DEBUG(("clearing properties..."));
	properties.clear();
	_properties.clear();

	//LOG_DEBUG(("deleting intermediate parser objects..."));
	delete _image;
	_image = NULL;
	_lastz = -1001;
	_w = _h = _tw = _th = _firstgid = 0;
	
	//LOG_DEBUG(("clearing damage layers and optimization maps..."));
	
	_imp_map.clear();
	_area_map.clear();
	
	_damage4.clear();
	_layer_z.clear();
	_cover_map.set_size(0, 0, 0);

	_corrections.clear();
	
	LOG_DEBUG(("clearing map generator..."));
	_generator->clear();
	
	_tilesets.clear();
	_name.clear();
	_path.clear();
	_torus = false;
}

IMap::~IMap() {
	LOG_DEBUG(("cleaning up map..."));
	clear();
	LOG_DEBUG(("clear() succeedes, deleting map generator..."));
	delete _generator;
}

const bool IMap::loaded() const {
	return _w != 0;
}

const v2<int> IMap::get_size() const {
	return v2<int>(_tw * _w,_th * _h);
}

const v2<int> IMap::getTileSize() const {
	return v2<int>(_tw, _th);
}

const v2<int> IMap::getPathTileSize() const {
	return v2<int>(_ptw, _pth);
}


void IMap::damage(const v2<float> &position, const int hp) {
	if (PlayerManager->is_client())
		return;

	v2<int> pos = position.convert<int>();
	validate(pos);
	pos.x /= _tw;
	pos.y /= _th;
	
	std::set<v3<int> > destroyed_cells;
	//LOG_DEBUG(("map damage: %g:%g -> %d:%d for %d hp", position.x, position.y, pos.x, pos.y, hp));
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		if (i->second->damage(pos.x, pos.y, hp))
			destroyed_cells.insert(v3<int>(pos.x, pos.y, i->first));
	}
	if (!destroyed_cells.empty())
		destroyed_cells_signal.emit(destroyed_cells);
}

void IMap::damage(const v2<float> &center, const int hp, const float radius) {
	if (PlayerManager->is_client())
		return;

	v2<float> position2 = center + radius, position = center - radius;
	std::set<v3<int> > destroyed_cells;
	
	v2<float> p;
	float r = radius * radius;
	for(p.y = position.y; p.y < position2.y; p.y += _th) {
		for(p.x = position.x; p.x < position2.x; p.x += _tw) {			
			if (p.quick_distance(center) <= r) {
				v2<int> pos ((int)(p.x / _tw), (int)(p.y / _th));
				validate(pos);
				//LOG_DEBUG(("map damage: %g:%g -> %d:%d for %d hp", position.x, position.y, pos.x, pos.y, hp));
				for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
					if (i->second->damage(pos.x, pos.y, hp))
						destroyed_cells.insert(v3<int>(pos.x, pos.y, i->first));
				}
			}
		}
	}
	if (!destroyed_cells.empty())
		destroyed_cells_signal.emit(destroyed_cells);	
}

void IMap::tick(const float dt) {
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		i->second->tick(dt);
	}
}

void IMap::_destroy(const int z, const v2<int> &cell) {
	LayerMap::iterator l = _layers.find(z);
	if (l == _layers.end())
		throw_ex(("cannot destroy cell at %d %d (z = %d)", cell.x, cell.y, z));
	l->second->_destroy(cell.x, cell.y);
}

void IMap::invalidateTile(const int x, const int y) {
	_cover_map.set(y, x, -10000);
	for(MatrixMap::iterator i = _imp_map.begin(); i != _imp_map.end(); ++i) 
		for(int yy = 0; yy < _split; ++yy)
			for(int xx = 0; xx < _split; ++xx) {
				int yp = y * _split + yy, xp = x * _split + xx;
			
				i->second.set(yp, xp, -2);
		}
	updateMatrix(x, y);
}

const Uint32 IMap::getTile(const Layer *l, const int x, const int y) const {
	if (!_torus)
		return l->get(x, y);
	int mx = x % _w, my = y % _h;
	return l->get(mx >= 0? mx: mx + _w, my >= 0? my: my + _h);
}

const sdlx::Surface* IMap::get_surface(const Layer *l, const int x, const int y) const {
	Uint32 t = getTile(l, x, y);
	if (t == 0 || t >= _tiles.size())
		return NULL;
	return _tiles[t].surface;
}
const sdlx::CollisionMap* IMap::getCollisionMap(const Layer *l, const int x, const int y) const {
	Uint32 t = getTile(l, x, y);
	if (t == 0 || t >= _tiles.size())
		return NULL;
	return _tiles[t].cmap;
}
const sdlx::CollisionMap* IMap::getVisibilityMap(const Layer *l, const int x, const int y) const {
	Uint32 t = getTile(l, x, y);
	if (t == 0 || t >= _tiles.size())
		return NULL;
	return _tiles[t].vmap;
}

void IMap::serialize(mrt::Serializator &s) const {
	s.add(_name);
	s.add(_path);
	s.add(_w); s.add(_h);
	s.add(_tw); s.add(_th);
	s.add(_ptw); s.add(_pth);
	s.add(_split);
	
	s.add((int)_tilesets.size());
	s.add((int)_layers.size());
	
	for(size_t i = 0; i < _tilesets.size(); ++i ) {
		s.add(_tilesets[i].first);	
		s.add(_tilesets[i].second);	
	}
	
	for(LayerMap::const_iterator i = _layers.begin(); i != _layers.end(); ++i) {
		s.add(i->first);
		int type = 'l';
		if (dynamic_cast<ChainedDestructableLayer *>(i->second) != NULL) 
			type = 'c';
		else if (dynamic_cast<DestructableLayer *>(i->second) != NULL) 
			type = 'd';

		s.add(type);
		s.add(*i->second);
	}
	
	s.add((int)properties.size());
	for(PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i) {
		s.add(i->first);
		s.add(i->second);
	}
}

void IMap::deserialize(const mrt::Serializator &s) {
	clear();

	s.get(_name);
	s.get(_path);
	s.get(_w); s.get(_h);
	s.get(_tw); s.get(_th);
	s.get(_ptw); s.get(_pth);
	s.get(_split);

	_full_tile.create(_tw, _th, true);
	
	int tn, ln;

	s.get(tn);
	s.get(ln);
	reset_progress.emit(tn + ln);
	
	while(tn--) {
		std::string name;
		int gid;
		s.get(name);	
		s.get(gid);	
		sdlx::Surface *image  = NULL;
		int n = 0;
		TRY {
			std::string fname = Finder->find("maps/" + name, false);
			if (fname.empty()) {
				//last resort, try match filename with any tilesets folder.
				fname = Finder->find("tilesets/" + mrt::FSNode::get_filename(name));
			}

			scoped_ptr<mrt::BaseFile> file(Finder->get_file(fname, "rb"));

			mrt::Chunk data;
			file->read_all(data);
			file->close();
			
			image = new sdlx::Surface;
			image->load_image(data);
			image->display_format_alpha();
			
			n = addTiles(image, gid);
			
			delete image;
			image = NULL;
		} CATCH("deserialize", { delete image; throw; });
		
		_tilesets.add(name, gid, n);
		notify_progress.emit(1, "tileset");
	}
	
	while(ln--) {
		int z;
		int type;
		s.get(z);
		s.get(type);

		Layer *layer = NULL;
		TRY {
			switch(type) {
			case 'c': 
				layer = new ChainedDestructableLayer();
				break;
			case 'd': 
				layer = new DestructableLayer(true);
				break;
			case 'l': 
				layer = new Layer;
				break;
			default: 
				throw_ex(("unknown layer type '%02x'(%c)", type, (type >= 0x20)?type:' '));
			}
			layer->deserialize(s);
			_layers.insert(LayerMap::value_type(z, layer));
		} CATCH("deserialize", {
			delete layer;
			throw;
		});
		
		notify_progress.emit(1, "layer");
	}

	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		ChainedDestructableLayer * cdl = dynamic_cast<ChainedDestructableLayer *>(i->second);
		if (cdl == NULL)
			continue;
		LayerMap::iterator l = _layers.find(cdl->slave_z);
		if (l == _layers.end())
			throw_ex(("no slave layer found (z: %d)", cdl->slave_z));
		cdl->setSlave(cdl->slave_z, l->second);
	}
	
	int pn;
	s.get(pn);
	while(pn--) {
		std::string name, value;
		s.get(name);
		s.get(value);
		properties.insert(PropertyMap::value_type(name, value));
	}

	{
		PropertyMap::const_iterator p = properties.find("config:map.torus");
		if (p != properties.end()) {
			if (p->second.find("true") != std::string::npos) {
				_torus = true;
				LOG_DEBUG(("torus mode switched on..."));
			}
		}
	}

	load_map_signal.emit();
}
	
const int IMap::addTiles(const sdlx::Surface *image, const int first_gid) {
	int id = 0;
TRY {
	const_cast<sdlx::Surface *>(image)->set_alpha(0, 0);
	int w = image->get_width(), h = image->get_height();

	for(int y = 0; y < h; y += _th) {
		for(int x = 0; x < w; x += _tw) {
			sdlx::Surface *s = new sdlx::Surface;
			s->create_rgb(_tw, _th, 24);
			s->display_format_alpha();

			sdlx::Rect from(x, y, _tw, _th);
			s->blit(*image, from);
			GET_CONFIG_VALUE("engine.strip-alpha-from-map-tiles", bool, strip_alpha, false);
			bool locked = false;
			if (strip_alpha) {
				s->lock();
				locked = true;
				Uint8 r,g,b,a;
				for(int y = 0; y < s->get_height(); ++y) 
					for(int x = 0; x < s->get_width(); ++x) {
						s->get_rgba(s->get_pixel(x, y), r, g, b, a);
						if (a != 255)
							s->put_pixel(x, y, s->map_rgba(r, g, b, (a > 51)?51:a));
					}
			}

			GET_CONFIG_VALUE("engine.mark-map-tiles", bool, marks, false);
			if (marks) {
				if (!locked) {
					s->lock();
					locked = true;
				}
				Uint32 color = s->map_rgba(255,0,255,249); //magic value to avoid Collision map confusing
				s->put_pixel(0, 0, color);
				s->put_pixel(1, 0, color);
				s->put_pixel(0, 1, color);
			}
			if (locked)
				s->unlock();

			//s->save_bmp(mrt::format_string("tile-%d.bmp", id));

			//LOG_DEBUG(("cut tile %d from tileset [%d:%d, %d:%d]", first_gid + id, x, y, _tw, _th));
			if ((size_t)(first_gid + id) >= _tiles.size())
				_tiles.resize(first_gid + id + 20);
				
			delete _tiles[first_gid + id].surface;
			_tiles[first_gid + id].surface = NULL;
			delete _tiles[first_gid + id].cmap;
			_tiles[first_gid + id].cmap = NULL;
			delete _tiles[first_gid + id].vmap;
			_tiles[first_gid + id].vmap = NULL;
				
			_tiles[first_gid + id].cmap = new sdlx::CollisionMap;
			_tiles[first_gid + id].cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
			_tiles[first_gid + id].vmap = new sdlx::CollisionMap;
			_tiles[first_gid + id].vmap->init(s, sdlx::CollisionMap::AnyVisible);
			_tiles[first_gid + id].surface = s;
			++id;
			s = NULL;
		}
	}
	const_cast<sdlx::Surface *>(image)->set_alpha(0, SDL_SRCALPHA);	//fixme: dangerous
} CATCH("addTiles", {const_cast<sdlx::Surface *>(image)->set_alpha(0, SDL_SRCALPHA); throw; })
	return id;
}

void IMap::getLayers(std::set<int> &layers_z) const {
	layers_z.clear();
	for(LayerMap::const_iterator i = _layers.begin(); i != _layers.end(); ++i) {
		layers_z.insert(i->first);
	}
}

Layer* IMap::getLayer(const int z) {
	LayerMap::iterator i = _layers.find(z);
	if (i == _layers.end())
		throw_ex(("getLayer(%d) could not find layer with given z", z));
	return i->second;
}

const IMap::TileDescriptor & IMap::getTile(const size_t idx) const {
	if (idx >= _tiles.size())
		throw_ex(("getTile(%u) is out of range 0-%u", (unsigned)idx, (unsigned)_tiles.size()));
	return _tiles[idx];
}

void IMap::generateXML(std::string &result) const {
	result = mrt::format_string(
		"<?xml version=\"1.0\"?>\n"
		"<map version=\"0.99b\" orientation=\"orthogonal\" width=\"%d\" height=\"%d\" tilewidth=\"%d\" tileheight=\"%d\">\n", 
		_w, _h, _tw, _th
		);
	if (!properties.empty()) {
		result += "\t<properties>\n";
		for(PropertyMap::const_iterator i = properties.begin(); i != properties.end(); ++i) {
			result += mrt::format_string("\t\t<property name=\"%s\" value=\"%s\"/>\n", escape(i->first).c_str(), escape(i->second).c_str());
		}
		result += "\t</properties>\n";
	}
	
	size_t n = _tilesets.size();
	for(size_t i = 0; i < n; ++i) {
		const TilesetList::value_type &ts = _tilesets[i];
		result += mrt::format_string("\t<tileset name=\"%s\" firstgid=\"%d\" tilewidth=\"%d\" tileheight=\"%d\">\n", 
			escape(mrt::FSNode::get_filename(ts.first, false)).c_str(), ts.second, _tw, _th);
		result += mrt::format_string("\t\t<image source=\"%s\"/>\n", escape(ts.first).c_str());
		result += "\t</tileset>\n";	
	}
	
	for(LayerMap::const_iterator i = _layers.begin(); i != _layers.end(); ++i) {
		std::string layer;
		i->second->generateXML(layer);
		result += layer;
	}
	result += "</map>\n";
}

void IMap::deleteLayer(const int kill_z) {
	LayerMap::iterator i = _layers.find(kill_z);
	if (i == _layers.end())
		throw_ex(("no layer with z %d", kill_z));
		
	LayerMap new_map;
	int z = -1000;
	
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ) {
		if (l->first == kill_z) {
			delete l->second;
			_layers.erase(l++);
			continue;
		}
		if (l->second->properties.find("z") != l->second->properties.end()) {
			z = atoi(l->second->properties["z"].c_str());
		}
		//LOG_DEBUG(("%s -> %d", l->second->name.c_str(), z));
		assert(new_map.find(z) == new_map.end());
		new_map[z++] = l->second;
		++l;
	}
	_layers = new_map;
	generateMatrixes();
}

void IMap::addLayer(const int after_z, const std::string &name) {
	int z = -1000;
	Layer *layer = NULL;
	
	if (!_layers.empty()) {
		LayerMap::iterator i = _layers.find(after_z);
		if (i == _layers.end())
			throw_ex(("no layer with z %d", after_z));
	} else {
		layer = new Layer(); //first layer
		layer->name = name;
		layer->init(_w, _h);
		_layers.insert(LayerMap::value_type(z++, layer));
		return;
	}
	
	LayerMap new_map;
	
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		if (l->second->properties.find("z") != l->second->properties.end()) {
			z = atoi(l->second->properties["z"].c_str());
		}
		//LOG_DEBUG(("%s -> %d", l->second->name.c_str(), z));
		if (new_map.find(z) != new_map.end()) {
			if (layer != NULL)
				delete layer;
			throw_ex(("no room for layer"));
		}
		new_map[z++] = l->second;
		
		if (z == after_z + 1) {
			layer = new Layer();
			layer->name = name;
			layer->init(_w, _h);
			new_map.insert(LayerMap::value_type(z++, layer));
		}
	}
	_layers = new_map;
}

const bool IMap::swapLayers(const int z1, const int z2) {
	LOG_DEBUG(("swap layers %d <-> %d", z1, z2));
	LayerMap::iterator l1 = _layers.find(z1), l2 = _layers.find(z2);
	if (l1 == _layers.end())
		throw_ex(("layer with z %d was not found", z1));
	if (l2 == _layers.end())
		throw_ex(("layer with z %d was not found", z2));

	bool has_z1 = l1->second->properties.find("z") != l1->second->properties.end();
	bool has_z2 = l2->second->properties.find("z") != l2->second->properties.end();
	if (has_z1 && has_z2) {
		LOG_DEBUG(("cannot swap two absolutely positioned layers."));
		return false;
	}
	math::exchange(l1->second, l2->second);
	LayerMap new_map;

	int z = -1000;
	for(LayerMap::iterator l = _layers.begin(); l != _layers.end(); ++l) {
		if (l->second->properties.find("z") != l->second->properties.end()) {
			z = atoi(l->second->properties["z"].c_str());
		}
		//LOG_DEBUG(("%s -> %d", l->second->name.c_str(), z));
		if (new_map.find(z) != new_map.end()) {
			LOG_DEBUG(("no room for new layer. restore changes..."));
			math::exchange(l1->second, l2->second);
			return false;
		}
		new_map[z++] = l->second;
	}
	_layers = new_map;
	return true;
}

template<typename T>
static void c2v(T &pos, const std::string &str) {
	std::string pos_str = str;

	const bool tiled_pos = pos_str[0] == '@';
	if (tiled_pos) { 
		pos_str = pos_str.substr(1);
	}

	TRY {
		pos.fromString(pos_str);
	} CATCH(mrt::format_string("parsing '%s'", str.c_str()).c_str() , throw;)

	if (tiled_pos) {
		v2<int> tile_size = Map->getTileSize();
		pos.x *= tile_size.x;
		pos.y *= tile_size.y;
		//keep z untouched.
	}
}

void IMap::resize(const int left_cut, const int right_cut, const int up_cut, const int down_cut) {
	if (!loaded() || (left_cut == 0 && right_cut == 0 && up_cut == 0 && down_cut == 0))
		return;
	
	LOG_DEBUG(("cutting map: %d %d %d %d", left_cut, right_cut, up_cut, down_cut));
	if (left_cut < 0 && right_cut < 0 && -left_cut - right_cut >= _w)
		throw_ex(("invalid left/right shrink width"));
	if (up_cut < 0 && down_cut < 0 && -up_cut - down_cut >= _h)
		throw_ex(("invalid up/down shrink height"));
	for(LayerMap::iterator i = _layers.begin(); i != _layers.end(); ++i) {
		i->second->resize(left_cut, right_cut, up_cut, down_cut);
	}
	_w += left_cut + right_cut;
	_h += up_cut + down_cut;
	
	for(PropertyMap::iterator i = properties.begin(); i != properties.end(); ++i) {
		const std::string &name = i->first;
		std::string &value = i->second;
		if (name.compare(0, 6, "spawn:") == 0 || name.compare(0, 9, "waypoint:") == 0) { 
			v3<int> pos;
			c2v< v3<int> >(pos, value);
			pos.x += left_cut * _tw;
			pos.y += up_cut * _th;
			value = mrt::format_string("%d,%d,%d", pos.x, pos.y, pos.z);
			LOG_DEBUG(("fixed %s->%s", name.c_str(), value.c_str()));
		} else if (name.compare(0, 5, "zone:") == 0) {
			std::vector<std::string> res;
			mrt::split(res, value, ":", 2);

			v3<int> pos;
			c2v< v3<int> >(pos, res[0]);
			pos.x += left_cut * _tw;
			pos.y += up_cut * _th;
			
			value = mrt::format_string("%d,%d,%d:", pos.x, pos.y, pos.z) + res[1];
			LOG_DEBUG(("fixed %s->%s", name.c_str(), value.c_str()));
		}
	}

	map_resize_signal.emit(left_cut * _tw, right_cut * _tw, up_cut * _th, down_cut * _th);
}

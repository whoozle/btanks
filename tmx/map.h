#ifndef __BT_MAP_H__
#define __BT_MAP_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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


#include <sigc++/sigc++.h>
#include "export_btanks.h"

#include <map>
#include <string>
#include <stack>
#include <set>
#include "mrt/chunk.h"
#include "math/v2.h"
#include "math/matrix.h"
#include "mrt/singleton.h"

#include "notifying_xml_parser.h"

#include "sdlx/c_map.h"
#include "tileset_list.h"

namespace sdlx {
class Surface;
class Rect;
}

class TMXEntity;
class Layer;
class Object;
class MapGenerator;

class BTANKSAPI IMap : public NotifyingXMLParser, public mrt::Serializable {
public:
	sigc::signal0<void> load_map_signal;
	sigc::signal0<void> load_map_final_signal;

	DECLARE_SINGLETON(IMap);
	struct TilePosition {
		v2<int> position;
		bool merged_x;
		bool merged_y;
		int prev_im;
	};
	
	const TilesetList & getTilesets() const { return _tilesets; }
	void getLayers(std::set<int> &layers_z) const;
	Layer* getLayer(const int z);

	typedef std::map<const std::string, std::string> PropertyMap;
	PropertyMap properties;

	IMap(); 
	~IMap();
	void clear();
	void load(const std::string &name);
	const bool loaded() const;
	
	void tick(const float dt);
	void render(sdlx::Surface &window, const sdlx::Rect &src, const sdlx::Rect &dst, const int z1, const int z2) const;
	const v2<int> getSize() const;
	const v2<int> getTileSize() const;
	const v2<int> getPathTileSize() const;
	
	virtual const int getImpassability(const Object *obj, const v2<int>& pos, TilePosition *tile_pos = NULL, bool *hidden = NULL) const;

	const Matrix<int>& getImpassabilityMatrix(const int z);
	const Matrix<int>& getAreaMatrix(const std::string &name);
	//void getSurroundings(Matrix<int> &matrix, const v2<int> &pos, const int filler = -1) const;
	
	void damage(const v2<float> &position, const int hp);
	void damage(const v2<float> &center_position, const int hp, const float radius);
	void _destroy(const int z, const v2<int> &cell);
	
	struct TileDescriptor {
		TileDescriptor() : surface(0), cmap(0), vmap(0) {}
		TileDescriptor(sdlx::Surface * surface, sdlx::CollisionMap *cmap, sdlx::CollisionMap *vmap) : 
			surface(surface), cmap(cmap), vmap(vmap) {}
		
		sdlx::Surface * surface;
		sdlx::CollisionMap *cmap, *vmap;
	};
	typedef std::vector< TileDescriptor > TileMap;
	
	void invalidateTile(const int xp, const int yp);

	void generateMatrixes();
	void getZBoxes(std::set<int> &layers);
	
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string &getName() const { return _name; }

private:
	void addTiles(sdlx::Surface *image, const int first_gid);

	Matrix<int> &getMatrix(int z);
	Matrix<int> &getMatrix(const std::string &name);

	void updateMatrix(const int x, const int y);
	void updateMatrix(Matrix<int> &matrix, const Layer *layer);

	virtual void start(const std::string &name, Attrs &attr);
	virtual void end(const std::string &name);
	virtual void charData(const std::string &data);

	virtual const sdlx::Surface* getSurface(const Layer *l, const int x, const int y) const;
	virtual const sdlx::CollisionMap* getCollisionMap(const Layer *l, const int x, const int y) const;
	virtual const sdlx::CollisionMap* getVisibilityMap(const Layer *l, const int x, const int y) const;

	typedef std::map<const int, Matrix<int> > MatrixMap;
	MatrixMap _imp_map;
	typedef std::map<const std::string, Matrix<int> > ObjectAreaMap;
	ObjectAreaMap _area_map;
	
	inline const bool collides(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const;
	inline const bool hiddenBy(const Object *obj, const int dx, const int dy, const sdlx::CollisionMap *tile) const;

	int _w, _h, _tw, _th, _ptw, _pth, _firstgid, _split;
	sdlx::CollisionMap _full_tile;
	
	int _lastz;
	mrt::Chunk _data;
	std::string _image_name, _image_source;
	sdlx::Surface *_image;
	bool _image_is_tileset;

	PropertyMap _properties;
	
	typedef std::map<const int, Layer *> LayerMap;
	LayerMap _layers;
	Matrix<int> _cover_map;
	
	std::map<const std::string, std::string> _damage4;
	std::map<const std::string, int> _layer_z;
	bool _layer;
	std::string _layer_name;

	TileMap _tiles;

	struct Entity {
		mrt::XMLParser::Attrs attrs;
		std::string data;
		Entity(const mrt::XMLParser::Attrs & attrs) : attrs(attrs), data() {}
	};
	
	typedef std::stack<Entity> EntityStack;
	EntityStack _stack;
	
	IMap(const IMap&);
	const IMap& operator=(const IMap&);
	
	MapGenerator *_generator;
	
	TilesetList _tilesets;
	std::string _name;
};

SINGLETON(BTANKSAPI, Map, IMap);

#endif


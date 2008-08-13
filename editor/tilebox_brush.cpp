/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include "tilebox_brush.h"
#include "tmx/map.h"
#include "tmx/generator.h"
#include "tmx/generator_object.h"
#include "command.h"
#include <assert.h>

TileBoxBrush::TileBoxBrush(const std::string &tileset, const std::string &name) {
	size.x = size.y = 1;
	tilebox = dynamic_cast<const generator::TileBox*>(Map->getGenerator()->getObject(tileset, name + "|"));
	if (tilebox == NULL)
		throw_ex(("generator object %s:%s is not a box", tileset.c_str(), name.c_str()));
	if (tilebox->split_w[1] == 0 || tilebox->split_h[1] == 0)
		throw_ex(("cannot handle primary boxes w/o central tiles. "));
	
	gid = Map->getTilesets().exists(tileset);
	
	if (gid == 0) 
		throw_ex(("map claims that it has not tileset '%s'. this is bug.", tileset.c_str()));
	
	TRY {
		tilebox_out = dynamic_cast<const generator::TileBox*>(Map->getGenerator()->getObject(tileset, "|" + name));
		if (tilebox_out == NULL)
			throw_ex(("generator object %s:%s is not a box", tileset.c_str(), name.c_str()));
	} CATCH("ctor", tilebox_out = NULL);
}

const bool TileBoxBrush::check(const generator::TileBox *tilebox, const int tid, const bool base) {
	if (tid == 0) 
		return !base;

	const int x1 = tilebox->split_w[0], y1 = tilebox->split_h[0];
	const int x2 = x1 + tilebox->split_w[1], y2 = y1 + tilebox->split_h[1];

	const Matrix<int> &tiles = tilebox->tiles;
	for(int y = 0; y < tiles.get_height(); ++y) 
		for(int x = 0; x < tiles.get_width(); ++x) {
			if (x >= x1 && x < x2 && y >= y1 && y < y2) {
				if (!base)
					continue;
			} else {
				if (base)
					continue;
			}
			if (tid == tiles.get(y, x))
				return true;
		}
	return false;
}

const bool TileBoxBrush::check(const int tid, const bool base) const {
	if (tid == 0) 
		return !base;
	return check(tilebox, tid, base) || (tilebox_out != NULL? check(tilebox_out, tid, base): false);
}


void TileBoxBrush::exec(Command& command,  const int x, const int y) const {
	const int w = tilebox->split_w[1], h = tilebox->split_h[1];
	int tx = tilebox->split_w[0] + (x % w), ty = tilebox->split_h[0] + (y % h);

	Matrix<int> around;
	around.set_size(3, 3, 0);
	for(int dy = -1; dy <= 1; ++dy) 
		for(int dx = - 1; dx <= 1 ; ++dx) {
			int tid = command.getTile(x + dx, y + dy);
			if (tid)
				around.set(dy + 1, dx + 1, tid - gid + 1);
		}
	around.set(1, 1, tilebox->tiles.get(ty, tx));
		
	LOG_DEBUG(("surround matrix: %s", around.dump().c_str()));

	for(int dy = 0; dy < around.get_height(); ++dy) {
		for(int dx = 0; dx < around.get_width(); ++dx) {
			if (dx == 1 && dy == 1)
				continue;
			morph(around, dy, dx, y + dy - 1, x + dx - 1);
		}
	}

	for(int dy = -1; dy <= 1; ++dy) 
		for(int dx = - 1; dx <= 1 ; ++dx) {
			int tid = around.get(dy + 1, dx + 1);
			if (tid != 0) {
				command.setTile(x + dx, y + dy, tid + gid - 1);
			}
		}

	command.exec();
}

#define BITS(m, b) (((m) & (b)) == (b))

void TileBoxBrush::morph(Matrix<int> &ground, const int y, const int x, const int map_y, const int map_x) const {
	int base_tid = ground.get(y, x);
	if (!check(tilebox, base_tid, false)) //allow tile already in tileset
		return;
	LOG_DEBUG(("morph %d:%d", y, x));
	unsigned tile_bits = 0;
	for(int dy = -1; dy <= 1; ++dy) 
		for(int dx = - 1; dx <= 1 ; ++dx) {
			int yp = y + dy, xp = x + dx;
			if (xp < 0 || xp >= ground.get_width() || yp < 0 || yp >= ground.get_height())
				continue;
			//LOG_DEBUG(("tid = %d", ground.get(yp, xp)));
			bool morphable = check(ground.get(yp, xp), false);
			if (!morphable) {
				tile_bits |= 1 << ((dy + 1) * 3 + dx + 1);
			}
	}
	LOG_DEBUG(("mask = %08x", tile_bits));
	int tid = 0;
	
	if (BITS(tile_bits, 0x82) || BITS(tile_bits, 0x28)) {
		tid = tilebox->tiles.get(tilebox->split_h[0] + (map_y % tilebox->split_h[1]), tilebox->split_w[0] + (map_x % tilebox->split_h[1]));
	//outer box:
	} else if (tilebox_out != NULL && BITS(tile_bits, 0x0a)) {
		tid = tilebox_out->tiles.get(
			map_y % tilebox_out->split_h[0], 
			map_x % tilebox_out->split_w[0]);
	} else if (tilebox_out != NULL && BITS(tile_bits, 0x22)) {
		tid = tilebox_out->tiles.get(
			map_y % tilebox_out->split_h[0], 
			tilebox_out->split_w[0] + tilebox_out->split_w[1] + (map_x % tilebox_out->split_w[2]));
	} else if (tilebox_out != NULL && BITS(tile_bits, 0xa0)) {
		tid = tilebox_out->tiles.get(
			tilebox_out->split_h[0] + tilebox_out->split_h[1] + (map_y % tilebox_out->split_h[0]), 
			tilebox_out->split_w[0] + tilebox_out->split_w[1] + (map_x % tilebox_out->split_w[2]));
	} else if (tilebox_out != NULL && BITS(tile_bits, 0x88)) {
		tid = tilebox_out->tiles.get(
			tilebox_out->split_h[0] + tilebox_out->split_h[1] + (map_y % tilebox_out->split_h[0]), 
			map_x % tilebox_out->split_w[0]);
	//single tiles
	} else if (BITS(tile_bits, 0x02)) {
		tid = tilebox->tiles.get(
			tilebox->split_h[0] + tilebox->split_h[1] + (map_y % tilebox->split_h[2]), 
			tilebox->split_w[0] + (map_x % tilebox->split_w[1]));
	} else if (BITS(tile_bits, 0x20)) {
		tid = tilebox->tiles.get(
			tilebox->split_h[0] + (map_y % tilebox->split_h[1]), 
			map_x % tilebox->split_w[0]);
	} else if (BITS(tile_bits, 0x80)) {
		tid = tilebox->tiles.get(
			map_y % tilebox->split_h[0], 
			tilebox->split_w[0] + (map_x % tilebox->split_w[1]));
	} else if (BITS(tile_bits, 0x08)) {
		tid = tilebox->tiles.get(
			tilebox->split_h[0] + (map_y % tilebox->split_h[1]), 
			tilebox->split_w[0] + tilebox->split_w[1] + (map_x % tilebox->split_w[2]));
	} else if (BITS(tile_bits, 0x01)) {
		tid = tilebox->tiles.get(
			tilebox->split_h[0] + tilebox->split_h[1] + (map_y % tilebox->split_h[2]), 
			tilebox->split_w[0] + tilebox->split_w[1] + (map_x % tilebox->split_w[2]));
	} else if (BITS(tile_bits, 0x04)) {
		tid = tilebox->tiles.get(
			tilebox->split_h[0] + tilebox->split_h[1] + (map_y % tilebox->split_h[2]), 
			map_x % tilebox->split_w[0]);
	} else if (BITS(tile_bits, 0x40)) {
		tid = tilebox->tiles.get(
			map_y % tilebox->split_h[0], 
			tilebox->split_w[0] + tilebox->split_w[1] + (map_x % tilebox->split_w[2]));
	} else if (BITS(tile_bits, 0x100)) {
		tid = tilebox->tiles.get(map_y % tilebox->split_h[0], map_x % tilebox->split_w[0]);
	}
	//patch tile
	if (tid)
		ground.set(y, x, tid);
}

void TileBoxBrush::render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) {
	int w = tilebox->split_w[1], h = tilebox->split_h[1];
	int tx = tilebox->split_w[0] + (window_pos_aligned.x % w), ty = tilebox->split_h[0] + (window_pos_aligned.y % h);

	int tid = gid + tilebox->tiles.get(ty, tx);

	const IMap::TileDescriptor &td = Map->getTile(tid);
	assert(td.surface != NULL);
	
	surface.blit(*td.surface, window_pos_aligned.x, window_pos_aligned.y);
}

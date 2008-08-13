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

#include "base_brush.h"
#include "mrt/exception.h"
#include "tmx/layer.h"
#include "command.h"

Brush::Brush() : _tile_size(0, 0), _tiles() {}

void Brush::exec(Command &command, const int xp, const int yp) const {
	if ((int)_tiles.size() < size.x * size.y) 
		throw_ex(("not enough tiles in brush (%d vs %d)", (int)_tiles.size(), size.x * size.y));
		
	for(int y = 0; y < size.y; ++y) {
		for(int x = 0; x < size.x; ++x) {
			command.setTile(x + xp, y + yp, _tiles[size.x * y + x]);
		}
	}
	command.exec();
}

void Brush::render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) {
	for(int y = 0; y < size.y; ++y) {
		for(int x = 0; x < size.x; ++x) {
			const IMap::TileDescriptor &td = Map->getTile(_tiles[size.x * y + x]);
			if (td.surface != NULL) {
				surface.blit(*td.surface, window_pos_aligned.x + _tile_size.x * x, window_pos_aligned.y + _tile_size.y * y);
			}
		}
	}	
}

Eraser::Eraser(const v2<int> tile_size) : _tile_size(tile_size) {
	size.x = size.y = 1;
	_tiles.push_back(0); //Brush model compatibility
	
	_eraser.create_rgb(tile_size.x, tile_size.y, 32);
	_eraser.display_format_alpha();
	_eraser.fill(_eraser.map_rgba(255, 0, 0, 64));
}

void Eraser::exec(Command& command,  const int x, const int y) const {
	command.setTile(x, y, 0);
	command.exec();
}

void Eraser::render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) {
	surface.blit(_eraser, window_pos_aligned.x /*+ _tile_size.x * x*/, window_pos_aligned.y/* + _tile_size.y * y */);
}

FillerBrush::FillerBrush(const Brush &brush, const v2<int> &map_size) : 
	Brush(brush), _map_size(map_size) {}

void FillerBrush::exec(Command& command,  const int xp, const int yp) const {
	if ((int)_tiles.size() < size.x * size.y) 
		throw_ex(("not enough tiles in brush (%d vs %d)", (int)_tiles.size(), size.x * size.y));
	
	bool inversed = true;
	
	std::deque<v2<int> > queue;
	for(int y = 0; y < size.y; ++y) {
		for(int x = 0; x < size.x; ++x) {
			int tid = command.getTile(xp + x, yp + y);
			if (tid == 0) 
				inversed = false;
			queue.push_back(v2<int>(xp + x, yp + y));
		}
	}
	
	std::set<v2<int> > visited;
	
	while(!queue.empty()) {
		v2<int> pos = queue.front();
		queue.pop_front();
		if (pos.x < 0 || pos.x >= _map_size.x || pos.y < 0 || pos.y >= _map_size.y || visited.find(pos) != visited.end())
			continue;
		
		int tid = command.getTile(pos.x, pos.y);
		if ( (!inversed && tid != 0) || (inversed && tid == 0))
			continue;
		int x = pos.x % size.x, y = pos.y % size.y;
		tid = _tiles[size.x * y + x];
		//LOG_DEBUG(("%d %d (+%d +%d) -> %d", pos.x, pos.y, x, y, tid));
		command.setTile(pos.x, pos.y, tid);
		visited.insert(pos);
		
		queue.push_back(v2<int>(pos.x, pos.y + 1));
		queue.push_back(v2<int>(pos.x, pos.y - 1));
		queue.push_back(v2<int>(pos.x + 1, pos.y));
		queue.push_back(v2<int>(pos.x - 1, pos.y));
	}
}

void FillerBrush::render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) {
	throw_ex(("implement me"));
}

#include "object.h"
#include "resource_manager.h"
#include "world.h"
#include "editor.h"

ObjectBrush::ObjectBrush(Editor * editor, const std::string &classname, const std::string &animation, const int z) : 
	classname(classname), animation(animation), z(z), 
	editor(editor), object(ResourceManager->createObject(classname, animation)) {
	World->addObject(object, v2<float>());
	size = object->size.convert<int>();
}

void ObjectBrush::exec(Command& command,  const int x, const int y) const {
	LOG_DEBUG(("adding object"));
}

void ObjectBrush::render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) {
	editor->moveObjectHack(object, window_pos - object->size.convert<int>() / 2);
}

ObjectBrush::~ObjectBrush() {
	object->Object::emit("death", NULL);
	World->purge(0);
}

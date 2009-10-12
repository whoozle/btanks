#ifndef BTANKS_EDITOR_BASE_BRUSH_H__
#define BTANKS_EDITOR_BASE_BRUSH_H__

/* Battle Tanks Game
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

#include "math/v2.h"
#include <vector>

class Layer;
class Command;

namespace sdlx {
	class Surface;
}

#include "sdlx/surface.h"


class BaseBrush {
public: 
	v2<int> size;
	virtual void exec(Command &, const int x, const int y) const = 0;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned) = 0;
	virtual ~BaseBrush() {}
};

class Brush : public BaseBrush {
public: 
	Brush();
	Brush(const v2<int> tile_size, const std::vector<int> &tiles) : 
		_tile_size(tile_size), _tiles(tiles) {}
	virtual void exec(Command& command,  const int x, const int y) const;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned);
protected:
	v2<int> _tile_size;
	std::vector<int> _tiles;
};

class FillerBrush : public Brush {
public: 
	FillerBrush(const Brush &brush, const v2<int> &map_size);
	virtual void exec(Command& command,  const int x, const int y) const;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned);
private: 
	v2<int> _map_size;
};

class Eraser: public Brush {
public: 
	Eraser(const v2<int> tile_size);
	virtual void exec(Command& command,  const int x, const int y) const;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned);
	
private: 
	v2<int> _tile_size;
	sdlx::Surface _eraser;
};

class Editor;
class Object;

class ObjectBrush: public BaseBrush {
public: 
	std::string classname, animation;
	int z;

	ObjectBrush(Editor * editor, const std::string &classname, const std::string &animation, const int z);
	~ObjectBrush();

	virtual void exec(Command& command,  const int x, const int y) const;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned);	
private: 
	Editor * editor;
	Object * object;
};

#endif



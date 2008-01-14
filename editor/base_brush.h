#ifndef BTANKS_EDITOR_BASE_BRUSH_H__
#define BTANKS_EDITOR_BASE_BRUSH_H__

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



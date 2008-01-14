#ifndef BTANKS_EDITOR_TILEBOX_BRUSH_H__
#define BTANKS_EDITOR_TILEBOX_BRUSH_H__

#include "base_brush.h"
#include "math/matrix.h"

namespace generator {
	class TileBox;
}

class TileBoxBrush : public BaseBrush {
public: 
	TileBoxBrush(const std::string &tileset, const std::string &name);
	virtual void exec(Command& command,  const int x, const int y) const;
	virtual void render(sdlx::Surface &surface, const v2<int>& window_pos, const v2<int>& window_pos_aligned);
private: 
	static const bool check(const generator::TileBox *tilebox, const int tid, const bool base);
	const bool check(const int tid, const bool base) const;
	void morph(Matrix<int> &ground, const int y, const int x, const int map_y, const int map_x) const;
	const generator::TileBox *tilebox, *tilebox_out;
	
	int gid;
};

#endif
